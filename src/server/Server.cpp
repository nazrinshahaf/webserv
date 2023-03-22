#include "Server.hpp"
#include "../socket/Socket.hpp"
#include "request/Request.hpp"
#include "response/Response.hpp"
#include "../config/ServerConfig.hpp"
#include "../config/ServerLocationDirectiveConfig.hpp"
#include "../config/ServerNormalDirectiveConfig.hpp"
#include "../config/ServerConfigParser.hpp"
#include "../log/Log.hpp"
#include "../colours.h"

#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <netinet/in.h>
#include <ostream>
#include <poll.h>
#include <fcntl.h>
#include <string>
#include <sys/_types/_size_t.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <string>
#include <utility>
#include <vector>

using namespace webserv;

using std::string;
using std::cout;
using std::endl;
using std::to_string;

Server::Server(const ServerConfigParser &config, char **envp) : _config(config), _envp(envp)
{
#ifdef PRINT_MSG
	cout << GREEN "Server Assignment Constructor Called" RESET << endl;
#endif
	//comment this out when testing.
	pair<ServerConfigParser::cit_t, ServerConfigParser::cit_t> config_block_range = _config.find_values("server");
	for (ServerConfigParser::cit_t config_in_block = config_block_range.first;
			config_in_block != config_block_range.second; config_in_block++)
	{
		ServerConfig server_config = dynamic_cast<ServerConfig&>(*(config_in_block->second));
		pair<ServerConfig::cit_t, ServerConfig::cit_t> server_block_range = server_config.find_values("listen");

		for (ServerConfig::cit_t sit = server_block_range.first; sit != server_block_range.second; sit++)
		{
			ServerNormalDirectiveConfig nd = dynamic_cast<ServerNormalDirectiveConfig&>(*(sit->second));
			add_socket(server_config, nd);
			Log(INFO, string("Server open at port ") + nd.get_value());
		}
	}
}

Server::~Server()
{
#ifdef PRINT_MSG
	cout << RED "Server Destructor Called" RESET << endl;
#endif
}
		
void	Server::add_socket(const ServerConfig &server_config, const ServerNormalDirectiveConfig &socket_config)
{
	int				port = atoi(socket_config.get_value().c_str());
	ListeningSocket	server_socket(AF_INET, SOCK_STREAM, 0, port, INADDR_ANY, 100, server_config);

	_server_sockets.insert(std::make_pair(server_socket.get_sock(), server_socket));
}

/*
 * For every socket that is open, see if any requets are sent to it.
 *	if so set _client_sockets[socket_fd] to open.
 *
 *	*not sure if we need to change the values for adddres for now its left at NULL
 * */

void	Server::acceptor(ListeningSocket &server)
{
	int	new_socket_fd;

	new_socket_fd = server.accept_connections();
	if (new_socket_fd >= 0)
	{
		Log(INFO, "========================================================");
		Log(INFO, string("Client open with fd : ") + to_string(new_socket_fd),
				0, NULL, NULL, 2, server.get_config());
		Log(INFO, "Client connected with ip : " + server.get_client_ip(),
				0, NULL, NULL, 2, server.get_config()); //im preety sure this addres needs to be client accept addr if so we might need to make a accept socket ):
		_client_sockets[new_socket_fd] = string("");

		struct pollfd client_fd_to_insert;
		client_fd_to_insert.fd = new_socket_fd;
		client_fd_to_insert.events = POLLIN; //set the event for client to be POLLIN
		_poll_fds.push_back(client_fd_to_insert);
		_client_server_pair.insert(std::make_pair(client_fd_to_insert.fd, server.get_sock()));
	}
}

int	Server::receiver(const int &client_fd)
{
	char	buffer[_recv_buffer_size + 1] = {0}; //+1 for \0
	long	valread;
	string	client_header_request;

	valread = recv(client_fd, buffer, _recv_buffer_size, 0);
	client_header_request.append(buffer, valread);
	if (valread == 0)
		return (-1);
	if (_requests.find(client_fd) == _requests.end()) // if this request is new
	{
		_requests[client_fd] = Request(client_header_request, client_fd);
		_timeout[client_fd] = 0;
	}
	else //if this request is already being processed
	{
		Log(DEBUG, "Request not new");
		if (!_requests[client_fd].header_done())
			_requests[client_fd].read_header(client_header_request);
		else 
			if (client_header_request.length() != 0)
				_requests[client_fd].add_body(client_header_request);
	}

	if ((_requests[client_fd].header_done() && (_requests[client_fd].type() == "GET" || _requests[client_fd].type() == "DELETE")) ||
		(_requests[client_fd].done() && _requests[client_fd].type() == "POST") ||
			_requests[client_fd].bad_request())
	{
		Log(INFO, _requests[client_fd].to_str());
		return (1); //sent full request
	}
	return (0); //sent partial request
}

int		Server::responder(ListeningSocket &server, int &client_fd, char **envp)
{
	Request	req = _requests.find(client_fd)->second;

	if (_responses.find(client_fd) == _responses.end()) // if this request is new
	{
		Log(DEBUG, "Respond new");
		Response responder(req, server, client_fd, envp);
		_responses[client_fd] = responder;
	}
	else
		Log(DEBUG, "Respond not new");

	_responses[client_fd].respond();
	if (_responses[client_fd].hasText() == false)
	{
		Log(INFO, _responses[client_fd].get_header());
		return (1); //sent full proper resonpose
	}
	return (0); //partion request
}

/*
 *
 * for (fd in pollfd)
 *	   If Server and revents POLLIN
 *			accepting connections
 *				if connect add client to pool
 *	   If Client and revents is POLLIN
 *			read http request
 *				if request is finish remove from pool and reinsert into pool as POLLOUT
 *				else if resuest is not done leave it as POLLIN
 *	   If Client revents POLLOUT
 *			write to client
 *				if response is chunked then keep clien in pool for next poll
 *				else if response is done remove clinet from pool than close client fd.
 * */

void	Server::launch()
{
	add_servers_to_poll();

    while(1)
    {
		/* Log(DEBUG, (string("Total amount of client_fds open : ") + to_string(_client_sockets.size()))); */
		/* Log(DEBUG, (string("Total amount of polls open : ") + to_string(_poll_fds.size()))); */
		int poll_rv = poll(_poll_fds.data(), _poll_fds.size(), 100);
		if (poll_rv < 0)
		{
			Log(ERROR, "Poll failed ", __LINE__, __PRETTY_FUNCTION__, __FILE__);
			return ;
		}
		for (size_t i = 0; i < _poll_fds.size(); i++)
		{
			struct pollfd			*curr_poll = &_poll_fds[i];
			sockets_type::iterator	server = _server_sockets.find(curr_poll->fd); //finds server

			if (server == _server_sockets.end() && ++_timeout[curr_poll->fd] > 1000)
			{
				// Log(DEBUG, string("Client: " + to_string(curr_poll->fd) + " timeout: " + to_string(_timeout[curr_poll->fd]) + "s"));
				remove_client(curr_poll->fd, i);
				continue;
			}
			if (curr_poll->revents == 0) //if no events are detected on server
			{
				/* Log(DEBUG, "No revents for " + to_string(curr_poll->fd)); */
				continue;
			}

			//printing all open clients on fd
			// for (std::map<int, string>::iterator server = _client_sockets.begin(); server != _client_sockets.end(); server++)
				// Log(INFO, (string("Client fd[") + to_string(server->first) + "] is open"));

			// if (curr_poll->revents != 0) {
			// 	printf("fd : %d; revents: %s%s%s\n", curr_poll->fd,
			// 			(curr_poll->revents & POLLIN)  ? "POLLIN "  : "",
			// 			(curr_poll->revents & POLLHUP) ? "POLLHUP " : "",
			// 			(curr_poll->revents & POLLERR) ? "POLLERR " : "");
			// }

			if (server != _server_sockets.end()) //if server socket
			{
				if (curr_poll->revents != POLLIN) //if server is not POLLIN fatal error
				{
					Log(ERROR, "Server no longer on POLLIN", __LINE__, __PRETTY_FUNCTION__, __FILE__);
					break ;
				}
				else if (curr_poll->revents & POLLIN) //if pollfd is a server and revents is triggered
				{
					// Log(DEBUG, "Poll server socket with fd : " + to_string(curr_poll->fd));
					acceptor(server->second);
				}
			}
			else //if client socket
			{
				// std::map<int,int>::iterator	pair = _client_server_pair.find(curr_poll->fd);
				// Log(DEBUG, "Poll client socket with fd : " + to_string(curr_poll->fd) + ". Connected to server with fd : " + to_string(pair->second));
				server = _server_sockets.find(_client_server_pair.find(curr_poll->fd)->second); //find server that client connected to.
				if (curr_poll->revents & POLLHUP) //if client hungup (refresh) or ^C
				{
					Log(INFO, string("Client Hung Up Connection " + string(YELLOW) + "(POLLHUP)" + string(RESET)));
					remove_client(curr_poll->fd, i);
				}
				else if (curr_poll->revents & POLLIN) //handling receiving of http request
				{
					// Log(DEBUG, "Client POLLIN");
					int status = receiver(curr_poll->fd);
					if (status == 1) //if request is done remove from list and add to POLLOUT
					{
						// Log(DEBUG, "Client sent full request");
						curr_poll->events = POLLOUT;
					}
					else if (status == -1)
						remove_client(curr_poll->fd, i);
					else //if request not done
					{
						// Log(DEBUG, "Client not done full request");
						continue;
					}
				}
				else if (curr_poll->revents & POLLOUT) //handling respoonse of http request
				{
					// Log(DEBUG, "Client POLLOUT");
					if (responder(server->second, curr_poll->fd, _envp) == 1) //if finish sending response
					{
						// Log(DEBUG, "Server send full response to client");
						remove_client(curr_poll->fd, i);
					}
					else
					{
						// Log(DEBUG, "Server send partial response to client");
						continue;
					}
				}
			}
		}
    }
}

void	Server::add_servers_to_poll(void)
{
	for (sockets_type::const_iterator server_socket = _server_sockets.begin();
			server_socket != _server_sockets.end(); server_socket++)
	{
		struct pollfd	server_poll_to_insert;

		server_poll_to_insert.fd = server_socket->second.get_sock();
		server_poll_to_insert.events = POLLIN;
		fcntl(server_poll_to_insert.fd, F_SETFL, O_NONBLOCK);
		_poll_fds.push_back(server_poll_to_insert);
		Log(INFO, (string("Server socket open with fd ") + to_string(server_poll_to_insert.fd)));
	}
}

void	Server::remove_client(const int &client_fd, const int &poll_index)
{
	Log(INFO, (string("Client socket closed with fd ") + to_string(client_fd)));
	Log(INFO, "========================================================");
	_client_server_pair.erase(client_fd);
	_requests.erase(client_fd);
	_responses.erase(client_fd);
	_client_sockets.erase(client_fd);
	_timeout.erase(client_fd);
	close(client_fd);
	_poll_fds.erase(_poll_fds.begin() + poll_index);
}

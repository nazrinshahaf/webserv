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

Server::Server(const ServerConfigParser &config) : _config(config)
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
			/* int	fd = std::stoi(nd.get_value()); */
			/* _server_sockets.push_back(ListeningSocket(AF_INET, SOCK_STREAM, 0, std::stoi(nd.get_value()), INADDR_ANY, 10, server_config)); */
			/* _server_sockets.insert(std::make_pair(fd, ListeningSocket(AF_INET, SOCK_STREAM, 0, std::stoi(nd.get_value()), INADDR_ANY, 10, server_config))); */
			add_socket(server_config, nd);
			Log(INFO, string("Server open at port ") + nd.get_value(), 2, server_config);
		}
	}
	//cout << _config << endl;
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
	ListeningSocket	server_socket(AF_INET, SOCK_STREAM, 0, port, INADDR_ANY, 10, server_config);

	_server_sockets.insert(std::make_pair(server_socket.get_sock(), server_socket));
}

//int	Server::handler(ListeningSocket &server_socket)
//{
//    long	valread;
//
//	const char *temp_message = "HTTP/1.1 500 FUCK OFF\r\nContent-Type: text/html\r\nContent-Length: 119\r\n\r\n";
//    // const char *err_msg = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n<!DOCTYPE html><html><body><h1>404 Not Found</h1></body></html>";
//
//    for (std::map<int, string>::iterator client = _client_sockets.begin(); client != _client_sockets.end(); )
//    {
//        char buffer[_recv_buffer_size + 1] = {0}; //+1 for \0
//		string temp;
//		
//       	valread = recv(client->first, buffer, _recv_buffer_size, 0);
//		if (valread < 0 && _responses[client->first].hasText() == false)
//		{
//			client++;
//			continue;
//		}
//
//		while (valread > 0)
//		{
//			temp.append(buffer, valread);
//	       	valread = recv(client->first, buffer, _recv_buffer_size, 0);
//		}
//
//		Request req(temp, client->first);
//		string root_path;
//		try 
//		{
//			root_path = server_socket.get_config().find_normal_directive("root").get_value();
//		}
//		catch (std::exception &e)
//		{
//			Log(WARN, string(e.what()), 2, server_socket.get_config());
//			root_path = "/";
//		}
//
//		if (_requests.find(client->first) == _requests.end()) // if this request is new
//			_requests[client->first] = req;
//		else //if this request is already being processed
//		{
//			req = _requests[client->first];
//			if (temp.length() != 0)
//				req.add_body(temp);
//		}
//
//		if (req.bad_request())
//		{
//			Log(DEBUG, string(RED) + "BAD REQUEST RECEIVED: " + string(RESET));
//            send(client->first , temp_message , strlen(temp_message), MSG_OOB);
//		    Log(INFO, "Client closed with ip : " + server_socket.get_client_ip(), 2, server_socket.get_config());
//			Log(ERROR, "Client ip is techincally wrong cause were changing client everything this might be an issue if we need to read socket address somwhere");
//			Log(DEBUG, (string("Client socket closed with fd ") + to_string(client->first)), 2, server_socket.get_config());
//			_requests.erase(client->first);
//            close(client->first);
//			_client_sockets.erase(client++);
//			return (1);
//		}
//	    
//        Log(DEBUG, req.to_str());
//		// Defaults to index.html
//		if (_responses.find(client->first) == _responses.end()) // if this request is new
//		{
//			Response responder(req, root_path, client);
//			_responses[client->first] = responder;
//		}
//		/* cout << "before" << endl; */
//		try {
//			_responses[client->first].respond();
//			if (errno == EPIPE)
//			{
//				_requests.erase(client->first);
//				_responses.erase(client->first);
//				close(client->first);
//				_client_sockets.erase(client++);
//				return (1);
//			}
//			if (_responses[client->first].hasText() == false)
//			{
//				Log(DEBUG, "------ Message Sent to Client ------ ");
//				cout << "For loop has ended bro\n";
//				Log(INFO, "Client closed with ip : " + server_socket.get_client_ip(), 2, server_socket.get_config());
//				Log(ERROR, "Client ip is techincally wrong cause were changing client everything this might be an issue if we need to read socket address somwhere");
//				Log(DEBUG, (string("Client socket closed with fd ") + to_string(client->first)), 2, server_socket.get_config());
//				_requests.erase(client->first);
//				_responses.erase(client->first);
//				close(client->first);
//				_client_sockets.erase(client++);
//				return (1);
//			}
//		} catch (std::exception &e) //sent = 0
//		{
//			cout << "send = 0 here" << endl;
//			cout << e.what() << endl;
//			/* _requests.erase(client->first); */
//			/* _responses.erase(client->first); */
//			/* close(client->first); */
//			/* _client_sockets.erase(client++); */
//			return (0);
//		}
//		// else
//		// 	client++;
//    }
//	return (0);
//	//TODO: close the fd incase poll cannot read the already open client, i.e client is closed
//}

/*
 * For every socket that is open, see if any requets are sent to it.
 *	if so set _client_sockets[socket_fd] to open.
 *	Active socket is a map of active client sockets (not sure why its value is string)
 *
 *	*not sure if we need to change the values for adddres for now its left at NULL
 * */

int	Server::acceptor(ListeningSocket &socket)
{
	int	new_socket_fd;

	new_socket_fd = socket.accept_connections();
	if (new_socket_fd >= 0)
	{
		Log(DEBUG, string("Server socket open with fd ") + to_string(new_socket_fd), 2, socket.get_config());
		Log(INFO, "Client connected with ip : " + socket.get_client_ip(), 2, socket.get_config()); //im preety sure this addres needs to be client accept addr if so we might need to make a accept socket ):
		_client_sockets[new_socket_fd] = string("");
	}
	return new_socket_fd;
}

int	Server::receiver(const ListeningSocket &server, const int &client_fd)
{
	(void)server;
	char	buffer[_recv_buffer_size + 1] = {0}; //+1 for \0
	long	valread;
	string	client_header_request;
	/* std::map<int, string>::iterator	client = _client_sockets.find(client_fd); */

	valread = recv(client_fd, buffer, _recv_buffer_size, 0);
	if (valread < 0) //nothing to read
		return 0; //sent partial request

	while (valread > 0)
	{
		client_header_request.append(buffer, valread);
		valread = recv(client_fd, buffer, _recv_buffer_size, 0);
	}

	Request	req(client_header_request, client_fd);

	if (_requests.find(client_fd) == _requests.end()) // if this request is new
		_requests[client_fd] = req;
	else //if this request is already being processed
	{
		req = _requests[client_fd];
		if (client_header_request.length() != 0)
			req.add_body(client_header_request);
	}
	if (req.done())
	{
		Log(DEBUG, req.to_str());
		return (1); //sent full request
	}
	return (0); //sent partial request
}

int		Server::responder(const ListeningSocket	&server, int client_fd)
{
	Request	req = _requests.find(client_fd)->second;
	string root_path;
	const char *temp_message = "HTTP/1.1 500 FUCK OFF\r\nContent-Type: text/html\r\nContent-Length: 119\r\n\r\n";


	try //try to find root path (this should be in responder)
	{
		root_path = server.get_config().find_normal_directive("root").get_value();
	}
	catch (std::exception &e)
	{
		Log(WARN, string(e.what()), 2, server.get_config());
		root_path = "/";
	}

	 //this should be in responder
	if (req.bad_request())
	{
		Log(DEBUG, string(RED) + "BAD REQUEST RECEIVED: " + string(RESET));
		send(client_fd , temp_message , strlen(temp_message), MSG_OOB);
		/* Log(INFO, "Client closed with ip : " + server.get_client_ip(), 2, server.get_config()); */
		/* Log(ERROR, "Client ip is techincally wrong cause were changing client everything this might be an issue if we need to read socket address somwhere"); */
		Log(DEBUG, (string("Client socket closed with fd ") + to_string(client_fd)), 2, server.get_config());
		close(client_fd);
		_requests.erase(client_fd);
		_client_sockets.erase(client_fd);
		return (1); //full bad reqquest response
	}

	if (_responses.find(client_fd) == _responses.end()) // if this request is new
	{
		std::map<int, string>::iterator	client = _client_sockets.find(client_fd);
		Response responder(req, root_path, client);
		_responses[client_fd] = responder;
	}
	try {
		_responses[client_fd].respond();
		/* if (errno == EPIPE) */
		/* { */
		/* 	_requests.erase(client_fd); */
		/* 	_responses.erase(client_fd); */
		/* 	close(client_fd); */
		/* 	_client_sockets.erase(client_fd); */
		/* 	return (1); //sent full respond epipe */
		/* } */
		if (_responses[client_fd].hasText() == false)
		{
			Log(DEBUG, "------ Message Sent to Client ------ ");
			cout << "For loop has ended bro\n";
			/* Log(INFO, "Client closed with ip : " + server_socket.get_client_ip(), 2, server_socket.get_config()); */
			// Log(ERROR, "Client ip is techincally wrong cause were changing client everything this might be an issue if we need to read socket address somwhere");
			/* Log(DEBUG, (string("Client socket closed with fd ") + to_string(client_fd)), 2, server_socket.get_config()); */
			close(client_fd);
			_requests.erase(client_fd);
			_responses.erase(client_fd);
			_client_sockets.erase(client_fd);
			return (1); //sent full proper resonpose
		}
	} catch (std::exception &e)
	{
		cout << e.what() << endl;
		/* close(client_fd); */
		/* _requests.erase(client_fd); */
		/* _responses.erase(client_fd); */
		/* _client_sockets.erase(client_fd); */
		return (0); //idk what error this is but its done
	}
	// else
	// 	client++;
	
	cout << "ENDING 0 RETURN" << endl;
	/* Log(ERROR, req.to_str()); */
	return (0); //partion request
}

void	Server::launch()
{
    /* struct pollfd fds[200]; */

	std::vector<struct pollfd>	poll_fds;

    /* int nfds = _server_sockets.size(); */
    
	int i = 0; //apparently u cant make a for loop with 2 variables if they are of different types
    for (sockets_type::const_iterator server_socket = _server_sockets.begin();
			server_socket != _server_sockets.end(); server_socket++, i++)
    {
		struct pollfd	fd_to_insert;

		fd_to_insert.fd = server_socket->second.get_sock();
		fd_to_insert.events = POLLIN;
		fcntl(fd_to_insert.fd, F_SETFL, O_NONBLOCK);
		poll_fds.push_back(fd_to_insert);
        /* fds[i].fd = server_socket->second.get_sock(); */
        /* fds[i].events = POLLIN; // Data other than high priority data maybe read without blocking */
        /* fcntl(fds[i].fd, F_SETFL, O_NONBLOCK); //set filestatus to non-blocking */
		Log(DEBUG, (string("Server socket open with fd ") + to_string(fd_to_insert.fd)));
    }

	std::map<int, int> client_server_pair;

    while(1)
    {
		/* Log(DEBUG, (string("Total amount of client_fds open : ") + to_string(_client_sockets.size()))); */
        // Run this only if a socket is queued (poll) OR we have open sockets that have not yet written bytes
		/* Log(ERROR, string("Request size : ") +to_string(_requests.size())); */
		int rc = poll(poll_fds.data(), poll_fds.size(), 1000);
		if (rc)
		{
			for (size_t i = 0; i < poll_fds.size(); i++)
			{
				/* cout << "i : " << i <<  endl; */
				/* cout << "fds[i].fd : " << fds[i].fd << endl; */

				//Server and revents POLLIN
				//	accepting connections
				//		if connect add client to pool
				//Client and revents is POLLIN
				//	read http request
				//		if request is finish remove from pool and reinsert into pool as POLLOUT
				//		if resuest is not done leave it as POLLIN
				//revents POLLOUT
				//	write to client
				//		if response is chunked then keep clien in pool for next poll
				//		else remove clinet from pool than close client fd.


				sockets_type::iterator	server = _server_sockets.find(poll_fds[i].fd); //finds server

				if (poll_fds[i].revents == 0)
				{
					std::cerr << "no revents for " << poll_fds[i].fd << " continue" << endl;
					continue;
				}

				if (poll_fds[i].revents != POLLIN && server != _server_sockets.end())
				{
					std::cerr << poll_fds[i].fd << " REVENTS NO LONGER POLLIN" << endl;
					break ;
				}

				//printing all open clients on fd
				for (std::map<int, string>::iterator server = _client_sockets.begin(); server != _client_sockets.end(); server++)
					Log(INFO, (string("Client fd[") + to_string(server->first) + "] is open"));

				if (poll_fds[i].revents != 0) {
					printf("fd : %d; revents: %s%s%s\n", poll_fds[i].fd,
							(poll_fds[i].revents & POLLIN)  ? "POLLIN "  : "",
							(poll_fds[i].revents & POLLHUP) ? "POLLHUP " : "",
							(poll_fds[i].revents & POLLERR) ? "POLLERR " : "");
				}

				if (server != _server_sockets.end() && (poll_fds[i].revents & POLLIN)) //if pollfd is a server and revents is triggered
				{
					cout << "fd " << poll_fds[i].fd << " IS a server" << endl;
					struct pollfd client_fd_to_insert;

					client_fd_to_insert.fd = acceptor(server->second);
					client_fd_to_insert.events = POLLIN; //set the event for client to be POLLIN
					poll_fds.push_back(client_fd_to_insert);
					cout << "pair insert: " << client_fd_to_insert.fd << "," << server->first << endl;
					client_server_pair.insert(std::make_pair(client_fd_to_insert.fd, server->first));
				}
				else //if client socket
				{
					std::map<int,int>::iterator	pair = client_server_pair.find(poll_fds[i].fd);
					server = _server_sockets.find(client_server_pair.find(poll_fds[i].fd)->second); //find server that client connected to.
					
					cout << "fd " << poll_fds[i].fd << " IS NOT a server" << endl;
					cout << "pair to find " << pair->second << endl;
					if (poll_fds[i].revents & POLLHUP) //handling respoonse of http request
					{
						Log(INFO, string("Client has closed connection"));
						client_server_pair.erase(poll_fds[i].fd);
						_requests.erase(poll_fds[i].fd);
						poll_fds.erase(poll_fds.begin() + i);
					}
					else if (poll_fds[i].revents & POLLIN) //handling receiving of http request
					{
						cout << "client POLLIN" << endl;
						if (receiver(server->second, poll_fds[i].fd) == 1) //if request is done remove from list and add to POLLOUT
						{
							cout << "client sent full request" << endl;
							poll_fds[i].events = POLLOUT;
						}
						else //if request not done
						{
							cout << "client not done full request" << endl;
							continue;
						}

					}
					else if (poll_fds[i].revents & POLLOUT) //handling respoonse of http request
					{
						cout << "client POLLOUT" << endl;
						cout << _requests.size() << endl;
						if (responder(server->second, poll_fds[i].fd) == 1) //if finish sending response
						{
							cout << "server send full response to client" << endl;
							client_server_pair.erase(poll_fds[i].fd);
							poll_fds.erase(poll_fds.begin() + i);
							/* poll_fds[i].fd = -1; */
							/* nfds--; */
						}
						else
						{
							cout << "server send partial response to client" << endl;
							continue;
						}
					}
				}
			}
		}
		else if (rc == 0)
		{
			std::cerr << "poll loop" << endl;
			cout << _requests.size() << endl;
		}
		else if (rc < 0)
		{
			std::cerr << "Poll failed" << endl;
			return ;
		}
    }
}


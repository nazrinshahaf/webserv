#include "Server.hpp"
#include "ListeningSocket.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ServerConfig.hpp"
#include "ServerLocationDirectiveConfig.hpp"
#include "ServerNormalDirectiveConfig.hpp"
#include "ServerConfigParser.hpp"
#include "Log.hpp"
#include "colours.h"

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

void	Server::handler(ListeningSocket &server_socket)
{
    long	valread;

	const char *temp_message = "HTTP/1.1 500 FUCK OFF\r\nContent-Type: text/html\r\nContent-Length: 119\r\n\r\n";
    // const char *err_msg = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n<!DOCTYPE html><html><body><h1>404 Not Found</h1></body></html>";

    for (std::map<int, string>::iterator it = _client_sockets.begin(); it != _client_sockets.end(); )
    {
        char buffer[_recv_buffer_size + 1] = {0}; //+1 for \0
		string temp;
		
       	valread = recv(it->first, buffer, _recv_buffer_size, 0);
		if (valread < 0 && _responses[it->first].hasText() == false)
		{
			it++;
			continue;
		}

		while (valread > 0)
		{
			temp.append(buffer, valread);
	       	valread = recv(it->first, buffer, _recv_buffer_size, 0);
		}

		Request req(temp, it->first);
		string root_path;
		try 
		{
			root_path = server_socket.get_config().find_normal_directive("root").get_value();
		}
		catch (std::exception &e)
		{
			Log(WARN, string(e.what()), 2, server_socket.get_config());
			root_path = "/";
		}

		if (_requests.find(it->first) == _requests.end()) // if this request is newd
			_requests[it->first] = req;
		else //if this request is already being processed
		{
			req = _requests[it->first];
			if (temp.length() != 0)
				req.add_body(temp);
		}

		if (req.bad_request())
		{
			Log(DEBUG, string(RED) + "BAD REQUEST RECEIVED: " + string(RESET));
		    Log(INFO, "Client closed with ip : " + server_socket.get_client_ip(), 2, server_socket.get_config());
            send(it->first , temp_message , strlen(temp_message), MSG_OOB);
            close(it->first);
			Log(ERROR, "Client ip is techincally wrong cause were changing it everything this might be an issue if we need to read socket address somwhere");
			Log(DEBUG, (string("Client socket closed with fd ") + to_string(it->first)), 2, server_socket.get_config());
			_client_sockets.erase(it++);
		}
		
	    
        Log(DEBUG, req.to_str());
		// Defaults to index.html
		if (_responses.find(it->first) == _responses.end()) // if this request is newd
		{
			Response responder(req, root_path, it);
			_responses[it->first] = responder;
		}
		_responses[it->first].respond();
		if (_responses[it->first].hasText() == false)
		{
			cout << "For loop has ended bro\n";
			_responses.erase(it->first);
			Log(DEBUG, "------ Message Sent to Client ------ ");
			close(it->first);
			_requests.erase(it->first);
			Log(INFO, "Client closed with ip : " + server_socket.get_client_ip(), 2, server_socket.get_config());
			Log(ERROR, "Client ip is techincally wrong cause were changing it everything this might be an issue if we need to read socket address somwhere");
			Log(DEBUG, (string("Client socket closed with fd ") + to_string(it->first)), 2, server_socket.get_config());
			_client_sockets.erase(it++);
		}
		// else
		// 	it++;
    }
	//TODO: close the fd incase poll cannot read the already open client, i.e client is closed
}

/*
 * For every socket that is open, see if any requets are sent to it.
 *	if so set _client_sockets[socket_fd] to open.
 *	Active socket is a map of active client sockets (not sure why its value is string)
 *
 *	*not sure if we need to change the values for adddres for now its left at NULL
 * */

void	Server::acceptor(ListeningSocket &socket)
{
	int	new_socket_fd;

	new_socket_fd = socket.accept_connections();
	if (new_socket_fd >= 0)
	{
		Log(DEBUG, string("Server socket open with fd ") + to_string(new_socket_fd), 2, socket.get_config());
		Log(INFO, "Client connected with ip : " + socket.get_client_ip(), 2, socket.get_config()); //im preety sure this addres needs to be client accept addr if so we might need to make a accept socket ):
		_client_sockets[new_socket_fd] = string("");
	}
}

void	Server::launch()
{
    struct pollfd fds[200];
    int nfds = _server_sockets.size();
    
	int i = 0; //apparently u cant make a for loop with 2 variables if they are of different types
    for (sockets_type::const_iterator server_socket = _server_sockets.begin();
			server_socket != _server_sockets.end(); server_socket++, i++)
    {
        fds[i].fd = server_socket->second.get_sock();
        fds[i].events = POLLIN; // Data other than high priority data maybe read without blocking
        fcntl(fds[i].fd, F_SETFL, O_NONBLOCK); //set filestatus to non-blocking
		Log(DEBUG, (string("Server socket open with fd ") + to_string(fds[i].fd)));
    }
    while(1)
    {
		/* Log(DEBUG, (string("Total amount of client_fds open : ") + to_string(_client_sockets.size()))); */
		/* for (std::map<int, string>::iterator it = _client_sockets.begin(); it != _client_sockets.end(); it++) */
		/* 	Log(DEBUG, (string("Client fd[") + to_string(it->first) + "] is open")); */
        // Run this only if a socket is queued (poll) OR we have open sockets that have not yet written bytes
        if (poll(fds, nfds, 1000) > 0 || _client_sockets.size() > 0)
        {
			for (sockets_type::iterator it = _server_sockets.begin(); it != _server_sockets.end(); it++)
			{
				acceptor(it->second);
				handler(it->second);
			}
        }
    }
}


#include "Server.hpp"
#include "ListeningSocket.hpp"
#include "Request.hpp"
#include "ServerConfig.hpp"
#include "ServerNormalDirectiveConfig.hpp"
#include "ServerConfigParser.hpp"
#include "colours.h"

#include <cstdlib>
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
#include <algorithm>
#include <cctype>
#include <string>

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
			_sockets.push_back(ListeningSocket(AF_INET, SOCK_STREAM, 0, std::stoi(nd.get_value()), INADDR_ANY, 10, server_config));
			log(INFO, string("Server open at port ") + nd.get_value(), 2, server_config);
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
		
void	Server::add_socket(const int &domain, const int &service, const int &protocol, const int &port, const u_long &interface, const int &backlog)
{
    _sockets.push_back(ListeningSocket(domain, service, protocol, port, interface, backlog));
}

void	Server::handler()
{
    struct sockaddr_in      address_read;
    // int                     address_read_len = sizeof(address_read);
    long	valread;

	const char *temp_message = "HTTP/1.1 404 OK\nContent-Type: text/html\nContent-Length: 100\n\n\
		<!DOCTYPE html>\
		<html>\
		<body>\
			<h1>My First fafafafafHeadifafng</h1>\
			<p>My first paragraph.</p>\
		</body>\
		</html>";

    for (std::map<int, string>::iterator it = _client_sockets.begin(); it != _client_sockets.end(); it++)
    {
        cout << string(inet_ntoa(address_read.sin_addr)) << endl; //to get callers ip
        char buffer[65535] = {0};
        valread = recv(it->first , buffer, 65535, 0);
        if (valread < 0)
            continue;
        else
        {
            printf("[%s]\n", buffer);
            printf("-------------\n");
        }
        // webserv::Request(string(buffer));
		// TODO: implement responder
        send(it->first , temp_message , strlen(temp_message), MSG_OOB);
        printf("------------------Hello message sent-------------------\n");
        close(it->first);
        _erase_list.push_back(it->first);
    }
	for (std::vector<int>::iterator it = _erase_list.begin(); it != _erase_list.end(); it++)
    	_client_sockets.erase(*it);
}

/*
 * For every socket that is open, see if any requets are sent to it.
 *	if so set _client_sockets[socket_fd] to open.
 *	Active socket is a map of active client sockets (not sure why its value is string)
 *
 *	*not sure if we need to change the values for adddres for now its left at NULL
 * */

void	Server::acceptor(const ListeningSocket &socket)
{
	int	new_socket_fd;

	new_socket_fd = socket.accept_connections();
	if (new_socket_fd >= 0)
		_client_sockets[new_socket_fd] = string("");
}

void	Server::launch()
{
    struct pollfd fds[200];
    int nfds = _sockets.size();
    
    for (unsigned long i = 0; i < _sockets.size(); i++)
    {
        fds[i].fd = _sockets[i].get_sock();
        fds[i].events = POLLIN; // Data other than high priority data maybe read without blocking
        fcntl(fds[i].fd, F_SETFL, O_NONBLOCK); //set filestatus to non-blocking
		log(DEBUG, (string("Server socket open with fd ") + to_string(fds[i].fd)));
    }
    while(1)
    {
		log(DEBUG, (string("Total amount of client_fds open : ") + to_string(_client_sockets.size())));
		for (std::map<int, string>::iterator it = _client_sockets.begin(); it != _client_sockets.end(); it++)
			log(DEBUG, (string("client fd[") + to_string(it->first) + "] is open"));
        // Run this only if a socket is queued (poll) OR we have open sockets that have not yet written bytes
        if (poll(fds, nfds, 1000) > 0 || _client_sockets.size() > 0)
        {
			for (sockets_type::iterator it = _sockets.begin(); it != _sockets.end(); it++)
			{
				acceptor(*it);
				//log(DEBUG, string("Total amount of client_fds open : ") + to_string(_client_sockets.size()));
				handler();
			}
        }
    }
}

/*
 * log_to_file :
 *	0 = just print to stdout
 *	1 = print to file passed in by server
 *	2 = print to both
 *
 *	@note : idk what to if file if error in config do i exit out early or what.
 * */

void	print_time(std::ostream &stream)
{
	std::time_t t = std::time(0);
	std::tm*	now = std::localtime(&t);
	char		prev_fill = std::cout.fill();

	stream << " " << (now->tm_year + 1900) << '-'
		<< std::setfill('0')<< std::setw(2) << (now->tm_mon + 1) << '-'
		<< std::setfill('0')<< std::setw(2) << now->tm_mday << " "
		<< std::setfill('0')<< std::setw(2) << now->tm_hour << ":"
		<< std::setfill('0')<< std::setw(2) << now->tm_min << ":"
		<< std::setfill('0')<< std::setw(2) << now->tm_sec<< " - ";
	std::setfill(prev_fill);
}

void	Server::log(const log_level &level, const string &log_msg, const int &log_to_file, ServerConfig const &server) const
{
	log_level		base_log_level = DEBUG;
	std::fstream	log_file;

	if (log_to_file >= 1) //opening file if log_to_file is set to 1 or 2
	{
		pair<ServerConfig::cit_t, ServerConfig::cit_t> pair = server.find_values("error_log");
		ServerNormalDirectiveConfig nd = dynamic_cast<ServerNormalDirectiveConfig&>(*(pair.first->second));

		log_file.open(nd.get_value(), std::ios::app); //append mode
		if (!log_file)
		{
			log(ERROR, "Cant write to " + nd.get_value()); //file permisions can prob just continue on
			return;
		}
		string	temp_log_level = nd.get_value2();
		std::transform(temp_log_level.begin(), temp_log_level.end(), temp_log_level.begin(), ::toupper); //convert to uppercase

		if (temp_log_level == "DEBUG") //no switch case for string 
			base_log_level = DEBUG;
		else if (temp_log_level == "INFO")
			base_log_level = INFO;
		else if (temp_log_level == "WARN")
			base_log_level = WARN;
		else if (temp_log_level == "ERROR")
			base_log_level = ERROR;
		else
		{
			log(ERROR, "Invalid error log format"); //prob need to exit out early if error in config. handle validity somwhere else??
			return ; 
		}
	}

	if (level < base_log_level)
		return ;

	switch(level)
	{
		case 0:
			if (log_to_file != 1)
				cout << GREEN "[DEBUG]\t" RESET;
			if (log_to_file >= 1)
				log_file << "[DEBUG]\t";
			break;
		case 1:
			if (log_to_file != 1)
				cout << BLUE "[INFO]\t" RESET;
			if (log_to_file >= 1)
				log_file << "[INFO]\t";
			break;
		case 2:
			if (log_to_file != 1)
				cout << YELLOW "[WARN]\t" RESET;
			if (log_to_file >= 1)
				log_file << "[WARN]\t";
			break;
		case 3:
			if (log_to_file != 1)
				cout << RED "[ERROR]\t" RESET;
			if (log_to_file >= 1)
				log_file << "[ERROR]\t";
			break;
		break;
	}

	switch (log_to_file) //can change all the print time to a function
	{
		case 2:
			print_time(cout);
			cout << log_msg << endl;
		case 1:
			print_time(log_file);
			log_file << log_msg << endl;
			break;
		case 0:
			print_time(cout);
			cout << log_msg << endl;
	}
	log_file.close();
}

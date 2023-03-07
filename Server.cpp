#include "Server.hpp"
#include "ListeningSocket.hpp"
#include "Request.hpp"
#include "ServerConfig.hpp"
#include "ServerLocationDirectiveConfig.hpp"
#include "ServerNormalDirectiveConfig.hpp"
#include "ServerConfigParser.hpp"
#include "colours.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <malloc/_malloc.h>
#include <netinet/in.h>
#include <ostream>
#include <poll.h>
#include <fcntl.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/wait.h>
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

Server::Server(const ServerConfigParser &config, char **envp) : _config(config),
	_envp(envp)
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
	const char *header = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n"; // Dynamically add content length TODO
	const char *header2 = "HTTP/1.1 200 OK\nContent-Type: image/*\r\n\r\n";
	const char *header_404 = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n";
	const char *header_cgi= "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";
    // const char *err_msg = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n<!DOCTYPE html><html><body><h1>404 Not Found</h1></body></html>";

    for (std::map<int, string>::iterator it = _client_sockets.begin(); it != _client_sockets.end(); )
    {
        char buffer[_recv_buffer_size + 1] = {0}; //+1 for \0
		string temp;
		
       	valread = recv(it->first, buffer, _recv_buffer_size, 0);
		if (valread < 0)
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
			log(WARN, string(e.what()), 2, server_socket.get_config());
			root_path = "/";
		}

		if (_requests.find(it->first) == _requests.end()) // if this request is newd
			_requests[it->first] = req;
		else //if this request is already being processed
		{
			req = _requests[it->first];
			req.add_body(temp);
		}

		if (req.bad_request())
		{
			log(DEBUG, string(RED) + "BAD REQUEST RECEIVED: " + string(RESET));
		    log(INFO, "Client closed with ip : " + server_socket.get_client_ip(), 2, server_socket.get_config());
            send(it->first , temp_message , strlen(temp_message), MSG_OOB);
            close(it->first);
			log(ERROR, "Client ip is techincally wrong cause were changing it everything this might be an issue if we need to read socket address somwhere");
			log(DEBUG, (string("Client socket closed with fd ") + to_string(it->first)), 2, server_socket.get_config());
			_client_sockets.erase(it++);
		}
		
	    
        log(DEBUG, req.to_str());
        std::ifstream myfile;
        string entireText;
        string line;
		// Defaults to index.html
		// TODO: implement in responder
		if (req.path() == "/")
			myfile.open(root_path + "/index.html", std::ios::binary);
		else if (req.path() == "/cgi")
		{
			cout << "ENTERED CGI PATH" << endl;
			entireText = "";
			int		fd[2];
			char	execve_buffer[100000];
			pipe(fd);
			pid_t i = fork();
			if (i == 0) //child
			{
				std::vector<std::string>  s;
				s.push_back("/Library/Frameworks/Python.framework/Versions/3.10/bin/python3");
				s.push_back("test.py");

				std::vector<char*> commands;
				for (size_t i = 0; i < s.size(); i++)
					commands.push_back(const_cast<char*>(s[i].c_str()));
				commands.push_back(nullptr);

				dprintf(2, "in child\n");
				dup2(fd[1], STDOUT_FILENO);
				close(fd[0]);
				close(fd[1]);
				execve("/Library/Frameworks/Python.framework/Versions/3.10/bin/python3", commands.data(), _envp);
				exit(1);
			}
			else if (i > 0) //parent
			{
				dprintf(2, "parent waiting\n");
				waitpid(i, NULL, 0);
				dprintf(2, "parent done waiting\n");
				int test = read(fd[0], execve_buffer, 99999);
				close(fd[0]);
				close(fd[1]);
				dprintf(2, "read count = [%d]\n", test);
				dprintf(2, "execve buffer = [%s]\n", execve_buffer);
				entireText = header_cgi;
				entireText += execve_buffer;
				cout << "[" << entireText << "]" << endl;
			}
			else
			{
				perror("fork failed");
				_exit(3);
			}
		}
		else
			myfile.open(root_path + req.path(), std::ios::binary);

		if (!myfile)
		{
			cout << "no myfile" << endl;
			entireText += header_404;
			myfile.open("public/404.html", std::ios::binary);
		}
		else if (req.path() != "/cgi") //remove this later
		{
			if (req.path() == "/edlim.jpg" || req.path() == "/edlim_lrg.jpg" || req.path() == "/jng.png")
				entireText += header2;
			else
				entireText += header;
		}

		char read_buffer[65535]; // create a read_buffer
		if (req.path() != "/cgi")
		{
			while (myfile.read(read_buffer, sizeof(read_buffer)))
				entireText.append(read_buffer, myfile.gcount());
			// send(it->first, read_buffer, myfile.gcount(), 0);
			while (std::getline(myfile, line))
				entireText += line;
			entireText.append(read_buffer, myfile.gcount());
			myfile.close();
		}
		if (req.done())
		{
			if (req.path() == "/upload.html")
				req.process_image();
			send(it->first , entireText.c_str() , entireText.length(), 0);
			log(DEBUG, "------ Message Sent to Client ------ ");
			close(it->first);
			_requests.erase(it->first);
			log(INFO, "Client closed with ip : " + server_socket.get_client_ip(), 2, server_socket.get_config());
			log(ERROR, "Client ip is techincally wrong cause were changing it everything this might be an issue if we need to read socket address somwhere");
			log(DEBUG, (string("Client socket closed with fd ") + to_string(it->first)), 2, server_socket.get_config());
			_client_sockets.erase(it++);
		}
    }
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
		log(DEBUG, string("Server socket open with fd ") + to_string(new_socket_fd), 2, socket.get_config());
		log(INFO, "Client connected with ip : " + socket.get_client_ip(), 2, socket.get_config()); //im preety sure this addres needs to be client accept addr if so we might need to make a accept socket ):
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
		log(DEBUG, (string("Server socket open with fd ") + to_string(fds[i].fd)));
    }
    while(1)
    {
		/* log(DEBUG, (string("Total amount of client_fds open : ") + to_string(_client_sockets.size()))); */
		/* for (std::map<int, string>::iterator it = _client_sockets.begin(); it != _client_sockets.end(); it++) */
		/* 	log(DEBUG, (string("Client fd[") + to_string(it->first) + "] is open")); */
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

// ============================== LOGGING ==================================

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
	std::cout.fill(prev_fill);

}

void	Server::print_debug_level(const log_level &level, const int &log_to_file, std::fstream &log_file, const log_level &file_log_level) const
{
	switch(level)
	{
		case 0:
			if (log_to_file != 1 && level >= _base_log_level)
				cout << GREEN "[DEBUG]\t" RESET;
			if (log_to_file >= 1 && level >= file_log_level)
				log_file << "[DEBUG]\t";
			break;
		case 1:
			if (log_to_file != 1 && level >= _base_log_level)
				cout << BLUE "[INFO]\t" RESET;
			if (log_to_file >= 1 && level >= file_log_level)
				log_file << "[INFO]\t";
			break;
		case 2:
			if (log_to_file != 1 && level >= _base_log_level)
				cout << YELLOW "[WARN]\t" RESET;
			if (log_to_file >= 1 && level >= file_log_level)
				log_file << "[WARN]\t";
			break;
		case 3:
			if (log_to_file != 1 && level >= _base_log_level)
				cout << RED "[ERROR]\t" RESET;
			if (log_to_file >= 1 && level >= file_log_level)
				log_file << "[ERROR]\t";
			break;
		break;
	}
}

void	Server::print_debug_msg(const log_level &level, const int &log_to_file, std::fstream &log_file,
			const log_level &file_log_level, const string &log_msg) const
{
	string	log_msg_copy = log_msg;

	while (1)
	{
		string log_line;
		if (log_msg_copy.length() == 0)
			break ;
		if (log_msg_copy.find_first_of("\n") == log_msg_copy.npos)
			log_line = log_msg_copy;
		else
			log_line = log_msg_copy.substr(0, log_msg_copy.find_first_of("\n"));
		print_debug_level(level, log_to_file, log_file, file_log_level);
		switch (log_to_file)
		{
			case 2 :
				if (level < _base_log_level)
					break;
				print_time(cout);
				cout << log_line << endl;
			case 1 :
				if (file_log_level < _base_log_level)
					break;
				print_time(log_file);
				log_file << log_line << endl;
				break;
			case 0 :
				if (level < _base_log_level)
					break;
				print_time(cout);
				cout << log_line << endl;
		}
		if (log_msg_copy.find_first_of("\n") == log_msg_copy.npos)
			break ;
		log_msg_copy = log_msg_copy.substr(log_msg_copy.find_first_of("\n") + 1);
	}
}

void	Server::log(const log_level &level, const string &log_msg, const int &log_to_file, ServerConfig const &server) const
{
	log_level		file_log_level;
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
			file_log_level = DEBUG;
		else if (temp_log_level == "INFO")
			file_log_level = INFO;
		else if (temp_log_level == "WARN")
			file_log_level = WARN;
		else if (temp_log_level == "ERROR")
			file_log_level = ERROR;
	}

	print_debug_msg(level, log_to_file, log_file, file_log_level, log_msg);
	log_file.close();
}


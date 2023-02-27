#include "Server.hpp"
#include <iostream>
#include <poll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Request.hpp"
#include <map>

using namespace webserv;
using std::string;

Server::Server()
{

}

Server::~Server()
{

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

    for (std::map<int, string>::iterator it = _active_sockets.begin(); it != _active_sockets.end(); it++)
    {
        std::cout << string(inet_ntoa(address_read.sin_addr)) << std::endl;
        char buffer[30000] = {0};
        valread = recv(it->first , buffer, 65535, 0);
        if (valread < 0)
        {
            perror("error");
            continue;
        }
        else
        {
            printf("[%s]\n",buffer );
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
    	_active_sockets.erase(*it);
}

void	Server::acceptor(struct pollfd *fds)
{
	int	new_socket_fd;

	for (unsigned long i = 0; i < _sockets.size(); i++)
    {
        if ((new_socket_fd = accept(fds[i].fd, NULL, NULL)) >= 0)
        {
            _active_sockets[new_socket_fd] = string("");
            while (new_socket_fd >= 0)
            {
                new_socket_fd = accept(fds[i].fd, NULL, NULL);
                if (new_socket_fd >= 0)
                    _active_sockets[new_socket_fd] = string("");
                else
                    break;
            }
            continue;
        }
    }
}

void Server::launch()
{
    struct pollfd fds[200];
    int nfds = _sockets.size();

    
    for (unsigned long i = 0; i < _sockets.size(); i++)
    {
        fds[i].fd = _sockets[i].get_sock();
        std::cout << "FDS[i].fd " << fds[i].fd << std::endl;
        fds[i].events = POLLIN; // Data other than high priority data maybe read without blocking
        fcntl(fds[i].fd, F_SETFL, O_NONBLOCK);
    }

    while(1)
    {
        std::cout << fds[nfds - 1].fd << " " << nfds << std::endl;
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        // Run this only if a socket is queued (poll) OR we have open sockets that have not yet written bytes
        if (poll(fds, nfds, 1000) > 0 || _active_sockets.size() > 0)
        {
			acceptor(fds);
            std::cout << _active_sockets.size() << " open sockets" << std::endl;
            handler();
        }
        else
            std::cout << "waiting..." << std::endl;
    }
}

#include "Server.hpp"
#include <iostream>
#include <poll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Request.hpp"

using namespace webserv;
using std::string;

Server::Server()
{

}

Server::~Server()
{

}
		
void Server::add_socket(const int &domain, const int &service, const int &protocol, const int &port, const u_long &interface, const int &backlog)
{
    _sockets.push_back(ListeningSocket(domain, service, protocol, port, interface, backlog));
}
		
void Server::launch()
{
    struct pollfd fds[200];
    int nfds = _sockets.size();

    const char *temp_message = "HTTP/1.1 404 OK\nContent-Type: text/html\nContent-Length: 100\n\n\
		<!DOCTYPE html>\
		<html>\
		<body>\
			<h1>My First fafafafafHeadifafng</h1>\
			<p>My first paragraph.</p>\
		</body>\
		</html>";

    for (unsigned long i = 0; i < _sockets.size(); i++)
    {
        fds[i].fd = _sockets[i].get_sock();
        std::cout << "FDS[i].fd " << fds[i].fd << std::endl;
        fds[i].events = POLLIN; // Data other than high priority data maybe read without blocking
        fcntl(fds[i].fd, F_SETFL, O_NONBLOCK);
    }
    
    struct sockaddr_in address_read;
    int                 address_read_len = sizeof(address_read);
    int                 new_socket_fd;
    long                valread;

    while(1)
    {
        std::cout << fds[nfds - 1].fd << " " << nfds << std::endl;
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if (poll(fds, nfds, 1000) > 0) //A positive value indicates the number of file descriptors whose returned event flags are not zero. A value of 0 indicates that the time limit expired and the requested events did not occur.
        {
            int temp_serv_fd;
            for (int i = 0; i < nfds; i++)
            {
                if (fds[i].revents)
                {
                    std::cout << "FD " << i << " HAS BEEN TRIGGERED " << std::endl;
                    temp_serv_fd = fds[i].fd;
                }
            }
            std::cout << "MESSAGE CAME" << std::endl;
            if ((new_socket_fd = accept(temp_serv_fd, (struct sockaddr *)&address_read, (socklen_t*)&address_read_len))<0)
            {
                perror("In accept");
                exit(EXIT_FAILURE);
            }
            std::cout << string(inet_ntoa(address_read.sin_addr)) << std::endl;
            char buffer[30000] = {0};
            valread = read( new_socket_fd , buffer, 30000);
            (void) valread;
            // printf("%s\n",buffer );
            // printf("-------------\n");
            webserv::Request(string(buffer));
            send(new_socket_fd , temp_message , strlen(temp_message), MSG_OOB);
            printf("------------------Hello message sent-------------------\n");
            close(new_socket_fd);
        }
        else
            std::cout << "waiting..." << std::endl;

    }
}

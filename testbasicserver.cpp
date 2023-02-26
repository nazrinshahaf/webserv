#include <poll.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#define PORT 81
#include "Request.hpp"

int test_basic_server()
{
    int server_fd;
	int new_socket;
	long valread;
    
    const char *hello = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 100\n\n\
		<!DOCTYPE html>\
		<html>\
		<body>\
			<h1>My First fafafafafHeadifafng</h1>\
			<p>My first paragraph.</p>\
		</body>\
		</html>";
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    int addrlen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
	struct pollfd mypoll;
    memset(&mypoll, server_fd, sizeof(mypoll));
    mypoll.events = POLLIN;
    mypoll.fd = server_fd;

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    struct pollfd fds[200];
    int    nfds = 1, current_size = 0, i, j;
    memset(fds, 0 , sizeof(fds));
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if (poll(fds, 1, 1000) > 0) //A positive value indicates the number of file descriptors whose returned event flags are not zero. A value of 0 indicates that the time limit expired and the requested events did not occur.
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("In accept");
                exit(EXIT_FAILURE);
            }

            char buffer[30000] = {0};
            valread = read( new_socket , buffer, 30000);
            // printf("%s\n",buffer );
            webserv::Request(string(buffer));
            send(new_socket , hello , strlen(hello), MSG_OOB);
            printf("------------------Hello message sent-------------------\n");
            close(new_socket);
        }
        else
            std::cout << "waiting..." << std::endl;
    }
    return 0;
}

int main(void)
{
    test_basic_server();
}

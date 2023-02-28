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
using std::cout;
using std::endl;

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

    for (std::map<int, string>::iterator it = _client_sockets.begin(); it != _client_sockets.end(); it++)
    {
        cout << string(inet_ntoa(address_read.sin_addr)) << endl; //to get callers ip
        char buffer[65535] = {0};
        valread = recv(it->first , buffer, 65535, 0);
        if (valread < 0)
            continue;
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
    	_client_sockets.erase(*it);
}

/*
 * For every socket that is open, see if any requets are sent to it.
 *	if so set _client_sockets[socket_fd] to open.
 *	Active socket is a map of active client sockets (not sure why its value is string)
 *
 *	*not sure if we need to change the values for adddres for now its left at NULL
 * */

void	Server::acceptor(const int &fd)
{
	int	new_socket_fd;

	if ((new_socket_fd = accept(fd, NULL, NULL)) >= 0) //*
		_client_sockets[new_socket_fd] = string("");
}

void Server::launch()
{
    struct pollfd fds[200];
    int nfds = _sockets.size();
    
    for (unsigned long i = 0; i < _sockets.size(); i++)
    {
        fds[i].fd = _sockets[i].get_sock();
        cout << "Socket open with fd : [" << fds[i].fd << "]" << endl;
        fds[i].events = POLLIN; // Data other than high priority data maybe read without blocking
        fcntl(fds[i].fd, F_SETFL, O_NONBLOCK); //set filestatus to non-blocking
    }
    while(1)
    {
        cout << "Final socket_fd open : " << fds[nfds - 1].fd << endl;
		cout << "Total amount of socket_fds open : " << nfds << endl;
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
		cout << _client_sockets.size() << " open sockets" << endl;
        // Run this only if a socket is queued (poll) OR we have open sockets that have not yet written bytes
        if (poll(fds, nfds, 1000) > 0 || _client_sockets.size() > 0)
        {
			for (unsigned long i = 0; i < _sockets.size(); i++)
			{
				acceptor(fds[i].fd);
				cout << _client_sockets.size() << " open sockets" << endl;
				handler();
			}
        }
        else
            cout << "waiting..." << endl;
    }
}

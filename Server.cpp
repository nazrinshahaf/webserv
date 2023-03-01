#include "Server.hpp"
#include <iostream>
#include <poll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Request.hpp"
#include <map>
#include <fstream>

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

	// const char *temp_message = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 119\r\n\r\n\
	// 	<!DOCTYPE html>\
	// 	<html>\
	// 	<body>\
	// 		<h1>My First fafafafafHeadifafng</h1>\
	// 		<p>My first paragraph.</p>\
	// 	</body>\
	// 	</html>";
	// const char *temp_message = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 13\n\nHello world!";

    // const char *err_msg = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n<!DOCTYPE html><html><body><h1>404 Not Found</h1></body></html>";

    for (std::map<int, string>::iterator it = _client_sockets.begin(); it != _client_sockets.end(); it++)
    {
        cout << string(inet_ntoa(address_read.sin_addr)) << endl; //to get callers ip
        char buffer[65535] = {0};
        valread = recv(it->first , buffer, 65535, 0);
        if (valread < 0)
            continue;
        it->second += buffer;
        Request req((string(buffer)));
        // printf("[%s]\n",buffer );
        // printf("-------------\n");
        // check if buffer is fully read
        // if so then close socket
		// TODO: implement responder AND COVERT IT TO OOP
        cout << req << "\n";
        cout << "The req.path is |" << req.path() << "|\n";

        std::ifstream myfile;
        string entireText;
        string line;

        if (req.path() == "/")
            myfile.open("public/index.html");
        else
            myfile.open("public" + req.path());
        if (!myfile)
            myfile.open("public/404.html");
        while (std::getline(myfile, line))
        {
            entireText += '\n';
            entireText += line;
        }
        myfile.close();
        // cout << "==========" << endl;
        // cout << entireText << "| This is entire text\n";
        // cout << "==========" << endl;
        // cout << err_msg << "| This is err_msg text\n";
        // cout << "==========" << endl;
        send(it->first , entireText.c_str() , strlen(entireText.c_str()) + 1, MSG_OOB);
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
        cout << YELLOW << "Socket open with fd : [" << fds[i].fd << "]" << RESET << endl;
        fds[i].events = POLLIN; // Data other than high priority data maybe read without blocking
        fcntl(fds[i].fd, F_SETFL, O_NONBLOCK); //set filestatus to non-blocking
    }
    cout << YELLOW << "Total amount of socket_fds open : " << nfds << RESET << endl;
    while(1)
    {
        // printf("\n+++++++ Waiting for new connection ++++++++\n\n");
		cout << "Connected clients: " << _client_sockets.size() << endl;
        // Run this only if a socket is queued (poll) OR we have open sockets that have not yet written bytes
        if (poll(fds, nfds, 1000) > 0 || _client_sockets.size() > 0)
        {
			for (unsigned long i = 0; i < _sockets.size(); i++)
			{
				acceptor(fds[i].fd);
				handler();
			}
        }
        else
            cout << "waiting..." << endl;
        std::cout << std::time(0) << std::endl;
    }
}

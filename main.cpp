#include <ios>
#include <sys/_types/_int8_t.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <poll.h>
#include "Request.hpp"
#include <fcntl.h>
#include "Server.hpp"

using std::string;

void start_server()
{
    webserv::Server server;

    //ListeningSocket(AF_INET, SOCK_STREAM, 0, 80, INADDR_ANY, 10);
    //server.add_socket(AF_INET, SOCK_STREAM, 0, 8001, INADDR_ANY, 10);
    server.add_socket(AF_INET, SOCK_STREAM, 0, 91, INADDR_ANY, 10);
    server.add_socket(AF_INET, SOCK_STREAM, 0, 80, INADDR_ANY, 10);
    server.launch();
    // server.add_socket(AF_INET, SOCK_STREAM, 0, 81, INADDR_ANY, 10);

}

int main(void)
{
    // string d_conf_file;

    // if (argc < 2)
    //     d_conf_file = "./webserv.conf";
    // else
    //     d_conf_file = argv[1];

    // if (access(d_conf_file.c_str(), F_OK) != 0) {
    //     perror("Could not find webserv.conf in current directory");
    //     return -1;
    // }
    start_server();
}

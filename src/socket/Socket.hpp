#ifndef SOCKET_HPP
# define SOCKET_HPP

#include "../config/BaseConfig.hpp"
#include "../colours.h"

#include <exception>
#include <iostream>
#include <netinet/in.h>
#include <cstdlib>
#include <sys/socket.h>
#include <stdio.h>
#include <string>

namespace webserv 
{

/*
 * Socket.
 *
 * The Socket class is used as the base class for creating a socket.
 * It is not meant to be instantiated directly and only meant as a
 * base template for socket types.
 * */

class Socket
{
	public:

		typedef	struct sockaddr_in	sockaddr_t;

		Socket() {};

		/*
		 * Socket Fill Constructor.
		 *
		 * @param 'domain' : specifies the current domain or address family 
		 *	that needs to be used.
		 *		ex: ipv4, ipv6, internal routing domain, etc.
		 * @param 'service' : specifies the type of services that is required
		 *	by the application.
		 *		ex: SOCK_STREAM (virtual circuit services),
		 *			SOCK_DGRAM(datagram services),
		 *			SOCK_RAW(direct ip services).
		 * @param 'protocol' : specifies a praticular protocal to be used
		 *	with the socket.
		 * @param 'port' : the port to connect the socket to.
		 * @param 'interface' : specifies the 4-byte IP address.
		 *
		 * @note : more about protocol sockets
		 * https://www.ibm.com/docs/en/aix/7.2?topic=protocols-socket
		 * */

		Socket(const int &domain, const int &service, const int &protocol,
				const int &port, const u_long &interface);
		virtual ~Socket();

		virtual int	bind_to_network(const int &sock, const sockaddr_t &address) = 0;

		void		test_connection(const int &sock_fd, const std::string &fd_name) const;

		sockaddr_t	*get_address();
		int			get_sock() const;
		int			get_connection_fd() const;
		string		get_client_ip();

		void		set_connection_fd(const int &new_connection_fd);

	private:

		/*
		 * sockaddr_in
		 * https://learn.microsoft.com/en-us/windows/win32/api/ws2def/ns-ws2def-sockaddr_in
		 * */

		sockaddr_t	_address;
		int			_sock;
		int			_connection_fd; //TODO : this is useless remove it and all other stuff related to it later
};

}

#endif

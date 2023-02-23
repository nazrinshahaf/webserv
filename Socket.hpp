#ifndef SOCKET_HPP
# define SOCKET_HPP

#include <iostream>
#include <netinet/in.h>
#include <cstdlib>
#include <sys/socket.h>
#include <stdio.h>

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

		/*
		 * Socket Default Constructor.
		 *
		 * @param 'domain' : specifies the current domain or address family 
		 *	that needs to be used.
		 *		ex: ipv4, ipv6, internal routing domain, etc.
		 * @param 'type' : specifies the type of services that is required
		 *	by the application.
		 *		ex: SOCK_STREAM (virtual circuit services),
		 *			SOCK_DGRAM(datagram services),
		 *			SOCK_RAW(direct ip services).
		 * @param 'protocol' : specifies a praticular protocal to be used
		 *	with the socket.
		 * @port 'port' : the port to connect the socket to
		 *
		 * @note : more about protocol sockets
		 * https://www.ibm.com/docs/en/aix/7.2?topic=protocols-socket
		 * */

		Socket(int domain, int service, int protocol, int port, u_long interface);
		~Socket();

		virtual int	connect_to_network(int sock, sockaddr_t address) = 0;

		void	test_connection(int to_test);

		sockaddr_t	get_address(void) const;
		int			get_sock(void) const;
		int			get_connection_fd(void) const;
		//void		set_connection_fd(const int &new_connection_fd);
	
	private:

		/*
		 * sockaddr_in
		 * https://learn.microsoft.com/en-us/windows/win32/api/ws2def/ns-ws2def-sockaddr_in
		 * */

		sockaddr_t	_address;
		int			_sock;
		int			_connection_fd;
};

}

#endif

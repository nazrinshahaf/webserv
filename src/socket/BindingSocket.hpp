#ifndef BINDINGSOCKET_HPP
# define BINDINGSOCKET_HPP

#include "Socket.hpp"

/*
 * BindingSocket.
 *
 * The BindingSocket class is used to bind a socket to an address.
 * The BindingSocket class inherits from Socket class.
 * */

namespace webserv
{

class BindingSocket : public Socket
{
	public:

		/*
		 * Binding Socket Default Constructor.
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

		BindingSocket() {};
		BindingSocket(const int &domain, const int &service, const int &protocol,
				const int &port, const u_long &interface);
		virtual ~BindingSocket();

		int	bind_to_network(const int &sock, const sockaddr_t &address);
};

}

#endif // !BINDINGSOCKET_HPP

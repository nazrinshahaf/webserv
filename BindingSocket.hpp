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

	BindingSocket(int domain, service, int protocol, int port, u_long interface);
	~BindingSocket();

	int	connect_to_network(int sock, sockaddr_t address);

private:
};

}

#endif // !BINDINGSOCKET_HPP

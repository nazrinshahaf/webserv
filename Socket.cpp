#include "Socket.hpp"

using std::cout;
using std::endl;

/*
 * @note 'socket' : socket is the defualt c way of creating a network socket.
 *	parameters are exactly the same as Socket Constructor.
 *
 * @note 'sin_family' : sin_family will always be AF_INET. Address family
 *	for the transport address.
 * @note 'htons' : htons converts a short from host byte order 
 *	to network byte order.
 * @note 'htonl' : htonl converts a long from host byte order 
 *	to network byte order.
 * */

webserv::Socket::Socket(int domain, int service, int protocol, int port, u_long interface)
{
#ifdef PRINT_MSG
	cout << GREEN "Socket Default Constructor Called" RESET << endl;
#endif

	_address.sin_family = domain;
	_address.sin_port = htons(port); 
	_address.sin_addr.s_addr = htonl(interface);

	_sock = socket(domain, service, protocol);
	test_connection(_sock);
}

webserv::Socket::~Socket()
{
#ifdef PRINT_MSG
	cout << GREEN "Socket Destructor Called" RESET << endl;
#endif
}

void		webserv::Socket::test_connection(int to_test)
{
	if (to_test < 0)
	{
		perror("Failed to connect...");
		exit(EXIT_FAILURE);
	}
	//some success stuff here
}

sockaddr_t	webserv::Socket::get_address(void) const
{
	return (_address);
}

int			webserv::Socket::get_sock(void) const
{
	return (_sock);
}

int			webserv::Socket::get_connection_fd() const
{
	return (_connection_fd);
}

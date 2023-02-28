#include "Socket.hpp"

using std::cout;
using std::endl;
using std::cerr;
using namespace webserv;

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

Socket::Socket(const int &domain, const int &service, const int &protocol,
				const int &port, const u_long &interface)
{
#ifdef PRINT_MSG
	cout << GREEN "Socket Default Constructor Called" RESET << endl;
#endif

	_address.sin_family = domain;
	_address.sin_port = htons(port); 
	_address.sin_addr.s_addr = htonl(interface);

	_sock = socket(domain, service, protocol);
	test_connection(_sock, "Default");
}

Socket::~Socket()
{
#ifdef PRINT_MSG
	cout << RED "Socket Destructor Called" RESET << endl;
#endif
}

void		Socket::test_connection(const int &sock_fd, const std::string &fd_name) const
{
	if (sock_fd < 0)
	{
		cerr << "Failed to connect. "<< endl;
		//exit(EXIT_FAILURE); //maybe throw instead of exit here.
	}
	else
		cout << GREEN << fd_name << " socket fd successfully created" RESET << endl;
}

Socket::sockaddr_t	Socket::get_address() const
{
	return (_address);
}

int			Socket::get_sock() const
{
	return (_sock);
}

int			Socket::get_connection_fd() const
{
	return (_connection_fd);
}

void		Socket::set_connection_fd(const int &new_connection_fd)
{
	_connection_fd = new_connection_fd;
}

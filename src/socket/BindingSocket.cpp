#include "BindingSocket.hpp"
#include "Socket.hpp"
#include "../colours.h"

using namespace webserv;

using std::cout;
using std::endl;

BindingSocket::BindingSocket(const int &domain, const int &service,
		const int &protocol, const int &port, const u_long &interface) :
	Socket(domain, service, protocol, port, interface)
{
#ifdef PRINT_MSG
	cout << GREEN "Binding Socket Default Constructor called" RESET << endl;
#endif // !PRINT_MSG
	
	int	connection_fd = bind_to_network(get_sock(), *get_address());
	set_connection_fd(connection_fd);
	test_connection(get_connection_fd(), "Binding");
}

BindingSocket::~BindingSocket()
{
#ifdef PRINT_MSG
	cout << RED "Binding Socket Destructor called" RESET << endl;
#endif // !PRINT_MSG
}

int	BindingSocket::bind_to_network(const int &sock, const sockaddr_t &address)
{
    return (bind(sock, (struct sockaddr *)&address, sizeof(address)));
}

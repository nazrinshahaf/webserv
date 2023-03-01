#include "ListeningSocket.hpp"

using namespace webserv;

using std::cout;
using std::endl;

ListeningSocket::ListeningSocket(const int &domain, const int &service,
		const int &protocol, const int &port, const u_long &interface,
		const int &backlog) :
			BindingSocket(domain, service, protocol, port, interface),
			_backlog(backlog)
{
#ifdef PRINT_MSG
	cout << GREEN "Listening Default Constructor called" RESET << endl;
#endif // !PRINT_MSG

	start_listening();
	test_connection(get_connection_fd(), "Listening");
}

ListeningSocket::~ListeningSocket()
{
#ifdef PRINT_MSG
	cout << RED "Listening Socket Destructor called" RESET << endl;
#endif // !PRINT_MSG
}

void	ListeningSocket::start_listening(void)
{
	_listening = listen(get_sock(), _backlog);
}

int		ListeningSocket::accept_connections(void) const
{
	int	client_socket_fd;
	//Socket::sockaddr_t	address = get_address();

	//client_socket_fd = accept(get_sock(), (struct sockaddr *)&address, sizeof(struct sockaddr)); //i have to change get_address() to a pointer
	client_socket_fd = accept(get_sock(), NULL, NULL);
	return (client_socket_fd);
}

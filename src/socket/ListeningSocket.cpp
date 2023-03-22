#include "ListeningSocket.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace webserv;

using std::cout;
using std::endl;

ListeningSocket::ListeningSocket(const int &domain, const int &service,
		const int &protocol, const int &port, const u_long &interface,
		const int &backlog, const ServerConfig &config) :
			BindingSocket(domain, service, protocol, port, interface),
			_backlog(backlog), _config(config)
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

int		ListeningSocket::accept_connections(void)
{
	int	client_socket_fd;
	int	addrlen = sizeof(*get_address());

	client_socket_fd = accept(get_sock(), (struct sockaddr *)get_address(), (socklen_t*)&addrlen);
	return (client_socket_fd);
}

const ServerConfig	&ListeningSocket::get_config() const
{
	return (_config);
}

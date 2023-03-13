#ifndef LISTENINGSOCKET_HPP
# define LISTENINGSOCKET_HPP

#include "BindingSocket.hpp"
#include "../config/ServerConfig.hpp"
#include <string>

/*
 * ListeningSocket.
 *
 * The ListeningSocket is used to listen and wait for incoming connections.
 * The ListeningSocket class inherits from the BindingSocket class.
 * */

namespace webserv
{

	class	ListeningSocket : public BindingSocket
	{
		public:

			typedef	struct sockaddr_in				sockaddr_t;
			typedef ServerConfig					config_t;

			/*
			 * Listening Socket Default Constructor.
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
			 * @param 'backlog' : the number of active connections that can
			 *	be in the queue.
			 * @example:
			 * 		ListeningSocket(AF_INET, SOCK_STREAM, 0, 80, INADDR_ANY, 10);
			 * @note : more about protocol sockets
			 * https://www.ibm.com/docs/en/aix/7.2?topic=protocols-socket
			 * */

			ListeningSocket() {};

			ListeningSocket(const int &domain, const int &service, const int &protocol,
					const int &port, const u_long &interface, const int &backlog,
					const ServerConfig &config = ServerConfig());
			~ListeningSocket();

			void	start_listening(void);
			int		accept_connections(void);

			const ServerConfig	&get_config() const;

		private:
			int				_backlog;
			int				_listening;
			ServerConfig	_config;
	};

}

#endif // !LISTENINGSOCKET_HPP

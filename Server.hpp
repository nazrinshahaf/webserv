#include "ListeningSocket.hpp"
#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>

namespace webserv 
{
	class	Server
	{
		public:

			/*
			 * Server Default Constructor.
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
			 * @param 'backlog' : the number of active connections that can
			 *	be in the queue.
			 *
			 * @note : more about protocol sockets
			 * https://www.ibm.com/docs/en/aix/7.2?topic=protocols-socket
			 * */

			Server();
			~Server();

			void		add_socket(const int &domain, const int &service, const int &protocol, const int &port, const u_long &interface, const int &backlog);
			void	launch(void);

			// ListeningSocket	*get_listening_socket(void) const;

		private:
			std::vector<ListeningSocket>	_sockets;
			int								_new_socket;

			void	acceptor(void);
			void	handler(void);
			void	responder(void);
	};
}

#endif // !SERVER_HPP

#ifndef SERVER_HPP
# define SERVER_HPP

#include "ListeningSocket.hpp"
#include "ServerConfig.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ServerNormalDirectiveConfig.hpp"
#include "ServerConfigParser.hpp"
#include "Log.hpp"

#include <vector>
#include <map>
#include <string>
#include <poll.h>


using std::map;
using std::string;

namespace webserv 
{
	class	Server
	{
		public:

			typedef map<int, ListeningSocket>						sockets_type;

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

			Server(const ServerConfigParser &config);
			~Server();

			/*
			 * add_socket.
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
			 * 	server.add_socket(AF_INET, SOCK_STREAM, 0, 80, INADDR_ANY, 10);
			 *
			 * @note : more about protocol sockets
			 * https://www.ibm.com/docs/en/aix/7.2?topic=protocols-socket
			 * */

			void		add_socket(const ServerConfig &server_config,
							const ServerNormalDirectiveConfig &socket_config);
			void		launch(void);

			// ListeningSocket	*get_listening_socket(void) const;

		private:
			sockets_type			_server_sockets;
    		map<int, string>		_client_sockets;
    		map<int, Request>    	_requests;
    		map<int, Response>    	_responses;
			ServerConfigParser		_config;
			static const int		_recv_buffer_size = 65535; //min read bytes

			int		acceptor(ListeningSocket &socket);
			/* int		handler(ListeningSocket &socket); */
			int		receiver(const ListeningSocket &server, const int &const_fd);
			int		responder(const ListeningSocket &server, int client_fd);
	};
}

#endif // !SERVER_HPP

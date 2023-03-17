#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../request/Request.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <exception>
#include "../../socket/ListeningSocket.hpp"
#include "../../config/ServerConfig.hpp"

using std::string;

namespace webserv
{
    class Response
    {
    private:
        Request							_req;
        ListeningSocket                 _server;
        ServerConfig                    _serverConfig;
        string							_root_path;
        int                             _client_fd;
		string							_entireHeader;
		string							_entireBody;
        string							_entireText;
        bool							_hasText;
		int								_error_code;

		void	handle_default_block(void);
		void	handle_location_block(void);

		string	handle_auto_index(const string &path);

		void	read_file(const string &path);
		void	build_header(void);
		void	build_error_body(void);

		string	get_true_root(const ServerLocationDirectiveConfig::map_type &location_block_config) const;
		string	get_true_index(const ServerLocationDirectiveConfig::map_type &location_block_config) const;

		int		is_location_block(void) const;
		int		is_autoindex(void) const;

    public:
        Response();
        Response(const Request &req, ListeningSocket &server, const int &client_fd);
        ~Response();

        void respond(void);
        bool hasText(void);

		class   ResponseException : public std::exception
		{
			public:
				ResponseException(string err_msg) : _err_msg(err_msg) {};
				~ResponseException() _NOEXCEPT {};

				virtual const char  *what(void) const throw() {
					return (_err_msg.c_str());
				};

			private:
				string  _err_msg;
		};
    };
}

#endif

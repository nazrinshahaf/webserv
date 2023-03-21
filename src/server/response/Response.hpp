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
		char							**_envp;

		string	handle_auto_index(string &path);

		void	read_file(const string &path);
		void	build_header(void);
		void	build_error_body(void);

		bool 	path_includes_cgi(void);
		
		string	process_cgi(const string cgi_path);
		string	get_full_path(void);
		string	get_true_root(const ServerLocationDirectiveConfig::map_type &location_block_config) const;
		string	get_true_index(const ServerLocationDirectiveConfig::map_type &location_block_config) const;
		string	find_query_string();
		string	find_path_info();

		string	get_location_path(void) const;
		bool		is_autoindex(void) const;
		bool	is_cgi(void) const;
		bool	is_file(const string path) const;
		bool	check_file_status(const string path) const;

    public:
        Response();
        Response(const Request &req, ListeningSocket &server, const int &client_fd, char **envp);
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

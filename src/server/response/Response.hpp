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

using std::string;

namespace webserv
{
	class Response
	{
	private:
		Request							_req;
		string							_root_path;
		std::map<int, string>::iterator _it;
		string							_entireText;
		bool							_hasText;
		char                            **_envp;

		void        readFile(void);
		string      handle_auto_index(const string &path);
		string      processCgi(void);
		string		find_query_string();
		string		find_path_info();

	public:
		Response();
		Response(const Request &req, const string &root_path, std::map<int, string>::iterator &it, char **envp);
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

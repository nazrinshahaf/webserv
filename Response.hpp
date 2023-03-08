#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Request.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

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

        void	readFile(void);
		string	handle_auto_index(const string &path);
    public:
        Response();
        Response(const Request &req, const string &root_path, std::map<int, string>::iterator &it);
        ~Response();

        void respond(void);
        bool hasText(void);
    };
}

#endif

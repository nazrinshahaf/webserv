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
        Request _req;
        string _root_path;
        std::map<int, string>::iterator _it;

		string	handle_auto_index(const string &path);

    public:
        Response();
        Response(const Request &req, const string &root_path, std::map<int, string>::iterator &it);

        void			respond(void);
    };
}

#endif

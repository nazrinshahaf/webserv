#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Request.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <sys/socket.h>

using std::string;
using std::cout;
using std::endl;

namespace webserv
{
    class Response
    {
    private:
        Request _req;
        string _root_path;
        std::map<int, string>::iterator _it;

    public:
        Response();
        Response(const Request &req, const string &root_path, std::map<int, string>::iterator &it);

        void respond(void);
    };
}

#endif

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
        string _entireText;
        bool    _hasText;
        void readFile(void);
    public:
        Response();
        Response(const Request &req, const string &root_path, std::map<int, string>::iterator &it);
        ~Response();

        void respond(void);
        bool hasText(void);
    };
}

#endif

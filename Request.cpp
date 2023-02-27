#include "Request.hpp"
#include <iostream>
#include <string>
#include <cstring>  
#include <vector>

using namespace webserv;

using std::cout;
using std::string;
using std::endl;


string find_string(std::vector<string> vec, string str)
{
    for (std::vector<string>::iterator it = vec.begin(); it != vec.end(); it++)
    {
        if ((*it).find(str) != string::npos)
            return (*it);
    }
    return "";
    // throw Request::RequiredHeaderParamNotGiven();
}

string find_request_type(std::vector<string> vec) {
    std::vector<string> request_types;
    std::string ret;

    request_types.push_back("GET");
    request_types.push_back("POST");
    request_types.push_back("DELETE");

    for (std::vector<string>::iterator it = request_types.begin(); it != request_types.end(); it++)
    {
        ret = find_string(vec, *it);
        if (ret != string(""))
            return (*it);
    }
    throw Request::RequiredHeaderParamNotGivenException();
}

Request::Request(string request_string)
{
#ifdef PRINT_MSG
	cout << "Request Default Constructor called" << endl;
#endif
    std::vector<string> headers;

    std::cout << "INNN" << std::endl;
    // std::cout << "REQUEST_STR <" : request_string << std::endl;
    while (request_string.find("\n") != request_string.npos)
    {
        string line = request_string.substr(0, request_string.find("\n"));
        std::cout << line << std::endl;
        headers.push_back(line);
        request_string = request_string.substr(request_string.find("\n") + 1);
        std::cout << request_string << std::endl;
    }
    // for (char *ptr = std::strtok((char *)request_string.c_str(), "\n"); ptr != NULL;  ptr = strtok(NULL, "\n"))
    // {
    //     headers.push_back(string(ptr));
    //     std::cout << string(ptr) << std::endl;
    // }

    for (std::vector<string>::iterator it = headers.begin(); it != headers.end(); it++)
        std::cout << *it << std::endl;

    _type = find_request_type(headers);
    std::cout << "REQUEST TYPE " << _type << std::endl;
}

Request::~Request()
{
#ifdef PRINT_MSG
	cout << "Request Destructor Constructor called" << endl;
#endif
}

const string Request::type() { return _type; }

const string Request::host() { return _host; }

const string Request::connection() { return _connection; }

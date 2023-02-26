#include "Request.hpp"
#include <iostream>
#include <string>
#include <cstring>  
#include <vector>

using namespace webserv;

using std::cout;
using std::string;
using std::endl;

Request::Request(string request_string)
{
    std::vector<string> headers;

    for (char *ptr = std::strtok((char *)request_string.c_str(), "\n"); ptr != NULL;  ptr = strtok (NULL, " , "))
        headers.push_back(ptr);

    for (std::vector<string>::iterator it = headers.begin(); it != headers.end(); it++)
        std::cout << *it;
#ifdef PRINT_MSG
	cout << "Request Default Constructor called" << endl;
#endif
}

Request::~Request()
{
#ifdef PRINT_MSG
	cout << "Request Destructor Constructor called" << endl;
#endif
}
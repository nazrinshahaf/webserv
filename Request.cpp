#include "Request.hpp"
#include <iostream>
#include <string>
#include <cstring>  
#include <vector>
#include <map>
#include "colours.h"

using namespace webserv;

using std::cout;
using std::string;
using std::endl;

void Request::find_request_type() {
    std::vector<string> request_types;
    string  type_line = _header_lines[0].substr(0, _header_lines[0].find(" "));

    request_types.push_back("GET");
    request_types.push_back("POST");
    request_types.push_back("DELETE");

    for (std::vector<string>::iterator it = request_types.begin(); it != request_types.end(); it++)
    {
        if (type_line.find(*it) != string::npos)
        {
            _type = *it;
            return;
        }
    }
    throw Request::RequiredHeaderParamNotGivenException();
}

void Request::find_request_path() {
    string first_line = _header_lines[0];

    std::vector<size_t> positions;
    size_t pos = first_line.find(" ", 0);
    while(pos != string::npos)
    {
        positions.push_back(pos);
        pos = first_line.find(" ", pos+1);
    }
    if (positions.size() != 2)
        _path = string("");
    else
        _path = first_line.substr(first_line.find("/"), positions[1] - positions[0] - 1); 
}

void Request::find_request_protocol_version() {
    string first_line = _header_lines[0];

    std::vector<size_t> positions;
    size_t pos = first_line.find(" ", 0);
    while(pos != string::npos)
    {
        positions.push_back(pos);
        pos = first_line.find(" ", pos+1);
    }
    if (positions.size() == 1)
        _protocol_version = first_line.substr(positions[0], first_line.length() - positions[0]);
    else
        _protocol_version = first_line.substr(positions[1], first_line.length() - positions[1]);
}

void Request::parse_request() {
    for (std::vector<string>::iterator line = _header_lines.begin() + 1; line != _header_lines.end(); line++)
    {
        string key = (*line).substr(0, (*line).find(" ") - 1);
        string value = (*line).substr((*line).find(" "), (*line).size() - (*line).find(" "));
        _headers[key] = value;
    }
}

Request::Request(string request_string)
{
#ifdef PRINT_MSG
	cout << "Request Default Constructor called" << endl;
#endif
    std::vector<string> headers;

    while (request_string.find("\n") != request_string.npos && request_string != "\r\n")
    {
        string line = request_string.substr(0, request_string.find("\n"));
        _header_lines.push_back(line);
        request_string = request_string.substr(request_string.find("\n") + 1);
    }
    if (_header_lines.size() < 0 || _header_lines[0].size() <= 1)
	{
        throw std::exception();
	}
    find_request_path();
    find_request_protocol_version();
    find_request_type();
    parse_request();
}

Request::~Request()
{
#ifdef PRINT_MSG
	cout << "Request Destructor Constructor called" << endl;
#endif
}

const std::map<string, string> &Request::headers() const { return _headers; }

const string    Request::to_str() const
{
    string ret;

    ret = string(CYAN) + "REQUEST TYPE: " + string(RESET) + string(YELLOW) + type() + string(RESET) + "\n"\
    + string(CYAN) + "REQUEST PATH: " + string(RESET) + string(YELLOW) + path() + string(RESET) + "\n"\
    + string(CYAN) + "REQUEST PROTOCOL VERSION: " + string(RESET) + string(YELLOW) + protocol_version() + string(RESET) + "\n";
    for (std::map<string, string>::const_iterator it = headers().begin(); it != headers().end(); it++)
    {
        ret += string(CYAN) + it->first + ": " + string(RESET) + string(YELLOW) + it->second + string(RESET) + "\n";
    }
    return (ret);
}

std::ostream &operator<<(std::ostream &os, const webserv::Request &request)
{
    os << CYAN << "REQUEST TYPE: " << RESET << YELLOW << request.type() << RESET << std::endl;
    os << CYAN << "REQUEST PATH: " << RESET << YELLOW << request.path() << RESET << std::endl;
    os << CYAN << "REQUEST PROTOCOL VERSION: " << RESET << YELLOW << request.protocol_version() << RESET << std::endl;
    for (std::map<string, string>::const_iterator it = request.headers().begin(); it != request.headers().end(); it++)
    {
        os << CYAN << it->first << ": " << RESET << YELLOW << it->second << RESET << std::endl;
    }
    return (os);
}

const string &Request::type() const { return _type; }

const string &Request::path() const { return _path; }

const string &Request::protocol_version() const { return _protocol_version; }

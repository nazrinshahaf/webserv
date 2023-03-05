#include "Request.hpp"
#include <iostream>
#include <string>
#include <cstring>  
#include <vector>
#include <map>
#include "colours.h"
#include <string.h>

using namespace webserv;

using std::cout;
using std::endl;
using std::string;

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

bool isNumber(const string &s)
 {
    return s.find_first_not_of("0123456789") == string::npos;
 }

void Request::parse_headers() {
    for (std::vector<string>::iterator line = _header_lines.begin() + 1; line != _header_lines.end(); line++)
    {
        if (*line == "\n")
            break;

        string key = (*line).substr(0, (*line).find(" ") - 1);
        string value = (*line).substr((*line).find(" "), (*line).size() - (*line).find(" "));
        _headers[key] = value;
    }
    // if post and no Content-Length
    // _response_code = 411;
}

void    Request::process_post() {
    if (_headers.find("Content-Length") == _headers.end() || !isNumber(_headers["Content-Length"]))
    {
        _is_done = 1; // TODO response should detect this
        return ;
    }
    if (std::stoi(_headers["Content-Length"]) >= 1
        && _body.size() == std::stoul(_headers["Content-Length"]))
        _is_done = true;
    // std::vector<string>::iterator line = _header_lines.begin();
    // std::cout << "start" << std::endl;
    // while (line != _header_lines.end())
    // {
    //     std::cout << *line << std::endl;
    //     for (string::iterator it = (*line).begin(); it != (*line).end(); it++)
    //         _body.push_back(*it);
    //     line++;
    // }
    // std::cout << "end" << std::endl;
}

Request::Request(string request_string, int socket) : _body(""), _socket(socket), _is_done(false)
{
#ifdef PRINT_MSG
	cout << "Request Default Constructor called" << endl;
#endif
    while (request_string.find("\n") != request_string.npos && request_string != "\r\n")
    {
        string line = request_string.substr(0, request_string.find("\n"));
        _header_lines.push_back(line);
        request_string = request_string.substr(request_string.find("\n") + 1);
        if (request_string.find("\r\n") == 0)
        {
            request_string = request_string.substr(request_string.find("\n") + 1);
            _body = request_string;
            break;
        }
    }
    if (_header_lines.size() < 1 || _header_lines[0].size() <= 1)
	{
        throw std::exception();
	}
    find_request_path();
    find_request_protocol_version();
    find_request_type();
    parse_headers();
    if (type() == "POST")
        process_post();
    if (type() == "GET" || type() == "DELETE")
        _is_done = 1;
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

void  Request::add_body(string buffer)
{
    if (type() != "POST")
        return ;
    if (_headers.find("Content-Length") == _headers.end())
        return ;
    if (std::stoul(_headers["Content-Length"]) == _body.size())
        _is_done = true;
    for (string::iterator it = buffer.begin(); it != buffer.end(); it++)
    {
        _body.push_back(*it);
    }
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


bool      Request::done() { return _is_done; }

const string    &Request::body() const { return _body; }

const string &Request::type() const { return _type; }

const string &Request::path() const { return _path; }

const int   &Request::socket() const { return _socket; }

const string &Request::protocol_version() const { return _protocol_version; }

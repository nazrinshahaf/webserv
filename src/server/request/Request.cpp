#include "Request.hpp"
#include "../../log/Log.hpp"
#include "../../utils/Utils.hpp"
#include <cstddef>
#include <exception>
#include <iostream>
#include <string>
#include <cstring>  
#include <vector>
#include <map>
#include "../../colours.h"
#include <string.h>
#include <fstream>

using namespace webserv;

using std::cout;
using std::endl;
using std::string;

int Request::find_request_type() {
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
            return (1);
        }
    }
    return (0);
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
        pos = first_line.find(" ", pos + 1);
    }

	if (positions.size() == 0) //if Request type is the only parameter.
		;
	else if (positions.size() == 1)
		_protocol_version = first_line.substr(positions[0], first_line.length() - positions[0]);
	else
		_protocol_version = first_line.substr(positions[1], first_line.length() - positions[1]);
}

void Request::parse_headers()
{
    for (std::vector<string>::iterator line = _header_lines.begin() + 1; line != _header_lines.end(); line++)
    {
        if (*line == "\r\n")
            break;

        string key = line->substr(0, line->find(" ") - 1);
        string value = (*line).substr((*line).find(" ") + 1, (*line).size() - (*line).find(" ") - 1); // -1 is for invisible characters at the end of every line in a http header
        _headers[key] = value;
    }
    // if post and no Content-Length
    // _response_code = 411;
}

void    Request::process_post()
{
	if (_headers.find("Transfer-Encoding") != _headers.end() && _headers["Transfer-Encoding"] == string("chunked"))
        _chunked = true;
	else if (_headers.find("Content-Length") == _headers.end() || !utils::isNumber(_headers["Content-Length"]))
    {
		cout << "no content length found" << endl;
        _is_done = true; //this changes to error_code
    }
	else if (std::stoi(_headers["Content-Length"]) >= 1
			&& _body.size() == std::stoul(_headers["Content-Length"]))
		_is_done = true;
}

void	Request::read_header(string request_string)
{
	//if no header content yet and first characters are \r\n (ignoring empty lines on first request)

	string test = request_string;
	utils::replaceAll(test, "\r\n", "\\r\\n\n");

	if (_header_lines.size() == 0 && request_string.find("\r\n") == 0) // can merge in while loop
		return ;

    while (request_string.find("\r\n") != request_string.npos)
    {
		if (_header_lines.size() == 1)
		{
			if (!find_request_type())
			{
				_bad_request = true;
				return ;
			}
			find_request_path();
			find_request_protocol_version();
		}
        if (request_string.find("\r\n") == 0)
        {
			Log(DEBUG, "header done");
			_header_done = true;
            request_string = request_string.substr(request_string.find("\r\n") + 2);
            _body = request_string;
            break;
        }
        string line = request_string.substr(0, request_string.find("\r\n"));
        _header_lines.push_back(line);
        request_string = request_string.substr(request_string.find("\r\n") + 2);
    }

	if (!header_done())
		return;

    parse_headers();
    if (type() == "POST")
        process_post();
    if (type() == "GET" || type() == "DELETE")
        _is_done = true;
}

Request::Request(string request_string, int socket) :
	_body(""), _socket(socket),
	_is_done(false), _header_done(false), _bad_request(false), _chunked(false), _chunk_length(0)
{
#ifdef PRINT_MSG
	cout << "Request Assignment Constructor called" << endl;
#endif
	read_header(request_string);
}

Request::Request() :
	_is_done(false), _header_done(false), _bad_request(false)
{
#ifdef PRINT_MSG
	cout << "Request Default Constructor called" << endl;
#endif
}

Request::Request(const Request &to_copy) :
	_headers(to_copy._headers), _header_lines(to_copy._header_lines),
	_body(to_copy._body), _type(to_copy._type), _path(to_copy._path),
	_protocol_version(to_copy._protocol_version), _socket(to_copy._socket),
	_is_done(to_copy._is_done), _header_done(to_copy._header_done),
	_bad_request(to_copy._bad_request)

{
#ifdef PRINT_MSG
	cout << "Request Copy Constructor called" << endl;
#endif
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
        ret += string(CYAN) + it->first + ": " + string(RESET) + string(YELLOW) + it->second + string(RESET) + "\n";
    return (ret);
}

string  Request::process_chunk(string buffer)
{
    double  chars_to_read;

	if (_chunk_length == 0)
    {
        try
        {
	     _chunk_length = std::stod(buffer.substr(0, buffer.find("\r\n")).c_str());
        }
        catch(const std::exception& e)
        {
            Log(ERROR, "Chunk request received in improper format", __LINE__, __PRETTY_FUNCTION__, __FILE__);
            _bad_request = true;
        }
        
    }
    chars_to_read = _chunk_length;
	buffer = buffer.substr(buffer.find("\r\n") + 2, chars_to_read);

	if (chars_to_read == 0 && buffer.length() == 0)
		_is_done = true;
    _chunk_length -= buffer.length();
	return (buffer);
}

void  Request::add_body(string buffer)
{
	if (type() != "POST")
		return ;

	if (is_chunked())
		buffer = process_chunk(buffer);
	else if (_headers.find("Content-Length") == _headers.end())
		return ;

	if (!is_chunked() && std::stoul(_headers["Content-Length"]) == _body.size())
		_is_done = true;
    _body.append(buffer, 0, buffer.length());
	
    if (!is_chunked() && std::stoul(_headers["Content-Length"]) == _body.size())
        _is_done = true;
	cout << "body length : " << _body.length() << endl;
}


std::ostream &operator<<(std::ostream &os, const webserv::Request &request)
{
    os << CYAN << "REQUEST TYPE: " << RESET << YELLOW << request.type() << RESET << std::endl;
    os << CYAN << "REQUEST PATH: " << RESET << YELLOW << request.path() << RESET << std::endl;
    os << CYAN << "REQUEST PROTOCOL VERSION: " << RESET << YELLOW << request.protocol_version() << RESET << std::endl;
    for (std::map<string, string>::const_iterator it = request.headers().begin(); it != request.headers().end(); it++)
        os << CYAN << it->first << ": " << RESET << YELLOW << it->second << RESET << std::endl;
    return (os);
}


bool			Request::done() { return _is_done; }

bool			Request::header_done() { return _header_done; }

bool			Request::bad_request() { return _bad_request; }

bool			Request::is_chunked() const {return _chunked; }

string			&Request::body() { return _body; }

const string	&Request::type() const { return _type; }

const string	&Request::path() const { return _path; }

const int		&Request::socket() const { return _socket; }

const string	&Request::protocol_version() const { return _protocol_version; }

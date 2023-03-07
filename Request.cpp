#include "Request.hpp"
#include <cstddef>
#include <iostream>
#include <string>
#include <cstring>  
#include <vector>
#include <map>
#include "colours.h"
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

bool isNumber(const string &s)
 {
    return s.find_first_not_of("0123456789") == string::npos;
 }

void Request::parse_headers()
{
    for (std::vector<string>::iterator line = _header_lines.begin() + 1; line != _header_lines.end(); line++)
    {
        if (*line == "\n")
            break;

        string key = (*line).substr(0, (*line).find(" ") - 1);
        string value = (*line).substr((*line).find(" ") + 1, (*line).size() - (*line).find(" ") - 2); // -2 is for invisible characters at the end of every line in a http header
        _headers[key] = value;
    }
    // if post and no Content-Length
    // _response_code = 411;
}

void    Request::process_post()
{
    if (_headers.find("Content-Length") == _headers.end() || !isNumber(_headers["Content-Length"]))
    {
        _is_done = 1; // TODO response should detect this
        return ;
    }
    if (std::stoi(_headers["Content-Length"]) >= 1
        && _body.size() == std::stoul(_headers["Content-Length"]))
        _is_done = true;
}

static string find_filename(string line)
{
    string temp = line;
    string data = "";
    std::map<string, string> temp_dict;

    while (temp.size() > 0)
    {
        if (temp.find("; ") == string::npos){
            data = temp;
            temp = temp.erase(0, data.length());
        }
        else{
            data = temp.substr(0, temp.find("; ") + 1);
            temp = temp.erase(0, data.length() + 1);
        }
        if (temp.size() >= 2 && temp[0] == '\r' && temp[1] == '\n')
            break;
        if (data.find("=") != string::npos)
        {
            string key = data.substr(0, data.find("="));
            string value = data.substr(data.find("=") + 1, (data.length() - data.find("=")));
            value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
            value.erase(std::remove(value.begin(), value.end(), ';'), value.end());
            temp_dict[key] = value;
        }
    }
    if (temp_dict.find("filename") != temp_dict.end())
        return (temp_dict.find("filename")->second);
    return ("nofilename");
}

void    Request::process_image()
{
    if (_headers.find("Content-Type") == _headers.end())
        return ;
    if (_headers["Content-Type"].find("multipart/form-data;") == 0)
    {
        string boundary;
        string body = _body;
        std::vector<string> datas;

        boundary = "--" + _headers["Content-Type"].substr(_headers["Content-Type"].find("=") + 1, 38);
        while (body.find(boundary) != string::npos)
        {
            string data = body.substr(boundary.length() + 2, body.find(boundary, 1) - boundary.length() - 2);
            body = body.erase(0, boundary.length() + data.length() + 2);
            datas.push_back(data);
        }

        for (std::vector<string>::iterator data = datas.begin(); data != datas.end(); data++)
        {
            string headers = *data;
            std::map<string, string> temp_dict;

            while (headers.find("\n") != string::npos)
            {
                string line =  (headers.substr(0, headers.find("\n")));
                if (line.find(":") != string::npos)
                {
                    string key = (line).substr(0, (line).find(" ") - 1);
                    string value = (line).substr((line).find(" ") + 1, (line).size() - (line).find(" ") - 2);
                    temp_dict[key] = value;
                }
                headers = headers.erase(0, headers.find("\n") + 1);
                if (headers.size() >= 2 && headers[0] == '\r' && headers[1] == '\n') {
                    temp_dict["body"] = headers.substr(2, headers.length() - 6);
                    break;
                }
            }

            if (temp_dict.find("Content-Type") != temp_dict.end() &&
                (temp_dict.find("Content-Type")->second == "image/png" ||
                 temp_dict.find("Content-Type")->second == "image/jpeg"))
            {

                string filename = find_filename(temp_dict["Content-Disposition"]);
                std::ofstream out(filename);
                out << temp_dict["body"];
                out.close();
            }
        }
    }
}

Request::Request(string request_string, int socket) : _body(""), _socket(socket), _is_done(false), _bad_request(false)
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
    if (_header_lines.size() < 1 || _header_lines[0].size() <= 2)
	{
        _bad_request = true;
        return ;
	}
    if (!find_request_type())
    {
        _bad_request = true;
        return ;
    }
    find_request_path();
    find_request_protocol_version();
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
        ret += string(CYAN) + it->first + ": " + string(RESET) + string(YELLOW) + it->second + string(RESET) + "\n";
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
        os << CYAN << it->first << ": " << RESET << YELLOW << it->second << RESET << std::endl;
    return (os);
}


bool      Request::done() { return _is_done; }

bool	  Request::bad_request() { return _bad_request; }

string    &Request::body() { return _body; }

const string &Request::type() const { return _type; }

const string &Request::path() const { return _path; }

const int   &Request::socket() const { return _socket; }


const string &Request::protocol_version() const { return _protocol_version; }

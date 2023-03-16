#include "Response.hpp"
#include "../../log/Log.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <sys/_types/_size_t.h>
#include <sys/_types/_ssize_t.h>
#include <sys/dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <sstream>
#include "../../Utils.cpp"

#define _XOPEN_SOURCE 700 //for autoindex
#define _GNU_SOURCE //for checking audoindex file type

using namespace webserv;

using std::cout;
using std::endl;

Response::Response()
{
    _hasText = false;
}

Response::Response(const Request &req, ListeningSocket &server, const int &client_fd) : 
	_req(req), _server(server), _client_fd(client_fd), _hasText(true)
{
	_serverConfig = server.get_config();
	try //try to find root path (this should be in responder)
	{
		_root_path = _serverConfig.find_normal_directive("root").get_value();
	}
	catch (std::exception &e)
	{
		Log(WARN, string(e.what()), 2, server.get_config());
		_root_path = "/";
	}
    this->readFile();
}

Response::~Response()
{
    
}

bool Response::hasText(void) { return (_hasText); }

void Response::readFile(void)
{
    const char *header = "HTTP/1.1 200 OK\nContent-Type: */*\n\n"; // Dynamically add content length TODO
    const char *header2 = "HTTP/1.1 200 OK\r\nContent-Type: image/*\r\n\r\n";
    const char *header3 = "HTTP/1.1 200 OK\r\nContent-Type: video/mp4\r\n\r\n";
    const char *header_404 = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n";

    std::ifstream myfile;

	string	header_and_text;
	string	full_path = _root_path + _req.path();
	utils::replaceAll(full_path, "%20", " ");

	cout << "full path = " << full_path << endl;
	if (_req.path() == "/")
	{
		myfile.open(_root_path + "/index.html", std::ios::binary);
	}
	else if (_req.path() == "/autoindex") //reemove hard code later
	{
		_entireText += header;
		_entireText += handle_auto_index(_root_path);
	}
	else
	{
		struct stat	dir_stat;
		if (stat(full_path.c_str(), &dir_stat) == 0) //if successfully stat
		{
			if (dir_stat.st_mode & S_IFDIR) //if dir
				myfile.open("public/404.html", std::ios::binary);
			else //if not dir
				myfile.open(full_path, std::ios::binary);
		}
		else
			Log(ERROR, "CANNOT OPEN DIR");
	}

    if (!myfile)
    {
        _entireText += header_404;
        myfile.open("public/404.html", std::ios::binary);
    }
    else
    {
        if (full_path.find(".jpg") != string::npos || full_path.find(".png") != string::npos)
            _entireText += header2;
        else if (full_path.find(".mp4") != string::npos)
            _entireText += header3;
        else
            _entireText += header;
    }

	/* cout << "before read buffer" << endl; */
    char read_buffer[65535]; // create a read_buffer
    while (myfile.read(read_buffer, sizeof(read_buffer)))
        _entireText.append(read_buffer, myfile.gcount());
    _entireText.append(read_buffer, myfile.gcount());
	///* if (_req.path().find(".jpg") != string::npos || _req.path().find(".png") != string::npos) */
	///* { */
	///* 	/* header_and_text += "Content-Length: "; */
	///* 	/* header_and_text += _entireText.length(); */
	///* 	cout << header_and_text << endl; */
	///* 	_entireText = header_and_text + _entireText; */
	///* } */
	///* cout << "before close" << endl; */
    myfile.close();
	/* cout << "after close" << endl; */
    cout << "WHOLE STRING IS: " << _entireText.length() << endl;
    cout << "BODY IS: " << _entireText.length() - strlen(header2) << endl;
}

void Response::respond(void)
{
    // cout << "I"
    if (_req.done())
    {
        if (_req.path() == "/upload.html")
            _req.process_image();
        ssize_t total_to_send = _entireText.length();
		cout << "total to send :" << total_to_send << endl;

		ssize_t sent = send(_client_fd, _entireText.c_str(), _entireText.length(), 0);
		if (sent == 0)
			cout << "SENT IS 0" << endl;
		if (sent == -1)
		{
			std::cerr << "send err : " << errno << endl;
			return ;
		}
		if (sent != (ssize_t)_entireText.length())
		{
			cout << "amount sent:" << sent << endl;
			_entireText = _entireText.substr(sent);
		}
		else
			_hasText = false;
    }
}

/*
 * change this later pls..
 * @note : DONT LOOK AT THIS PLS
 * */

static string	auto_index_apply_syle(void)
{
	//return (" html { min-height: 100%; text-align: center; display: flex; justify-content: center; flex-direction: column; } body { font-family: Arial, sans-serif; margin: 0; padding: 0; text-align: center; min-height: 100%; } h1 { text-align: center; } table { border: none; border-bottom: 1px solid black; border-collapse: collapse; margin: 0 auto; width: 600px; } thead { border-bottom: 1px solid black; } th, td { /* border: 1px solid black; */ padding: 10px; text-align: left; } td a { font-weight: 600; }");
	return("<link rel=\"stylesheet\" href=\"https://drive.google.com/uc?export=view&id=1ZCGfFPqxAPPh66miEdAjFlYmeC8krMjc\">");
}

static string		auto_index_create_header(const string &path)
{
	string header;
	header += "<div class=\"header\">";
	header += "<div class=\"box\">";
	header += "<h1 class=\"box-text-1\">";
	header += "AutoIndex of";
	header += "</h1>";
	header += "<p class=\"box-text-2\">";
	header += path;
	header += "</p>";
	header += "</div>";
	header += "</div>";
	return (header);
}

static string		auto_index_create_html(const int &type, const string &path = string())
{
	string	header;

	if (type == 0) //header
	{
		header += "<!DOCTYPE html>";
		header += "<html>";
		header += "<head>";
		/* header += "<style>" + auto_index_apply_syle() + "</style>"; */
		header += auto_index_apply_syle();
		header += "</head>";
		header += "<body>";
		header += auto_index_create_header(path);
		header += "<table>";
		header += "<thead>";
		header += "<tr>";
		header += "<th>Name</th>";
		header += "<th>Last Modified</th>";
		header += "<th>Size</th>";
		header += "</tr>";
		header += "</thead>";
		header += "<tbody>";
	}
	else if (type == 1) //footer
	{
		header += "</tbody>";
		header += "</table>";
		header += "<body>";
		header += "<html>";
	}
	return (header);
}

static string			auto_index_print_last_modified(const string &path, const string &file_name)
{
	struct stat attr;
	string		full_path = path + "/" + file_name;

	stat(full_path.c_str(), &attr);
	return (string(ctime(&attr.st_mtime)));
}

static string			auto_index_print_file_size(const string &path, const string &file_name)
{
	struct stat attr;
	string		full_path = path + "/" + file_name;

	stat(full_path.c_str(), &attr);
	if (attr.st_size > 1000)
		return (std::to_string(attr.st_size/1000) + "MB");
	return (std::to_string(attr.st_size) + "B");
}

static string			auto_index_create_file(const string &path, const int &file_type, const string &file_name)
{
	string	file_str;
	string	full_path = path + "/" + file_name;

	if (file_type == 1) //everything else (mainly file)
	{
		file_str += "<tr>";
		file_str += "<td><a href=\"" + full_path + "\">" + file_name + "</a></td>";
		file_str += "<td>" + auto_index_print_last_modified(path, file_name) + "</td>";
		file_str += "<td>" + auto_index_print_file_size(path, file_name) + "</td>";
		file_str += "</tr>";
	}
	else if (file_type == 0) //directory
	{
		string	file_name_with_slash = (file_name == "." || file_name == "..") ? file_name : file_name + "/";
		file_str += "<tr>";
		file_str += "<td><a href=\"" + full_path + "\">" + file_name_with_slash + "</a></td>";
		file_str += "<td>" + auto_index_print_last_modified(path, file_name) + "</td>";
		file_str += "<td>-</td>";
		file_str += "</tr>";
	}
	return (file_str);
}

std::string	Response::handle_auto_index(const string &path)
{
	string						auto_index_html;
	DIR							*directory;
	struct dirent				*file;     
	std::multimap<int, string>	sorted_mmap;

	auto_index_html += auto_index_create_html(0, path);
	directory = opendir(path.c_str());
	if (directory != NULL)
	{
		while((file = readdir(directory)) != NULL)
		{
			int	file_type = (file->d_type == DT_REG) ? 1 : 0;
			sorted_mmap.insert(std::make_pair(file_type, string(file->d_name)));
		}
		(void)closedir(directory);
	}
	else
		perror("Couldn't open the directory");
	for (std::multimap<int,string>::iterator it = sorted_mmap.begin(); it != sorted_mmap.end(); it++) {
		auto_index_html += auto_index_create_file(path, it->first, it->second);
	}
	auto_index_html += auto_index_create_html(1);
	return (auto_index_html);
}

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

Response::Response() : _hasText(false)
{
}

Response::Response(const Request &req, ListeningSocket &server, const int &client_fd) : 
	_req(req), _server(server), _client_fd(client_fd), _hasText(true)
{
	if (_req.bad_request())
	{
		_error_code = 400;
		build_header();
		Log(DEBUG, "BAD REQUEST RECEIVED");
		send(client_fd , _entireHeader.c_str() , _entireHeader.length(), MSG_OOB);
		_hasText = false;
	}

	_serverConfig = server.get_config();
	_error_code = 0;
	try //try to find root path (this should be in responder)
	{
		_root_path = _serverConfig.find_normal_directive("root").get_value();
	}
	catch (std::exception &e)
	{
		Log(WARN, string(e.what()), 0, NULL, NULL, 2, server.get_config());
		_root_path = "/";
	}

    /* this->readFile(); */
	if (_req.type() == "GET")
	{
		if (is_location_block() == 1)
			handle_location_block();
		else
			handle_default_block();
		build_header();
		_entireText = _entireHeader + _entireBody;
	}
	if (_req.type() == "POST")
	{
	}
}

Response::~Response()
{
    
}

string	Response::get_true_root(const ServerLocationDirectiveConfig::map_type &location_block_config) const
{
	string	true_root;

	if (location_block_config.find("root") != location_block_config.end()) //if root exist in location
		true_root = location_block_config.find("root")->second;
	else //if root doesnt exist in location
		true_root = _serverConfig.find_normal_directive("root").get_value(); //can sub to _root_path if we use it often
	return (true_root);
}

string	Response::get_true_index(const ServerLocationDirectiveConfig::map_type &location_block_config) const
{
	string	true_index;

	if (location_block_config.find("index") != location_block_config.end()) //if index exist in location
		true_index = location_block_config.find("index")->second;
	else //if index doesnt exist in location
		true_index = _serverConfig.find_normal_directive("index").get_value(); //can sub to _root_path if we use it often
	return (true_index);
}

int		Response::is_location_block(void) const
{
	try {
		ServerLocationDirectiveConfig location_block = _serverConfig.find_location_directive(_req.path());
		return (1);
	} catch (BaseConfig::ConfigException &e) {
		return (0);
	}
}

int		Response::is_autoindex(void) const
{
	try {
		ServerLocationDirectiveConfig location_block = _serverConfig.find_location_directive(_req.path());

		if (location_block.get_config().find("autoindex") == location_block.get_config().end())
			return (0);
		return (1);
	} catch (BaseConfig::ConfigException &e) {
		return (0);
	}
}

void	Response::handle_location_block(void)
{
	string	full_path = _root_path + _req.path();

	utils::replaceAll(full_path, "%20", " ");

	cout << "full path = " << full_path << endl;
	Log(DEBUG, "Path = " + _req.path());
	Log(DEBUG, "Full path = " + full_path);

	//if req path is default
	if (_req.path() == "/")
		cout << "Server Default" << endl;

	//check if req path is a location block.
	try {
		ServerLocationDirectiveConfig location_block = _serverConfig.find_location_directive(_req.path());
		ServerLocationDirectiveConfig::map_type	location_block_config = location_block.get_config();
		Log(DEBUG, "URL path is a location");

		full_path = get_true_root(location_block_config) + "/";
		if (is_autoindex() == 1) //if autoindex
		{
			_entireBody += handle_auto_index(full_path);
			return ;
		}
		Log(WARN, "full_path : " + full_path);
		full_path += get_true_index(location_block_config);
		Log(WARN, "full_path : " + full_path);
		read_file(full_path);
	} catch (BaseConfig::ConfigException &e) { //no location just search root directory
		Log(DEBUG, "URL path is not location");
	}
}

void	Response::handle_default_block(void)
{
	string	full_path;

	full_path += _serverConfig.find_normal_directive("root").get_value() + "/";
	if (_req.path() == "/")
		full_path += _serverConfig.find_normal_directive("index").get_value();
	else
		full_path += _req.path();
	read_file(full_path);
}

void	Response::build_header(void)
{
    //const char *header = "HTTP/1.1 200 OK\nContent-Type: */*\n\n"; // Dynamically add content length TODO

	_entireHeader = "HTTP/1.1 ";
	if (_error_code)
	{
		_entireHeader += std::to_string(_error_code) + " ";
		switch (_error_code)
		{
			case (400): _entireHeader += "Bad Request"; break;
			case (403): _entireHeader += "Forbidden"; break;
			case (404): _entireHeader += "Not Found"; break;
			default: _entireHeader += "Not OK"; break;
		}
	}
	else
		_entireHeader += "200 OK";
	_entireHeader += "\r\n";
	_entireHeader += "Content-Type: */*\r\n";
	_entireHeader += "Content-Length: " + std::to_string(_entireBody.length()) + "\r\n\r\n";
}


void Response::read_file(const string &path) //change name later
{
    std::ifstream file;
	string path_no_spaces = path;
	struct stat	dir_stat;

	utils::replaceAll(path_no_spaces, "%20", " ");
	Log(WARN, "in read_file : [" + path_no_spaces + "]");

	if (stat(path_no_spaces.c_str(), &dir_stat) == 0) //if successfully stat
	{
		if (dir_stat.st_mode & S_IFDIR) //if dir
		{
			Log(ERROR, "FILE IS A DIRECTORY HANDLE OPEN NEW PATH", __LINE__, __PRETTY_FUNCTION__, __FILE__);
			_error_code = 404;
			file.open("public/404.html", std::ios::binary);
		}
		else //if not dir
		{
			Log(DEBUG, "File opened");
			file.open(path_no_spaces, std::ios::binary);
		}
	}
	else
		Log(ERROR, "CANNOT OPEN FILE SOMETHING VERY BAD HAPPENED", __LINE__, __PRETTY_FUNCTION__, __FILE__);

    if (!file)
    {
		_error_code = 404;
        file.open("public/404.html", std::ios::binary);
    }

    char read_buffer[65535]; // create a read_buffer
    while (file.read(read_buffer, sizeof(read_buffer)))
        _entireBody.append(read_buffer, file.gcount());
    _entireBody.append(read_buffer, file.gcount());

	file.close();
}

void Response::respond(void)
{
    if (_req.done())
    {
        if (_req.path() == "/upload.html") //move later
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

bool Response::hasText(void) { return (_hasText); }

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

	cout << path << endl;
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

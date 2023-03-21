#include "Response.hpp"
#include "../../log/Log.hpp"
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <sys/_types/_s_ifmt.h>
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
#include <unistd.h>
#include <fcntl.h>

#include "../../utils/Utils.hpp"

#define _XOPEN_SOURCE 700 //for autoindex
#define _GNU_SOURCE //for checking audoindex file type

using namespace webserv;

using std::cout;
using std::endl;

Response::Response() : _hasText(false)
{
}

Response::Response(const Request &req, ListeningSocket &server, const int &client_fd, char **envp) : 
	_req(req), _server(server), _serverConfig(server.get_config()),
	_client_fd(client_fd), _entireHeader(""), _entireBody(""), _entireText(""),
	_hasText(true), _error_code(0), _envp(envp) //change later when req.bad_req is int
{
	_root_path = _serverConfig.find_normal_directive("root").get_value(); //change again
	if (_req.bad_request())
	{
		Log(INFO, "Bad Request Recieved");
		_error_code = 400;
		build_error_body();
		build_header();
		_entireText = _entireHeader + _entireBody;
		/* Log(DEBUG, std::to_string(_req.done())); */
		return;
	}

	if (_req.type() == "GET")
	{
		string full_path = get_full_path();

		/* Log(WARN, "Auto index = " + std::to_string(is_autoindex())); */
		cout << "full_path : " << full_path << endl;
		if (is_autoindex() && !is_file(full_path))
		{
			cout << "in 1" << endl;
			_entireBody = handle_auto_index(full_path);
		}
		else if (is_cgi())
		{
			cout << "in 2" << endl;
			Log(DEBUG, "PROCESS IS CGI");
			_entireBody = process_cgi(full_path);
		}
		else if (!is_redirect())
		{
			cout << "in 3" << endl;
			read_file(full_path);
		}
		if (_error_code != 0)
			build_error_body();
		build_header();
		_entireText = _entireHeader + _entireBody;
		cout << _entireHeader << endl;
	}
	else if (_req.type() == "POST")
	{
		string	full_path = get_full_path();

		Log(DEBUG, "POST IS CGI");
		if (is_cgi())
			_entireText = process_cgi(full_path);
		if (_error_code != 0)
			build_error_body();
		build_header();
	}
}

Response::~Response()
{
    
}

string	Response::get_full_path(void)
{
	string	full_path;
	string	location_path = get_location_path();

	string	is_url_file = _root_path + _req.path();
	if (is_file(is_url_file))
	{
		Log(DEBUG, "url is file");
		return (is_url_file);
	}

	/* Log(DEBUG, "Location path : " + location_path); */
	/* Log(DEBUG, "url_path : " + _req.path()); */
	if (_req.path() == "/" || _req.path() == "")
	{
		full_path = _serverConfig.find_normal_directive("root").get_value();
		Log(WARN, "true root in / : " + full_path);
		/* if (is_autoindex()) */
		/* 	return full_path; */
		full_path += "/" + _serverConfig.find_normal_directive("index").get_value();
	}
	else if (location_path != "")
	{
		try {
			ServerLocationDirectiveConfig location_block = _serverConfig.find_location_directive(location_path);
			ServerLocationDirectiveConfig::map_type	location_block_config = location_block.get_config();
			Log(DEBUG, "URL path is a location");

			full_path = _req.path();
			string true_root = get_true_root(location_block_config);
			cout << "url_path : " << full_path << endl;
			cout << "location_path : " << location_path << endl;
			if (true_root.back() == '/')
				true_root.pop_back();
			cout << "true_root : " << true_root << endl;
			cout << "full_path : " << full_path << endl;
			if (!is_autoindex())
			{
				full_path = get_true_root(location_block_config);
				if (full_path.back() != '/')
					full_path += "/";
				full_path += get_true_index(location_block_config);
				return (full_path);
			}
			cout << "full_path with index : " << full_path << endl;
			utils::replaceAll(full_path, location_path, true_root);
		} catch (BaseConfig::ConfigException &e) { //no location just search root directory
			Log(ERROR, "URL path is not location (i dont think the error should show up here)", __LINE__, __PRETTY_FUNCTION__, __FILE__);
		}
	}
	else
	{
		full_path += _serverConfig.find_normal_directive("root").get_value();
		full_path += _req.path();
	}
	return (full_path);
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

string	Response::get_location_path(void) const
{
	pair<ServerConfig::cit_t, ServerConfig::cit_t> range = _serverConfig.find_values("location");

	string longest_match;
	for (ServerConfig::cit_t locations = range.first; locations != range.second; locations++)
	{
		ServerLocationDirectiveConfig l = dynamic_cast<ServerLocationDirectiveConfig&>(*locations->second);

		string path = l.get_path();
		if (_req.path().find(path) == 0)
			if (path.length() > longest_match.length())
				longest_match = path;
	}
	return (longest_match);
}

bool	Response::is_autoindex(void) const
{
	string location_path = get_location_path();
	if (location_path == "")
	{
		try {
			_serverConfig.find_normal_directive("autoindex");
			return (true);
		} catch (BaseConfig::ConfigException &e) {
			return (false);
		}
	}
	else
	{
		try {
			ServerLocationDirectiveConfig location_block = _serverConfig.find_location_directive(location_path);

			if (location_block.get_config().find("autoindex") == location_block.get_config().end())
				return (false);
			return (true);
		} catch (BaseConfig::ConfigException &e) {
			return (false);
		}
	}
}
void	Response::build_header(void)
{
	_entireHeader = "HTTP/1.1 ";
	if (is_redirect()) // todo: dynamically redir
	{
		_entireHeader += "301 Moved Permanently\r\nLocation: " + get_redirected_path() + "\r\n\r\n";
		return;
	}
	else if (_error_code)
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
	if (_entireBody.length() != 0)
		_entireHeader += "Content-Length: " + std::to_string(_entireBody.length()) + "\r\n\r\n";
}

void Response::build_error_body()
{
	try {
		pair<ServerConfig::cit_t, ServerConfig::cit_t> error_pages = _serverConfig.find_values("error_page");
		for (ServerConfig::cit_t error_page = error_pages.first; error_page != error_pages.second; error_page++)
		{
			ServerNormalDirectiveConfig err_p = dynamic_cast<ServerNormalDirectiveConfig&>(*error_page->second);

			if (std::stoi(err_p.get_value()) == _error_code)
			{
				int save_err_code = _error_code;
				Log(DEBUG, "Same error code");
				read_file(_root_path + "/" + err_p.get_value2());
				_error_code = save_err_code;
			}
		}
	} catch (BaseConfig::ConfigException &e) {
		Log(ERROR, "Error page not found:", __LINE__, __PRETTY_FUNCTION__, __FILE__);
		return;
	}
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
			Log(ERROR, "File is a directory", __LINE__, __PRETTY_FUNCTION__, __FILE__);
			_error_code = 403;
			/* file.open("public/404.html", std::ios::binary); */
		}
		else //if not dir
		{
			Log(DEBUG, "File " + path_no_spaces + " opened");
			file.open(path_no_spaces, std::ios::binary);
		}
	}
	else
	{
		Log(ERROR, "File doesnt exist or cant be opened", __LINE__, __PRETTY_FUNCTION__, __FILE__);
		_error_code = 404;
	}

    char read_buffer[65535]; // create a read_buffer
    while (file.read(read_buffer, sizeof(read_buffer)))
        _entireBody.append(read_buffer, file.gcount());
    _entireBody.append(read_buffer, file.gcount());

	file.close();
}

bool Response::path_includes_cgi(void)
{
	if (_req.path().find("/cgi") != string::npos) //TODO:, find cgi paths dynamically
		return true;
	return false;
}

bool	Response::is_cgi(void) const
{
	string location_path = get_location_path();
	if (location_path == "")
	{
		try {
			_serverConfig.find_normal_directive("cgi_pass");
			return (1);
		} catch (BaseConfig::ConfigException &e) {
			return (0);
		}
	}
	else
	{
		try {
			ServerLocationDirectiveConfig location_block = _serverConfig.find_location_directive(location_path);

			if (location_block.get_config().find("cgi_pass") == location_block.get_config().end())
				return (0);
			return (1);
		} catch (BaseConfig::ConfigException &e) {
			return (0);
		}
	}
}

bool	Response::is_redirect(void) const
{
	string location_path = get_location_path();
	if (location_path == "")
	{
		try {
			_serverConfig.find_normal_directive("return");
			return (1);
		} catch (BaseConfig::ConfigException &e) {
			return (0);
		}
	}
	else
	{
		try {
			ServerLocationDirectiveConfig location_block = _serverConfig.find_location_directive(location_path);

			if (location_block.get_config().find("return") == location_block.get_config().end())
				return (0);
			return (1);
		} catch (BaseConfig::ConfigException &e) {
			return (0);
		}
	}
}

string	Response::get_redirected_path(void) const
{

	string location_path = get_location_path();
	try {
		ServerLocationDirectiveConfig location_block = _serverConfig.find_location_directive(location_path);

		if (location_block.get_config().find("return") == location_block.get_config().end())
			return ("");
		return (location_block.get_config().find("return")->second);
	} catch (BaseConfig::ConfigException &e) {
		return ("");
	}
}

bool	Response::is_file(const string path) const
{
	struct stat	file;
	stat(path.c_str(), &file);

	return (S_ISREG(file.st_mode));
}

bool	Response::check_file_status(const string path) const
{
	struct stat	dir_stat;
	if (stat(path.c_str(), &dir_stat) == -1) //if cgi file cant be opened
		return (1);
	return (0);
}

string	Response::find_query_string()
{
	//TODO: dynamically find cgi path
	string query;

	if (_req.path().find("?") == string::npos)
		return (string(""));
	query = _req.path().substr(_req.path().find("?"), _req.path().length() - _req.path().find("?"));
	return (query);
}

string	Response::find_path_info()
{
	string path;
	int end;

	if (_req.path().find("?") != string::npos)
		end = _req.path().find("?") - string("/cgi").length(); //TODO: dynamic
	else
		end = _req.path().length();

	path = _req.path().substr(string("/cgi").length(), end); //TODO: dynamic

	if (path.length() == 0)
		path = "/";
	return (path);
}

char	**create_new_envp(string query_string, string path_info, char **envp)
{
	int count;
	char **new_envp;

	count = 0;
	for (char **env = envp; *env != 0; env++)
		count++;
	new_envp = (char **)malloc(sizeof(char *) * (count + 3));
	count = 0;
	for (char **env = envp; *env != 0; env++)
	{
		new_envp[count] = envp[count];
		count++;
	}
	new_envp[count] = strdup((string("QUERY_STRING=") + query_string).c_str());
	new_envp[count + 1] = strdup((string("PATH_INFO=") + path_info).c_str());
	new_envp[count + 2] = NULL;	
	return new_envp;
}

string Response::process_cgi(const string cgi_path)
{
	cout << "ENTERED CGI PATH" << endl;
	string entireText = "";
	int		fd[2];
	char	execve_buffer[100000];
	pipe(fd);

	if (check_file_status(cgi_path) == 1) //if file err
	{
		Log(ERROR, "CGI path " + cgi_path + " cant be opened", __LINE__, __PRETTY_FUNCTION__, __FILE__);
		return (string());
	}
	pid_t i = fork();
	if (i == 0) //child
	{
		std::vector<std::string>  s;
		s.push_back("/opt/homebrew/opt/python@3.9/libexec/bin/python3");
		s.push_back(cgi_path); //dynamic if file exists 404 if not

		std::vector<char*> commands;
		for (size_t i = 0; i < s.size(); i++)
			commands.push_back(const_cast<char*>(s[i].c_str()));
		commands.push_back(nullptr);

		/* dprintf(2, "in child\n"); */

		string query_string, path_info;

		query_string = find_query_string();
		path_info = find_path_info();

		// int file;
		if (_req.type() == "POST") //read the request body if its post
		{
			int tempfd[2];

			pipe(tempfd);
			// file = open((string("body") + std::to_string(_req.socket())).c_str(), O_RDWR);
			write(tempfd[1], _req.body().c_str(), _req.body().length());
			dup2(tempfd[0], STDIN_FILENO);
			close(tempfd[1]);
			close(tempfd[0]);
		}
		
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		close(fd[0]);
		execve("/opt/homebrew/opt/python@3.9/libexec/bin/python3", commands.data(), create_new_envp(query_string, path_info, _envp));
		exit(1);
	}
	else if (i > 0) //parent
	{
		/* dprintf(2, "parent waiting\n"); */
		if (_req.type() == "POST")
			std::remove((string("body") + std::to_string(_req.socket())).c_str());
		close(fd[1]);
		waitpid(i, NULL, 0);
		/* dprintf(2, "parent done waiting\n"); */
		int test = read(fd[0], execve_buffer, 99999);
		close(fd[0]);
		/* dprintf(2, "\t\t--BEFORE---\n"); */
		/* dprintf(2, "%s", entireText.c_str()); */
		/* dprintf(2, "\t\t-----\n"); */
		/* dprintf(2, "read count = [%d]\n", test); */
		/* // dprintf(2, "execve buffer = [%s]\n", execve_buffer); */
		execve_buffer[test] = '\0';
		entireText = string(execve_buffer);
		/* dprintf(2, "\t\t-----\n"); */
		/* dprintf(2, "%s", entireText.c_str()); */
		/* dprintf(2, "\t\t-----\n"); */
	}
	else
	{
		Log(ERROR, "Fork Failed", __LINE__, __PRETTY_FUNCTION__, __FILE__);
		perror("fork failed");
		_error_code = 500;
		_exit(3);
	}
	return (entireText);
}

void Response::respond(void)
{
    if (_req.done() || _req.bad_request())
    {
		//check if path is ridrection dynamically
		if (_req.path() == "/upload.html") //move later (dynamic)
		{
			cout << "IN HERE" << endl;
			_req.process_image();
		}

        ssize_t total_to_send = _entireText.length();
		cout << "total to send :" << total_to_send << endl;

		ssize_t sent = send(_client_fd, _entireText.c_str(), _entireText.length(), 0);
		cout << "sent : " << sent << endl;
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
	/* return("<link rel=\"stylesheet\" href=\"https://drive.google.com/uc?export=view&id=1ZCGfFPqxAPPh66miEdAjFlYmeC8krMjc\">"); */
	return("<link rel=\"stylesheet\" href=\"/style/autoindex.css\">");
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

static string			auto_index_create_file(const string &path, const int &file_type, const string &file_name, const string &url_path)
{
	string	file_str;
	string	full_path = url_path + file_name;
	Log(DEBUG, "full_path : " + full_path);
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

std::string	Response::handle_auto_index(string &path)
{
	string						auto_index_html;
	DIR							*directory;
	struct dirent				*file;     
	string						url_path = _req.path();
	std::multimap<int, string>	sorted_mmap;

	/* Log(WARN, "Path in autoindex : " + path); */
	if (url_path.back() != '/')
		url_path.push_back('/');
	/* Log(WARN, "Path in autoindex after pop : " + path); */
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
	{
		Log(ERROR, "Couldn't open directory", __LINE__, __PRETTY_FUNCTION__, __FILE__);
		_error_code = 404;
		return "";
	}

	auto_index_html += auto_index_create_html(0, path);
	for (std::multimap<int,string>::iterator it = sorted_mmap.begin(); it != sorted_mmap.end(); it++) {
		auto_index_html += auto_index_create_file(path, it->first, it->second, url_path);
	}
	auto_index_html += auto_index_create_html(1);
	return (auto_index_html);
}

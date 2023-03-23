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
	_hasText(true), _error_code(0), _envp(envp), _redirected_path("") //change later when req.bad_req is int
{
	_root_path = _serverConfig.find_normal_directive("root").get_value(); //change again
	if (_req.bad_request())
	{
		Log(INFO, "Bad Request Recieved");
		_error_code = 400;
		build_error_body();
		build_header();
		_entireText = _entireHeader + _entireBody;
		return;
	}

	if (is_redirect())
	{
		_error_code = 301;
		_redirected_path = _serverConfig.find_location_directive(get_location_path()).get_value("return");
		build_header();
		_entireText = _entireHeader + _entireBody;
		return;
	}

	if (_req.type() == "GET")
	{
		string full_path = get_full_path();

		if (is_autoindex() && !is_file(full_path))
			_entireBody = handle_auto_index(full_path);
		else if (is_cgi())
		{
			Log(DEBUG, "PROCESS IS CGI");
			_entireBody = process_cgi(full_path);
		}
		else if (!is_redirect())
			read_file(full_path);
		if (_error_code != 0)
			build_error_body();
		build_header();
		_entireText = _entireHeader + _entireBody;
	}
	else if (_req.type() == "DELETE")
	{
		string full_path = get_full_path();
		if (!has_allowed_method(_req.type()))
		{
			Log(INFO, "Method not allowed");
			_error_code = 405;
		}
		else
		{
			int	status;
			status = remove(full_path.c_str());
			if (status == 0)
				Log(INFO, "File has been deleted 💀");
			else
			{
				_error_code = 204;
				Log(INFO, "File to delete doesn't exists 🤰");
			}
		}
		if (_error_code != 0)
			build_error_body();
		build_header();
		_entireText = _entireHeader + _entireBody;
	}
	else if (_req.type() == "POST")
	{
		string	full_path = get_full_path();
		string	file_name;

		Log(DEBUG, "POST IS CGI");
		if (is_cgi())
			_entireBody = process_cgi(full_path);
		else
			_entireBody = process_image(&file_name);
		if (has_client_max_body_size())
		{
			Log(DEBUG, "Client has max body size of :" + std::to_string(get_client_max_body_size()));
			if (_entireBody.length() > get_client_max_body_size())
			{
				_error_code = 413;
				_entireBody.clear();
			}
		}
		if (_error_code == 0 && !is_cgi()) //if image post and length not mismatch
		{
			Log(DEBUG, "IMAGE SAVED");
			save_image(file_name);
			_entireBody.clear();
			_error_code = 302;
		}
		if (_error_code != 0)
			build_error_body();
		build_header();
		_entireText = _entireHeader + _entireBody;
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

	if (_req.path() == "/" || _req.path() == "")
	{
		full_path = _serverConfig.find_normal_directive("root").get_value();
		Log(DEBUG, "true root in / : " + full_path);
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
			if (true_root.back() == '/')
				true_root.pop_back();
			Log(DEBUG, "true_root : " + true_root);
			Log(DEBUG, "full_path : " + full_path);
			if ((get_location_path() == _req.path() && !is_autoindex()) || is_cgi())
			{
				full_path = get_true_root(location_block_config);
				if (full_path.back() != '/')
					full_path += "/";
				full_path += get_true_index(location_block_config);
				utils::replaceAll(full_path, "%20", " ");
				return (full_path);
			}
			Log(DEBUG, "full_path with index : " + full_path);
			utils::replaceAll(full_path, location_path, true_root);
			utils::replaceAll(full_path, "%20", " ");
			Log(DEBUG, "full_path with index after replace : " + full_path);
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
			string autoindex = _serverConfig.find_normal_directive("autoindex").get_value();
			return (autoindex == "on");
		} catch (BaseConfig::ConfigException &e) {
			return (false);
		}
	}
	else
	{
		try {
			ServerLocationDirectiveConfig location_block = _serverConfig.find_location_directive(location_path);
			ServerLocationDirectiveConfig::cit_t directive = location_block.get_config().find("autoindex");

			if (directive == location_block.get_config().end())
				return (false);
			return (directive->second == "on");
		} catch (BaseConfig::ConfigException &e) {
			return (false);
		}
	}
}

bool	Response::has_client_max_body_size(void) const
{
	string location_path = get_location_path();

	if (location_path == "")
	{
		try {
			_serverConfig.find_normal_directive("client_max_body_size");
			return (true);
		} catch (BaseConfig::ConfigException &e) {
			return (false);
		}
	}
	try {
		ServerLocationDirectiveConfig location_block = _serverConfig.find_location_directive(location_path);

		if (location_block.get_config().find("client_max_body_size") == location_block.get_config().end())
			return (false);
		return (true);
	} catch (BaseConfig::ConfigException &e) {
		return (false);
	}
}

unsigned long int Response::get_client_max_body_size(void) const
{
	string location_path = get_location_path();

	if (location_path == "")
	{
		try {
			_serverConfig.find_normal_directive("client_max_body_size");
			return (std::stoul(_serverConfig.find_normal_directive("client_max_body_size").get_value()));
		} catch (BaseConfig::ConfigException &e) {
			return (0);
		}
	}
	else
	{
		try {
			ServerLocationDirectiveConfig location_block = _serverConfig.find_location_directive(location_path);

			if (location_block.get_config().find("client_max_body_size") == location_block.get_config().end())
				return (0);
			return (std::stoul(location_block.get_config().find("client_max_body_size")->second));
		} catch (BaseConfig::ConfigException &e) {
			return (0);
		}
	}
}

void	Response::build_header(void)
{
	_entireHeader = "HTTP/1.1 ";
	if (_error_code)
	{
		_entireHeader += std::to_string(_error_code) + " ";
		switch (_error_code)
		{
			default: _entireHeader += "Not OK"; break;
			case (301): _entireHeader += "Moved Permanently\r\nLocation: " + _redirected_path + "\r\n\r\n"; return;
			case (302): _entireHeader += "Moved Temporarily\r\nLocation: " + _redirected_path + "\r\n\r\n"; return;
			case (400): _entireHeader += "Bad Request"; break;
			case (403): _entireHeader += "Forbidden"; break;
			case (404): _entireHeader += "Not Found"; break;
			case (413): _entireHeader += "Content Too Large"; break;
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
	Log(DEBUG, "in read_file : [" + path_no_spaces + "]");

	if (stat(path_no_spaces.c_str(), &dir_stat) == 0) //if successfully stat
	{
		if (dir_stat.st_mode & S_IFDIR) //if dir
		{
			Log(ERROR, "File {" + path_no_spaces  + "} is a directory", __LINE__, __PRETTY_FUNCTION__, __FILE__);
			_error_code = 403;
		}
		else //if not dir
		{
			Log(DEBUG, "File " + path_no_spaces + " opened");
			file.open(path_no_spaces, std::ios::binary);
		}
	}
	else
	{
		Log(ERROR, "File {" + path_no_spaces  + "} doesnt exist or cant be opened", __LINE__, __PRETTY_FUNCTION__, __FILE__);
		_error_code = 404;
	}

    char read_buffer[655350]; // create a read_buffer
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
	try {
		ServerLocationDirectiveConfig location_block = _serverConfig.find_location_directive(location_path);

		if (location_block.get_config().find("return") == location_block.get_config().end())
			return (0);
		return (1);
	} catch (BaseConfig::ConfigException &e) {
		return (0);
	}
}

bool	Response::has_allowed_method(const string method) const
{
	string location_path = get_location_path();
	if (location_path == "")
	{
		try {
			_serverConfig.find_normal_directive("allowed_methods"); //TODO add split methods for allowed methods in server block
			return (1);
		} catch (BaseConfig::ConfigException &e) {
			return (0);
		}
	}
	else
	{
		try {
			ServerLocationDirectiveConfig location_block = _serverConfig.find_location_directive(location_path);
			if (location_block.get_config().find("allowed_methods") == location_block.get_config().end())
				return (0);
			else
			{
				std::vector<string> methods = location_block.split_methods();
				for (std::vector<string>::const_iterator it = methods.begin(); it != methods.end(); it++)
					if (method == *it)
						return (1);
				return (0);
			}
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
		end = _req.path().find("?") - get_location_path().length(); //TODO: dynamic
	else
		end = _req.path().length();

	path = _req.path().substr(get_location_path().length(), end); //TODO: dynamic

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
	string entireText = "";
	int		fd[2];
	int		filefd[2];
	char	execve_buffer[100000];
	pipe(fd);

	if (check_file_status(cgi_path) == 1) //if file err
	{
		Log(ERROR, "CGI path " + cgi_path + " cant be opened", __LINE__, __PRETTY_FUNCTION__, __FILE__);
		return (string());
	}

	string test_path = get_location_path();

	if (_req.type() == "POST") //read the request body if its post
		pipe(filefd);

	pid_t i = fork();
	
	if (i == 0) //child
	{
		std::vector<std::string>  s;
		s.push_back("/usr/bin/python3");
		s.push_back(cgi_path); //dynamic if file exists 404 if not


		std::vector<char*> commands;
		for (size_t i = 0; i < s.size(); i++)
			commands.push_back(const_cast<char*>(s[i].c_str()));
		commands.push_back(nullptr);


		string query_string, path_info;

		query_string = find_query_string();
		path_info = find_path_info();

		// int file;
		if (_req.type() == "POST") //read the request body if its post
		{
			// file = open((string("body") + std::to_string(_req.socket())).c_str(), O_RDWR);
			dup2(filefd[0], STDIN_FILENO);
			close(filefd[0]);
			close(filefd[1]);
		}
		
		dup2(fd[1], STDOUT_FILENO);//NOTE: try comment this and it works
		close(fd[1]);
		close(fd[0]);
		execve("/usr/bin/python3", commands.data(), create_new_envp(query_string, path_info, _envp));
		exit(1);
	}
	else if (i > 0) //parent
	{
		string to_write = _req.body();
		if (_req.type() == "POST") //read the request body if its post
		{
			while (to_write.length() > 0)
			{
				int w = write(filefd[1], to_write.c_str(), to_write.length() > 60000 ? 60000 : to_write.length());
				if (w > 0)
					to_write.erase(0, w);
			}
			close(filefd[1]);
			close(filefd[0]);
		}
		close(fd[1]);
		int test = -1;
		while (test != 0)
		{
			test = read(fd[0], execve_buffer, 60000);
			if (test > 0)
			{
				execve_buffer[test] = '\0';
				entireText = entireText + string(execve_buffer);
			}
		}
		waitpid(i, NULL, 0);
		close(fd[0]);
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

void	Response::save_image(const string file_name)
{
	string	path_to_save = get_true_root(_serverConfig.find_location_directive(get_location_path()).get_config());
	string	path_to_redirect = _req.path();

	if (path_to_save.back() != '/')
		path_to_save.push_back('/');
	if (path_to_redirect.back() != '/')
		path_to_redirect.push_back('/');
	path_to_save += file_name;
	path_to_redirect += file_name;
	_redirected_path = path_to_redirect;

	std::ofstream file(path_to_save);
	file << _entireBody;
	file.close();
}

string	Response::process_image(string *file_name)
{
	string	res;

    if (_req.headers().find("Content-Type") == _req.headers().end())
        return (res);

    if (_req.headers().find("Content-Type")->second.find("multipart/form-data;") == 0)
    {
        string boundary;
        string body = _req.body();
        std::vector<string> datas;

        boundary = "--" + _req.headers().find("Content-Type")->second.substr(_req.headers().find("Content-Type")->second.find("=") + 1, 38);
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

            while (headers.find("\r\n") != string::npos)
            {
                string line =  (headers.substr(0, headers.find("\r\n")));
                if (line.find(":") != string::npos)
                {
                    string key = (line).substr(0, (line).find(" ") - 1);
                    string value = (line).substr((line).find(" ") + 1, (line).size() - (line).find(" ") - 1);
					/* cout << "Key : [" << key << "]" << endl; */
					/* cout << "value : [" << value << "]" << endl; */
                    temp_dict[key] = value;
                }
                headers = headers.erase(0, headers.find("\r\n") + 2);
                if (headers.size() >= 2 && headers[0] == '\r' && headers[1] == '\n') {
                    temp_dict["body"] = headers.substr(2, headers.length() - 4);
                    break;
                }
            }

            if (temp_dict.find("Content-Type") != temp_dict.end())
            {
				cout << "doing something" << endl;
				cout << "writing length : " << temp_dict["body"].length() << endl;
				*file_name = find_filename(temp_dict["Content-Disposition"]);
				res.append(temp_dict["body"]);
            }
        }
    }
	return (res);
}

void Response::respond(void)
{
    if (_req.done() || _req.bad_request())
    {
		ssize_t sent = send(_client_fd, _entireText.c_str(), _entireText.length(), 0);
		if (sent == 0)
			cout << "SENT IS 0" << endl;
		if (sent == -1)
		{
			cout << "lmao test" << endl;
			std::cerr << "send err : " << errno << endl;
			return ;
		}
		if (sent != (ssize_t)_entireText.length())
			_entireText = _entireText.substr(sent);
		else
			_hasText = false;
    }
}

bool Response::hasText(void) { return (_hasText); }

const string & Response::get_header(void) const { return (_entireHeader) ;}

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
	if (attr.st_size > 1000000)
		return (std::to_string(attr.st_size/1000000) + "MB");
	else if (attr.st_size > 1000)
		return (std::to_string(attr.st_size/1000) + "KB");
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

	if (url_path.back() != '/')
		url_path.push_back('/');
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

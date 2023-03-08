#include "Response.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <sys/dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define _XOPEN_SOURCE 700 //for autoindex
#define _GNU_SOURCE //for checking audoindex file type

using namespace webserv;

using std::cout;
using std::endl;

Response::Response()
{
    cout << "RESPONSE CONSTRUCTOR CALLED\n";
}

Response::Response(const Request &req, const string &root_path, std::map<int, string>::iterator &it)
{
    _req = req;
    _root_path = root_path;
    _it = it;
    this->respond();
}

void Response::respond(void)
{
    const char *header = "HTTP/1.1 200 OK\nContent-Type: */*\n\n"; // Dynamically add content length TODO
    const char *header2 = "HTTP/1.1 200 OK\nContent-Type: image/*\r\n\r\n";
    const char *header_404 = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n\n";

    std::ifstream myfile;
    string entireText;
    string line;

	if (_req.path() == "/")
	{
		myfile.open(_root_path + "/index.html", std::ios::binary);
	}
	else if (_req.path() == "/autoindex")
	{
		entireText += header;
		entireText += handle_auto_index(_root_path);
	}
	else
		myfile.open(_root_path + _req.path(), std::ios::binary);

    if (!myfile)
    {
        entireText += header_404;
        myfile.open("public/404.html", std::ios::binary);
    }
    else
    {
        if (_req.path() == "/edlim.jpg" || _req.path() == "/edlim_lrg.jpg" || _req.path() == "/jng.png")
            entireText += header2;
        else
            entireText += header;
    }

    char read_buffer[65535]; // create a read_buffer
    while (myfile.read(read_buffer, sizeof(read_buffer)))
        entireText.append(read_buffer, myfile.gcount());
    entireText.append(read_buffer, myfile.gcount());
    myfile.close();
    if (_req.done())
    {
        if (_req.path() == "/upload.html")
            _req.process_image();
        int total_to_send = entireText.length();
        cout << "total to send :" << total_to_send << endl;
        for (ssize_t total_sent = 0; total_sent < total_to_send;)
        {
            size_t len = (entireText.length() > 50000 ? 50000 : entireText.length());
            string temp = entireText.substr(0, len);
            int sent = send(_it->first, temp.c_str(), len, 0);
            if (sent == -1)
            {
                cout << "errno :" << errno << endl;
                break;
            }
            entireText = entireText.substr(len);
            total_sent += sent;
            cout << "length sent :" << sent << endl;
            cout << "total sent :" << total_sent << endl;
        }
    }
}

/*
 * change this later pls..
 * @note : DONT LOOK AT THIS PLS
 * */

static string	auto_index_apply_syle(void)
{
	//return (" html { min-height: 100%; text-align: center; display: flex; justify-content: center; flex-direction: column; } body { font-family: Arial, sans-serif; margin: 0; padding: 0; text-align: center; min-height: 100%; } h1 { text-align: center; } table { border: none; border-bottom: 1px solid black; border-collapse: collapse; margin: 0 auto; width: 600px; } thead { border-bottom: 1px solid black; } th, td { /* border: 1px solid black; */ padding: 10px; text-align: left; } td a { font-weight: 600; }");
	return("<link rel=\"stylesheet\" href=\"autoindex.css\">");
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

	cout << file_name << endl;
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
	cout << auto_index_html << endl;
	return (auto_index_html);
}

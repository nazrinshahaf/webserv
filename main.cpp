#include "BaseConfig.hpp"
#include "BindingSocket.hpp"
#include "ListeningSocket.hpp"
#include "ServerBaseConfig.hpp"
#include "ServerConfig.hpp"
#include "ServerConfigParser.hpp"
#include "ServerNormalDirectiveConfig.hpp"
#include "Socket.hpp"
#include <utility>
#include <vector>
#define PORT 80

#include <ios>
#include <sys/_types/_int8_t.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <sstream>

using std::string;
using std::endl;
using std::cout;
using std::pair;

using namespace webserv;

int test_basic_server()
{
    int server_fd;
	int new_socket;
	long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    const char *hello = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 100\n\n\
		<!DOCTYPE html>\
		<html>\
		<body>\
			<h1>My First Heading</h1>\
			<p>My first paragraph.</p>\
		</body>\
		</html>";
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        
        char buffer[30000] = {0};
        valread = read( new_socket , buffer, 30000);
        printf("%s\n",buffer );
        write(new_socket , hello , strlen(hello));
        printf("------------------Hello message sent-------------------\n");
        close(new_socket);
    }
    return 0;
}

//int	test_multimap()
//{
//	typedef std::multimap<int,string>			map_type;
//	typedef std::multimap<int,string>::iterator	map_iterator;
//
//	std::multimap<int, string>	mm;
//
//	mm.insert(pair<int,string>(1,"80"));
//	mm.insert(pair<int,string>(1,"[::]80"));
//	mm.insert(pair<int,string>(3,"lmao"));
//	mm.insert(pair<int,string>(2,"lmao"));
//
//	pair<map_iterator,map_iterator>	range = mm.equal_range(1);
//
//	for (map_iterator it = range.first; it != range.second; it++)
//		cout << it->second << endl;
//
//	return (0);
//}

void	test_parser()
{
	//cout << "Testing Constructor with ifstream" << endl;
	//{
	//	std::ifstream	conf_file("./test.conf");
	//	webserv::ServerConfigParser	parsed_config(conf_file);
	//}

	//cout << "Testing Constructor with ifstream" << endl;
	//{
	//	string			conf_str;

	//	if (conf_file)
	//	{
	//		std::ostringstream ss;
	//		ss << conf_file.rdbuf();
	//		conf_str = ss.str();
	//	}

	//	webserv::ServerConfigParser	parsed_config(conf_str);
	//}
	
	{
		std::ifstream	conf_file("./test.conf");
		webserv::ServerConfigParser	config_parser(conf_file);

		config_parser.parse_config();
		//pair<ServerConfigParser::const_iterator_type, ServerConfigParser::const_iterator_type> range = config_parser.find_values("server");
		//for (ServerConfigParser::const_iterator_type it = range.first; it != range.second; it++)
		//{
		//	if (it->first == "server")
		//	{
		//		ServerConfig sc = dynamic_cast<ServerConfig&>(*(it->second));
		//		pair<ServerConfig::const_iterator_type, ServerConfig::const_iterator_type> range2 = sc.find_values("listen");

		//		cout << it->first << endl;
		//		for (ServerConfig::const_iterator_type sit = range2.first; sit != range2.second; sit++)
		//		{
		//			cout << "\t" << sit->first << endl;
		//			ServerNormalDirectiveConfig nd = dynamic_cast<ServerNormalDirectiveConfig&>(*(sit->second));
		//			cout << "\t\t" << nd.get_value() << endl;
		//		}
		//	}
		//}
	}
}

void	test_parser_abs()
{
	std::map<string, ServerBaseConfig*> config;

	ServerNormalDirectiveConfig *test = new ServerNormalDirectiveConfig();

	test->set_config("val1", "val2");

	//cout << test->get_value() << "," << test->get_value2() << endl;

	config.insert(std::make_pair("key", test));

}

//struct B {
//  virtual B* get_base() { return new B; }
//  virtual ~B() { }
//};
//
//struct D: public B {
//  D() {d_val = "B.lmao";}
//  D* get_base() { return new D; }
//  string get_content() { return d_val;}
//
//  string d_val;
//};
//
//struct E: public B {
//  E()
//  {
//	  e_val.push_back("E.lmao0");
//	  e_val.push_back("E.lmao1");
//	  e_val.push_back("E.lmao2");
//  }
//  E* get_base() { return new E; }
//  std::vector<string> get_content() { return e_val;}
//
//  std::vector<string> e_val;
//};
//
//int main() {
//	B* obj1 = new D();
//	B* obj2 = new E();
//
//	D* d_obj = dynamic_cast<D*>(obj1);
//	if (d_obj != nullptr)
//	{
//		std::cout << "d_val: " << d_obj->get_content() << std::endl;
//	}
//
//	E* e_obj = dynamic_cast<E*>(obj2);
//	if (e_obj != nullptr)
//	{
//		std::vector<string> e_val = e_obj->get_content();
//		std::cout << "e_val: ";
//		for (std::vector<string>::iterator it = e_val.begin(); it != e_val.end(); it++)
//			std::cout << *it << " ";
//		std::cout << std::endl;
//	}
//}

//using namespace webserv;
//
//int	main()
//{
//	ServerNormalDirectiveConfig *test = new ServerNormalDirectiveConfig();
//
//	test->set_config(std::string("lmao"));
//}

void	test_socket()
{
	//Socket(AF_INET, SOCK_STREAM, 0, 80, INADDR_ANY);
	//BindingSocket(AF_INET, SOCK_STREAM, 0, 80, INADDR_ANY);
	ListeningSocket(AF_INET, SOCK_STREAM, 0, 80, INADDR_ANY, 10);
}

int	main()
{
	//test_basic_server();
	//test_multimap();
	//test_parser();
	//test_parser_abs();
	test_socket();

	return (0);
}

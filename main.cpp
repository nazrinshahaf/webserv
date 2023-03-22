#include "src/config/ServerConfigParser.hpp"
#include "src/server/Server.hpp"

#include <cstdlib>
#include <exception>
#include <ios>
#include <map>
#include <set>
#include <sys/_types/_int8_t.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <poll.h>
#include <fcntl.h>
#include <utility>
#include <vector>

using std::string;
using std::cout;
using std::endl;

using namespace webserv;

void start_server(char **envp)
{
	std::ifstream		conf_file("./webserv.conf");
	ServerConfigParser	config_parser(conf_file);
	
	config_parser.parse_config();
	config_parser.validate_config();

	cout << config_parser << endl;

	Server				server(config_parser, envp);
	server.launch();
}

int main(int argc, char **argv, char **envp)
{
	(void) argc;
	(void) argv;
	
	try
	{
		start_server(envp);
	}
	catch (std::exception &e)
	{
		cout << RED << e.what() << RESET << endl;
		return (1);
	}
#ifdef TEST_LEAKS
	system("leaks webserv");
#endif // DEBUG
	return (0);
}

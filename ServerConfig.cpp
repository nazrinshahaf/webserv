#include "ServerConfig.hpp"
#include "ServerBaseConfig.hpp"
#include "ServerNormalDirectiveConfig.hpp"
#include "ServerLocationDirectiveConfig.hpp"
#include "Socket.hpp"
#include <string>
#include <utility>

using namespace webserv;

using std::cout;
using std::endl;

ServerConfig::ServerConfig()
{
#ifdef PRINT_MSG
	cout << "ServerConfig Default Constructor called" << endl;
#endif
}

ServerConfig::ServerConfig(const ServerConfig &to_copy) :
	_server_config(to_copy._server_config)
{
#ifdef PRINT_MSG
	cout << "ServerConfig Copy Constructor called" << endl;
#endif
}

ServerConfig::~ServerConfig()
{
#ifdef PRINT_MSG
	cout << "ServerConfig Default Destructor called" << endl;
#endif
}

ServerConfig*	ServerConfig::get_base() const
{
	return (new ServerConfig());
}

ServerConfig*	ServerConfig::get_copy() const
{
	return (new ServerConfig(*this));
}

const ServerConfig::map_type&	ServerConfig::get_server_config() const
{
	return (_server_config);
}

pair<ServerConfig::const_iterator_type, ServerConfig::const_iterator_type>
ServerConfig::find_values(const string &key) const
{
	ServerConfig::cit_t it = _server_config.find(key);

	if (it == _server_config.end())
		throw ConfigException("Key {" + key + "} not found in find_values.");
	return (_server_config.equal_range((key)));
}

const ServerNormalDirectiveConfig&
ServerConfig::find_normal_directive(const string &key) const
{
	ServerConfig::cit_t it = _server_config.find(key);

	if (it == _server_config.end())
		throw ConfigException("Key {" + key + "} not found in find_normal_directive.");
	ServerNormalDirectiveConfig *nd = dynamic_cast<ServerNormalDirectiveConfig*>(it->second);
	return (*nd);
}

const ServerLocationDirectiveConfig&
ServerConfig::find_location_directive(const string &path) const
{
	pair<ServerConfig::cit_t, ServerConfig::cit_t>	range = _server_config.equal_range("location");
	if (range.first == _server_config.end())
		throw ConfigException("No location blocks found in find_location_directive");

	ServerLocationDirectiveConfig *location;
	for (ServerConfig::cit_t location_block = range.first; location_block != range.second; location_block++)
	{
		location = dynamic_cast<ServerLocationDirectiveConfig*>(range.first->second);
		if (location->get_path() == path) {
			break;
		}
	}
	return (*location);
}

/*
 * @note : It's done in this over complicated way so that in the upper levels I dont
 *	have to deal with data management, and I can deal with data management internally.
 *	But damn this looks confusing af.
 * */

void	ServerConfig::insert_config(const pair<string, ServerBaseConfig*> &pair_to_insert)
{
	pair<string, ServerBaseConfig*> copy(pair_to_insert.first,
			dynamic_cast<ServerBaseConfig*>(pair_to_insert.second->get_copy()));

	_server_config.insert(copy);
}

std::ostream&	operator<<(std::ostream& os, const ServerConfig &server_config)
{
	//std::multimap<string, ServerBaseConfig*> test = server_config.get_server_config();
	ServerConfig::map_type	map = server_config.get_server_config();

	for (ServerConfig::iterator_type it = map.begin(); it != map.end(); it++)
	{
		os << "ServerConfig key : " BLUE  << (*it).first << RESET << endl;
		if ((*it).first != "location")
		{
			ServerNormalDirectiveConfig	nd = dynamic_cast<ServerNormalDirectiveConfig&>(*(*it).second);
			os << "\t" << nd << endl;
		}
		else
		{
			ServerLocationDirectiveConfig	ld = dynamic_cast<ServerLocationDirectiveConfig&>(*(*it).second);
			ServerLocationDirectiveConfig::map_type	map = ld.get_config();

			for (ServerLocationDirectiveConfig::map_type::iterator it = map.begin(); it != map.end(); it++)
				os << "\t<key : " MAGENTA << (*it).first << RESET ", value : " CYAN << (*it).second << ">" RESET << endl;
			os << endl;
		}
	}
	return (os);
}

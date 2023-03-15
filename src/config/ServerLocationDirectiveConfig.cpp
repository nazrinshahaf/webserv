#include "ServerLocationDirectiveConfig.hpp"
#include <map>
#include <utility>
#include <vector>

using namespace webserv;

using std::cout;
using std::endl;

ServerLocationDirectiveConfig::ServerLocationDirectiveConfig()
{
#ifdef PRINT_MSG
	cout << "ServerLocationDirectiveConfig Default Constructor called" << endl;
#endif
}

ServerLocationDirectiveConfig::ServerLocationDirectiveConfig(const ServerLocationDirectiveConfig &to_copy) :
	_location_directive_config(to_copy._location_directive_config),
	_path(to_copy._path)
{
#ifdef PRINT_MSG
	cout << "ServerLocationDirectiveConfig Copy Constructor called" << endl;
#endif
}

ServerLocationDirectiveConfig::~ServerLocationDirectiveConfig()
{
#ifdef PRINT_MSG
	cout << "ServerLocationDirectiveConfig Destructor called" << endl;
#endif
}

ServerLocationDirectiveConfig*
ServerLocationDirectiveConfig::get_base(void) const
{
	return (new ServerLocationDirectiveConfig());
}

ServerLocationDirectiveConfig*
ServerLocationDirectiveConfig::get_copy(void) const
{
	return (new ServerLocationDirectiveConfig(*this));
}

void	ServerLocationDirectiveConfig::set_path(const string &path_to_set)
{
	_path = path_to_set;
}


const string &ServerLocationDirectiveConfig::get_path(void) const
{
	return (_path);
}

const ServerLocationDirectiveConfig::map_type &
ServerLocationDirectiveConfig::get_config() const
{
	return (_location_directive_config);
}

pair<ServerLocationDirectiveConfig::const_iterator_type, ServerLocationDirectiveConfig::const_iterator_type>
ServerLocationDirectiveConfig::find_values(const string &key) const
{
	cit_t pair = _location_directive_config.find(key);

	if (pair == _location_directive_config.end())
		throw BaseConfig::ConfigException("Key {" + key + "} not found in get_value.");
	return (_location_directive_config.equal_range(key));
}

const string &
ServerLocationDirectiveConfig::get_value(const string &key) const
{
	cit_t	pair = _location_directive_config.find(key);

	if (pair == _location_directive_config.end())
		throw BaseConfig::ConfigException("Key {" + key + "} not found in get_value.");
	return (pair->second);
}

std::vector<string>	ServerLocationDirectiveConfig::split_methods() const
{
	string					methods;
	std::vector<string>		res;
	size_t					start = 0;
	size_t					end = -1;

	methods = _location_directive_config.find("allowed_methods")->second;

	while (start != end)
	{
		start = methods.find_first_not_of("\t ", start);
		end = methods.find_first_of("\t ", start);
		if (start == methods.npos)
			break;
		res.push_back(methods.substr(start, end - start));
		start = end;
		end = -1;
	}
	return (res);
}

void	ServerLocationDirectiveConfig::insert_config(const std::pair<string,string> &pair_to_insert)
{
	_location_directive_config.insert(pair_to_insert);
}

std::ostream&	operator<<(std::ostream& os,
		const ServerLocationDirectiveConfig	&location_directive)
{
	ServerLocationDirectiveConfig::map_type	map = location_directive.get_config();

	for (ServerLocationDirectiveConfig::map_type::iterator it = map.begin(); it != map.end(); it++)
		os << "<key : " MAGENTA << (*it).first << RESET ", value : " CYAN << (*it).second << ">" RESET << endl;
	return (os);
}

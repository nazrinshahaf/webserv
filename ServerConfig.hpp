#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include "BaseConfig.hpp"
#include "ServerBaseConfig.hpp"
#include "ServerNormalDirectiveConfig.hpp"
#include "ServerLocationDirectiveConfig.hpp"

#include <map>
#include <string>
#include <iostream>
#include <utility>
#include <exception>

using std::string;
using std::pair;

namespace webserv
{
	/*
	 * ServerConfig.
	 * ServerConfig is the class that holds all contents withing a server
	 * block.
	 * 
	 * @var : contains a multimap<string, ServerConfig*>
	 * */

	class	ServerConfig : public BaseConfig
	{
		public:
			typedef std::multimap<string, ServerBaseConfig*>					map_type;
			typedef std::multimap<string, ServerBaseConfig*>::iterator			iterator_type;
			typedef std::multimap<string, ServerBaseConfig*>::const_iterator	const_iterator_type;
			typedef std::multimap<string, ServerBaseConfig*>::iterator			it_t;
			typedef std::multimap<string, ServerBaseConfig*>::const_iterator	cit_t;

			ServerConfig();
			ServerConfig(const ServerConfig &to_copy);
			~ServerConfig();

			ServerConfig &operator=(const ServerConfig &to_copy);

			/*
			 * get_base.
			 * Returns a pointer the ServerConfig type.
			 *
			 * @return : malloced pointer to ServerConfig.
			 * */

			ServerConfig	*get_base() const;

			/*
			 * get_copy.
			 * Returns a copy of the ServerConfig.
			 * */

			ServerConfig	*get_copy() const;

			/*
			 * get_server_config.
			 * Returns a reference to the underlying map.
			 * */

			const map_type &get_server_config() const;

			/*
			 * find_values.
			 * Searches for all occurences of key in map.
			 * 
			 * @return : A pair of const_iterators to the range of all
			 *	values that have 'key'.
			 *
			 * @param 'key' : the key to look for in the ServerConfig.
			 * */

			pair<const_iterator_type, const_iterator_type>	find_values(const string &key) const;

			/*
			 * find_normal_directives.
			 * Searches for a normal_directive.
			 *
			 * @return : The normal_directive.
			 *
			 * @param 'key' : the key of the normal_directive to look
			 *	for.
			 * */

			const ServerNormalDirectiveConfig	&find_normal_directive(const string &key) const;

			/*
			 * find_location_directive.
			 * Searches for a location.
			 *
			 * @return : The location_directive.
			 *
			 * @param 'key' : the key of the location_directive to look
			 *	for.
			 * */

			const ServerLocationDirectiveConfig	&find_location_directive(const string &path) const;

			/*
			 * insert_config.
			 * Inserts config into _server_config.
			 *
			 * @param 'pair_to_insert' : the pair to insert into the _server_config.
			 * */

			void	insert_config(const pair<string, ServerBaseConfig*> &pair_to_insert);

		private:
			std::multimap<string, ServerBaseConfig*>	_server_config;
	};
}

std::ostream& operator<<(std::ostream& os,
		const webserv::ServerConfig &server_config);

#endif

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include "BaseConfig.hpp"
#include "ServerBaseConfig.hpp"

#include <map>
#include <string>
#include <iostream>
#include <utility>

using std::string;
using std::pair;

/*
 * 
 * server {
 *   listen			80;
 *   listen			[::]:80;
 *   server_name	domain1.com www.domain1.com;
 *   access_log 	logs/domain1.access.log main;
 *   root       	html;
 *
 *   location \ {
 *    fastcgi_pass  127.0.0.1:1025;
 *    root			/var/www/virtual/big.server.com/htdocs;
 *    expires		30d;
 *   }
 * }
 *
 * multimap<string, BaseConfig>			_config;
 * multimap<string, ServerBaseConfig*>	_server_config;
 *
 * ServerNormalDirectiveConfig -> ServerBaseConfig;
 *	string	_server_normal_directive_config;
 *
 * ServerLocationDirectiveConfig -> ServerBaseConfig;
 *	multimap<string, string> _server_location_directive_config;
 *	multimap<string, pair<string, string> > _server_location_directive_config; (not sure yet how i want to store path)
 *
 * <user, www-data>
 *
 * <server, <
 *	 <listen, <80> >
 *	 <listen, <[::]:80> >,
 *	 <server_name, <domain1.com www.domain1.com> >,
 *	 <access_long, <logs/domain1.access.log main> >,
 *	 <root, <html> >,
 *	
 *	 <location /path/to/dir/here, ,
 *		<fastcgi_pass, 127.0.0.1:1025>,
 *		<root, /var/www/virtual/big.server.com/htdocs>,
 *		<expires, 30d>,
 *	 >
 * >
 *
 * multimap<server, ServerBaseConfig multimap<
 *	ServerNormalDirectiveConfig <listen, 80>,
 *	ServerNormalDirectiveConfig <listen, [::]:80>,
 *	ServerNormalDirectiveConfig <server_name, <domain1.com www.domain1.com> >
 *	ServerNormalDirectiveConfig <access_log, <logs/domain1.access_log.log main> >
 *	ServerNormalDirectiveConfig <root, html>
 *
 *	ServerLocationDirectiveConfig multimap <location /path/to/dir/here,
 *		<fastcgi_pass, 127.0.0.1:1025>,
 *		<root, /var/www/virtual/big/server/com/htdocs>,
 *		<expires, 30d>,
 *	>
 * >
 * */


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

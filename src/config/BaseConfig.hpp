#ifndef BASECONFIG_HPP
# define BASECONFIG_HPP

#include "ServerBaseConfig.hpp"
#include <exception>
#include <string>
#include <map>

using std::string;

namespace webserv
{
	/*
	 * BaseConfig.
	 * The BaseConfig is the parent class for the first level
	 * of hierarchy in the conf file.
	 * */

	class BaseConfig
	{
		public:
			virtual ~BaseConfig() {};

			virtual BaseConfig	*get_base() const = 0;
			virtual BaseConfig	*get_copy() const = 0;

			class   ConfigException : public std::exception
			{
				public:
					ConfigException(string err_msg) : _err_msg(err_msg) {};
					~ConfigException() _NOEXCEPT {};

					virtual const char  *what(void) const throw() {
						return (_err_msg.c_str());
					};

				private:
					string  _err_msg;
			};

	};
}

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

#endif

#ifndef SERVERLOCATIONDIRECTIVECONFIG_HPP
# define SERVERLOCATIONDIRECTIVECONFIG_HPP

#include "ServerBaseConfig.hpp"
#include "colours.h"

#include <string>
#include <iostream>
#include <map>

using std::string;
using std::pair;

namespace webserv
{
	class	ServerLocationDirectiveConfig : public ServerBaseConfig
	{
		public:

			typedef std::multimap<string, string>					map_type;
			typedef std::multimap<string, string>::iterator			iterator_type;
			typedef std::multimap<string, string>::const_iterator	const_iterator_type;
			typedef std::multimap<string, string>::iterator			itt;
			typedef std::multimap<string, string>::const_iterator	citt;

			ServerLocationDirectiveConfig();
			ServerLocationDirectiveConfig(const ServerLocationDirectiveConfig &to_copy);
			~ServerLocationDirectiveConfig();

			ServerLocationDirectiveConfig	*get_base(void) const;
			ServerLocationDirectiveConfig	*get_copy(void) const;

			const map_type&									get_config() const;
			pair<const_iterator_type,const_iterator_type>	find_values(const string &key) const;

			//void	insert_config(const string &key, const string &value);
		
			void	set_path(const string &path_to_set);

			void	insert_config(const std::pair<string, string> &pair_to_insert);

		private:
			std::multimap<string,string>	_location_directive_config;
			string							_path;
	};
}

std::ostream& operator<<(std::ostream& os,
		webserv::ServerLocationDirectiveConfig const &location_directive);

#endif //!SERVERLOCATIONDIRECTIVECONFIG_HPP

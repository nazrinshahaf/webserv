#ifndef SERVERLOCATIONDIRECTIVECONFIG_HPP
# define SERVERLOCATIONDIRECTIVECONFIG_HPP

#include "ServerBaseConfig.hpp"
#include "BaseConfig.hpp"
#include "../colours.h"

#include <string>
#include <iostream>
#include <map>
#include <vector>

using std::string;
using std::pair;
using std::multimap;

namespace webserv
{
	class	ServerLocationDirectiveConfig : public ServerBaseConfig
	{
		public:

			typedef multimap<string, string>	map_type;
			typedef pair<string, string>		pair_type;
			typedef map_type::iterator			iterator_type;
			typedef map_type::const_iterator	const_iterator_type;
			typedef map_type::iterator			it_t;
			typedef map_type::const_iterator	cit_t;

			ServerLocationDirectiveConfig();
			ServerLocationDirectiveConfig(const ServerLocationDirectiveConfig &to_copy);
			~ServerLocationDirectiveConfig();

			ServerLocationDirectiveConfig	*get_base(void) const;
			ServerLocationDirectiveConfig	*get_copy(void) const;

			const map_type									&get_config() const;
			pair<const_iterator_type,const_iterator_type>	find_values(const string &key) const;
			const string									&get_value(const string &key) const;
			std::vector<string>								split_methods() const;
		
			void			set_path(const string &path_to_set);
			const string	&get_path(void) const;

			void			insert_config(const std::pair<string, string> &pair_to_insert);

		private:
			map_type	_location_directive_config;
			string		_path;
	};
}

std::ostream& operator<<(std::ostream& os,
		webserv::ServerLocationDirectiveConfig const &location_directive);

#endif //!SERVERLOCATIONDIRECTIVECONFIG_HPP

#ifndef SERVERNORMALDIRECTIVECONFIG_HPP
# define SERVERNORMALDIRECTIVECONFIG_HPP

#include "ServerBaseConfig.hpp"
#include "../colours.h"

#include <string>
#include <iostream>
#include <utility>

using std::string;
using std::pair;

namespace webserv
{
	class	ServerNormalDirectiveConfig : public ServerBaseConfig
	{
		public:
			ServerNormalDirectiveConfig();
			ServerNormalDirectiveConfig(const ServerNormalDirectiveConfig &to_copy);
			~ServerNormalDirectiveConfig();

			ServerNormalDirectiveConfig &operator=(const ServerNormalDirectiveConfig &to_copy);

			ServerNormalDirectiveConfig	*get_base(void) const;
			ServerNormalDirectiveConfig	*get_copy(void) const;

			const pair<string, string>	&get_config(void) const;

			const string				&get_value(void) const;
			const string				&get_value2(void) const;

			void	set_config(const string &value1, const string &value2 = string());
			void	set_config(const pair<string, string> &pair_to_insert);
			

		private:
			pair<string, string>	_normal_directive_config;
	};
}

/*
 * @note : All ostream overloads have to be outside of namespace 
 * idk why but if its not i get compile error.
 * */

std::ostream& operator<<(std::ostream& os,
		const webserv::ServerNormalDirectiveConfig &normal_directive);

#endif // !SERVERNORMALDIRECTIVECONFIG_HPP

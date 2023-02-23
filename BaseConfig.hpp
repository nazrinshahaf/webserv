#ifndef BASECONFIG_HPP
# define BASECONFIG_HPP

#include "ServerBaseConfig.hpp"
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
	};
}

#endif

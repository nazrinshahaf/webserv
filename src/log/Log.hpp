#ifndef LOG_HPP
# define LOG_HPP

#include "ServerConfig.hpp"
#include <string>
#include <iomanip>
#include <fstream>

using std::string;

namespace webserv
{
	enum	log_level
	{
		DEBUG,
		INFO,
		WARN,
		ERROR
	};
	class	Log
	{
		public:
			static const log_level	_base_log_level = DEBUG;

			Log(const log_level &level, const string &log_msg, const int &log_to_file = 0,
					ServerConfig const &server = ServerConfig());
		private:
			void	print_debug_level(const log_level &level, const int &log_to_file, std::fstream &log_file,
						const log_level &file_log_level) const;
			void	print_debug_msg(const log_level &level, const int &log_to_file, std::fstream &log_file,
						const log_level &file_log_level,const string &log_msg) const;
	};
}

#endif // !LOG_HPP

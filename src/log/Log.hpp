#ifndef LOG_HPP
# define LOG_HPP

#include "../config/ServerConfig.hpp"
#include <cstddef>
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
		ERROR,
		NONE
	};
	class	Log
	{
		public:
			static const log_level	_base_log_level = INFO;

			Log(const log_level &level, const string &log_msg,
					const int &LINE = __LINE__, const char *FUNC = __FILE__, const char *FILE = __FILE__,
					const int &log_to_file = 0, ServerConfig const &server = ServerConfig());
		private:
			void	print_debug_level(const log_level &level, const int &log_to_file, std::fstream &log_file,
						const log_level &file_log_level) const;
			void	print_error_msg(const int &log_to_file, std::fstream &log_file,
						const log_level &file_log_level, const string &log_msg) const;
			void	print_debug_msg(const log_level &level, const int &log_to_file, std::fstream &log_file,
						const log_level &file_log_level,const string &log_msg) const;

			int		_err_line;
			const char	*_err_func;
			const char	*_err_file;
	};
}

#endif // !LOG_HPP

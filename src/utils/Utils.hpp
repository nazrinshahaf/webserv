#ifndef UTILS_HPP
# define UTILS_HPP

#include <string>

namespace webserv
{
	namespace utils 
	{
		void replaceAll(std::string& str, const std::string& from, const std::string& to);
		bool isNumber(const string &s);
	}
}

#endif // ! UTILS_HPP

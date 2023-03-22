#include <string>

namespace webserv
{
    namespace utils
    {
        void replaceAll(std::string &str, const std::string &from, const std::string &to)
        {
            if (from.empty())
                return;
            size_t start_pos = 0;
            while ((start_pos = str.find(from, start_pos)) != std::string::npos)
            {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
            }
        }
        
        bool isNumber(const std::string &s)
        {
            return s.find_first_not_of("0123456789") == std::string::npos;
        }
    }
}

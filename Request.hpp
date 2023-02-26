#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <iostream>
#include <string>

using std::string;

namespace webserv 
{
	class	Request
	{
		public:

			/**
			 * Request Default Constructor.
			 *
			 * @param 'request' : specifies the raw request string which was received
             *
			 * */

			Request(string request_string);
			~Request();

			void	launch(void);

		private:

	};
}

#endif // !SERVER_HPP
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

			const string type();
			const string host();
			const string connection();

			class RequiredHeaderParamNotGivenException : public std::exception {
    			public:
					string what () {
    					return "required header has not been supplied";
					}
			};

		private:
			string _type;
			string _host;
			string _connection;
	};
}

#endif // !SERVER_HPP
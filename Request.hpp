#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>

using std::string;

namespace webserv 
{
	class	Request
	{

		private:
    		std::vector <string>		_header_lines;
    		std::map <string, string>	_headers;
			string						_body;
			string						_type;
			string						_path;
			string						_protocol_version;
			int							_socket;
			bool						_is_done;
			void						find_request_type();
			void						find_request_path();
			void						find_request_protocol_version();
			void						parse_headers();
			void						process_post();


		public:
			/**
			 * Request Default Constructor.
			 *
			 * @param 'request' : specifies the raw request string which was received
             *
			 * */
			Request() {};
			Request(string request_string, int socket);


			~Request();

			const string    				&body() const;
			const string 					&type() const;
			const string 					&path() const;
			const string 					&protocol_version() const;
			const std::map<string, string>	&headers() const;
			const int						&socket() const;
			const string					to_str() const;
			bool							done();
			void							add_body(string buffer);

			struct RequiredHeaderParamNotGivenException : public std::exception {
				const char * what () const throw () {
					return "required header has not been supplied";
				}
			};
	};
}

std::ostream &operator<<(std::ostream &os, const webserv::Request &request);

#endif // !SERVER_HPP

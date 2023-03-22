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
    		std::map <string, string>	_headers;
    		std::vector <string>		_header_lines; //move to private later
			string						_body;
			string						_type;
			string						_path;
			string						_protocol_version;
			int							_socket;
			bool						_is_done;
			bool						_header_done;
			bool						_bad_request; //change to int
			bool						_chunked;
			double			_chunk_length;


			int							find_request_type();
			void						find_request_path();
			void						find_request_protocol_version();
			void						parse_headers();
			void						process_post();
			string						process_chunk(string buffer);

		public:
			/**
			 * Request Default Constructor.
			 *
			 * @param 'request' : specifies the raw request string which was received
             *
			 * */
			Request();
			Request(const Request &to_copy);
			Request(string request_string, int socket);
			~Request();

			string    						&body();
			const string 					&type() const;
			const string 					&path() const;
			const string 					&protocol_version() const;
			const std::map<string, string>	&headers() const;
			const int						&socket() const;
			const string					to_str() const;
			bool							bad_request();
			bool							is_chunked() const;
			bool							done();
			bool							header_done();
			void							add_body(string buffer);
			void							read_header(string request_string);
			struct RequiredHeaderParamNotGivenException : public std::exception {
				const char * what () const throw () {
					return "required header has not been supplied";
				}
			};
	};
}

std::ostream &operator<<(std::ostream &os, const webserv::Request &request);

#endif // !SERVER_HPP

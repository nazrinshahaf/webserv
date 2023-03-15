#ifndef SERVERCONFIGPARSER_HPP
# define SERVERCONFIGPARSER_HPP

#include "BaseConfig.hpp"
#include "ServerConfig.hpp"
#include "ServerNormalDirectiveConfig.hpp"
#include "ServerLocationDirectiveConfig.hpp"

#include <_ctype.h>
#include <algorithm>
#include <cstddef>
#include <sys/_types/_size_t.h>
#include <map>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <sstream>
#include <vector>

using std::string;

namespace webserv
{
	/*
	 * ServerConfigParser.
	 *
	 * The class that is used to parse the config that is later needed
	 * in during server creation.
	 * */

	class ServerConfigParser
	{
		public:

			typedef std::multimap<string, BaseConfig*>					map_type;
			typedef std::multimap<string, BaseConfig*>::iterator		iterator_type;
			typedef std::multimap<string, BaseConfig*>::const_iterator	const_iterator_type;
			typedef std::multimap<string, BaseConfig*>::iterator		it_t;
			typedef std::multimap<string, BaseConfig*>::const_iterator	cit_t;

			/*
			 * ServerConfigParser string Assignment Constructor.
			 *
			 * @param 'config_str' : the string to be parsed into 
			 *	relevant data structure.
			 * */

			ServerConfigParser(const string &config_str);

			/*
			 * ServerConfigParser ifstream Assignment Constructor.
			 *
			 * @param 'config_file' : the file to be parsed into 
			 *	relevant data structure.
			 * */

			ServerConfigParser(std::ifstream &config_file);

			/*
			 * ServerConfigParser Copy constructor.
			 *
			 * @param 'to_copy' : the other ServerConfigParser
			 *	to be copied.
			 * */

			ServerConfigParser(const ServerConfigParser &to_copy);

			/*
			 * ServerConfigParser Copy Assignment Operator.
			 *
			 * @param 'to_copy' : the other ServerConfigParser
			 *	to be copied.
			 * */

			ServerConfigParser	&operator=(const ServerConfigParser &to_copy);

			/*
			 * ServerConfigParser Destructor.
			 * */

			~ServerConfigParser();

			/*
			 * parse_config.
			 * Parses the _config_str to be relevant data format to 
			 * later be passed into server instantiation.
			 * */

			void		parse_config(void);

			/*
			 * validate_config.
			 * Validates the _config to ensure that all data passed into
			 * server will be in the appropriate format.
			 * */

			void		validate_config(void);

			const map_type	&get_config() const;

			/*
			 * find_values.
			 * Searches for all occurences of key in map.
			 * 
			 * @return : A pair of const_iterators to the range of all
			 *	values that have 'key'.
			 *
			 * @param 'key' : the key to look for in the ParserConfig.
			 * */

			pair<const_iterator_type, const_iterator_type>
				find_values(const string &key) const;

			/*
			 * insert_config.
			 * Inserts a config into the parser config.
			 *
			 * @param : a pair of <string, BaseConfig*> to be inserted
			 *	into the parser config
			 * */

			void	insert_config(const pair<string, BaseConfig*> &pair_to_insert);

		private:

			/*
			 * extract_bracket_content.
			 * Extracts all content within a bracket.
			 *
			 * @note: this function is made with the assumption that all opening
			 *	brackets within the block are all of the same type.
			 * @note : I might need to change how this works.
			 *
			 * @param 'config' : the entirety of the string to extract
			 *	config[0] must move past initial opening bracket.
			 * @param 'open_bracket' : the opening bracket specifier passed in.
			 * @param 'close_bracket' : the closinng bracket specifier passed in.
			 * */

			string	extract_bracket_content(std::string *config,
						const char &open_bracket, const char &close_bracket) const;

			/*
			 * extract_key.
			 * Extracts the first word before a bracket.
			 * Can also extracts the first word before a bracket.
			 *
			 * @param 'line' : the entirety of the string to extract 
			 *	the first word of.
			 */

			string	extract_key(const string &line) const;

			/*
			 * extract_value.
			 * Extracts the value in a line.
			 *
			 * @note : value has to be on the same line as key.
			 *
			 * @param 'line' : the line to extract the first value of.
			 */

			string	extract_value(const string &line, int look_for_bracket = 0) const;

			int		count_values_in_line(const string &line) const;

			/*
			 * parse_server_block.
			 * Parse the contents within a server_block.
			 *
			 * @param 'server_block' : the contents of the server block
			 *	to be parsed. Has to be passed into extract_bracket_content
			 *	prior to this.
			 * */

			const ServerConfig					parse_server_block(const string &server_block);

			/*
			 * parse_server_location_block.
			 * Parses the contents within a server_location_block.
			 *
			 * @param 'location_block' : the contents of the location block
			 *	to be parsed. Has to be passed into extract_bracket_content
			 *	prior to this.
			 * */

			ServerLocationDirectiveConfig	parse_server_location_block(const string &location_block);

			/*
			 * parse_server_normal_directive.
			 * Parses the contents within a server_normal_directive_line.
			 *
			 * @param 'normal_directive_line' : the contents of the normal directive
			 *	line to be parsed. Line has to be extracted to the nearest ";" prior
			 *	to this.
			 * */

			const ServerNormalDirectiveConfig		parse_server_normal_directive(const string &normal_directive_line);

			/*
			 * is_valid_server_normal_directive.
			 * Returns weather or not the key is a valid server_normal_directive.
			 *
			 * @return :
			 *	0 if key is not a valid directive.
			 *	1 if key is a valid directive.
			 * 	2 if key is a valid directive with multiple values.
			 * 	3 if key is a valid directive with block contents.
			 *
			 * @param 'key' : key to check for valid directive.
			 * */

			int			is_valid_server_normal_directive(const string &key) const;
			int			is_valid_location_directive(const string &key) const;
			int			is_valid_base_directive(const string &directive) const;
		
			/*
			 * initialize_valid_directives.
			 * Used to initialize valid directives because fucking cpp98 doesn't
			 * have a better way of initializing a set of strings. Dynamic way
			 * to initialize all types of valid directives.
			 *
			 * @param 'directives' : the const array of c_strings that are the 
			 *	valid directives to be added into 'set'. I set this at the top
			 *	of ServerConfigParser.cpp.
			 * @param 'set' : a pointer to the private_variable set of valid 
			 *	directives to add the c_strings into.
			 * @param 'len' : the len of the array of c_strings (idk why i cant do it
			 *  inside the funcition instead).
			 * */

			void		initialize_valid_directives(const char **directives, std::set<string> *set, size_t len);

			void		validate_server_map_directive_count(const ServerConfig::map_type &server_map) const;
			void		validate_location_map_directive_count(const ServerLocationDirectiveConfig::map_type &location_map) const;

			void		validate_listen(const string &value) const;
			void		validate_error_log(const string &value) const;
			void		validate_error_page(const string &value) const;
			void		validate_allowed_methods(const std::vector<string> &allowed_methods) const;
			void		validate_auto_index(const string &value) const;

			class   ServerParserException : public std::exception
			{
				public:
					ServerParserException(string err_msg) : _err_msg(err_msg) {};
					~ServerParserException() _NOEXCEPT {};

					virtual const char  *what(void) const throw() {
						return (_err_msg.c_str());
					};

				private:
					string  _err_msg;
			};

			std::set<string>	_valid_base_directives;
			std::set<string>	_valid_server_normal_directives;
			std::set<string>	_valid_server_location_directives;
			string				_config_str;
			map_type			_config;
	};
}

std::ostream& operator<<(std::ostream& os,
		const webserv::ServerConfigParser &server_config);

#endif

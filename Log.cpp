#include "Log.hpp"

using std::cout;
using std::endl;

using namespace webserv;

/*
 * log_to_file :
 *	0 = just print to stdout
 *	1 = print to file passed in by server
 *	2 = print to both
 *
 *	@note : idk what to if file if error in config do i exit out early or what.
 * */

void	print_time(std::ostream &stream)
{
	std::time_t t = std::time(0);
	std::tm*	now = std::localtime(&t);
	char		prev_fill = std::cout.fill();

	stream << " " << (now->tm_year + 1900) << '-'
		<< std::setfill('0')<< std::setw(2) << (now->tm_mon + 1) << '-'
		<< std::setfill('0')<< std::setw(2) << now->tm_mday << " "
		<< std::setfill('0')<< std::setw(2) << now->tm_hour << ":"
		<< std::setfill('0')<< std::setw(2) << now->tm_min << ":"
		<< std::setfill('0')<< std::setw(2) << now->tm_sec<< " - ";
	std::cout.fill(prev_fill);

}

void	Log::print_debug_level(const log_level &level, const int &log_to_file, std::fstream &log_file, const log_level &file_log_level) const
{
	switch(level)
	{
		case 0:
			if (log_to_file != 1 && level >= _base_log_level)
				cout << GREEN "[DEBUG]\t" RESET;
			if (log_to_file >= 1 && level >= file_log_level)
				log_file << "[DEBUG]\t";
			break;
		case 1:
			if (log_to_file != 1 && level >= _base_log_level)
				cout << BLUE "[INFO]\t" RESET;
			if (log_to_file >= 1 && level >= file_log_level)
				log_file << "[INFO]\t";
			break;
		case 2:
			if (log_to_file != 1 && level >= _base_log_level)
				cout << YELLOW "[WARN]\t" RESET;
			if (log_to_file >= 1 && level >= file_log_level)
				log_file << "[WARN]\t";
			break;
		case 3:
			if (log_to_file != 1 && level >= _base_log_level)
				cout << RED "[ERROR]\t" RESET;
			if (log_to_file >= 1 && level >= file_log_level)
				log_file << "[ERROR]\t";
			break;
		break;
	}
}

void	Log::print_debug_msg(const log_level &level, const int &log_to_file, std::fstream &log_file,
			const log_level &file_log_level, const string &log_msg) const
{
	string	log_msg_copy = log_msg;

	while (1)
	{
		string log_line;
		if (log_msg_copy.length() == 0)
			break ;
		if (log_msg_copy.find_first_of("\n") == log_msg_copy.npos)
			log_line = log_msg_copy;
		else
			log_line = log_msg_copy.substr(0, log_msg_copy.find_first_of("\n"));
		print_debug_level(level, log_to_file, log_file, file_log_level);
		switch (log_to_file)
		{
			case 2 :
				if (level < _base_log_level)
					break;
				print_time(cout);
				cout << log_line << endl;
			case 1 :
				if (file_log_level < _base_log_level)
					break;
				print_time(log_file);
				log_file << log_line << endl;
				break;
			case 0 :
				if (level < _base_log_level)
					break;
				print_time(cout);
				cout << log_line << endl;
		}
		if (log_msg_copy.find_first_of("\n") == log_msg_copy.npos)
			break ;
		log_msg_copy = log_msg_copy.substr(log_msg_copy.find_first_of("\n") + 1);
	}
}

Log::Log(const log_level &level, const string &log_msg, const int &log_to_file, ServerConfig const &server)
{
	log_level		file_log_level;
	std::fstream	log_file;

	if (log_to_file >= 1) //opening file if log_to_file is set to 1 or 2
	{
		pair<ServerConfig::cit_t, ServerConfig::cit_t> pair = server.find_values("error_log");
		ServerNormalDirectiveConfig nd = dynamic_cast<ServerNormalDirectiveConfig&>(*(pair.first->second));

		log_file.open(nd.get_value(), std::ios::app); //append mode
		if (!log_file)
		{
			Log(ERROR, "Cant write to " + nd.get_value()); //file permisions can prob just continue on
			return;
		}
		string	temp_log_level = nd.get_value2();
		std::transform(temp_log_level.begin(), temp_log_level.end(), temp_log_level.begin(), ::toupper); //convert to uppercase

		if (temp_log_level == "DEBUG") //no switch case for string 
			file_log_level = DEBUG;
		else if (temp_log_level == "INFO")
			file_log_level = INFO;
		else if (temp_log_level == "WARN")
			file_log_level = WARN;
		else if (temp_log_level == "ERROR")
			file_log_level = ERROR;
	}

	print_debug_msg(level, log_to_file, log_file, file_log_level, log_msg);
	log_file.close();
}

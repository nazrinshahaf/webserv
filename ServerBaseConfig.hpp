#ifndef SERVERBASECONFIG_HPP
# define SERVERBASECONFIG_HPP

namespace webserv
{
	class	ServerBaseConfig
	{
		public:
			virtual ~ServerBaseConfig() {};

			virtual ServerBaseConfig	*get_base()	const = 0;
			virtual ServerBaseConfig	*get_copy()	const = 0;
	};
}

#endif // !SERVERBASECONFIG_HPP


#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class Server
{

	private:
		int         _port;												// port
		std::string _password;											// password

	public:
		Server(int port, const std::string& password);					// constructor
		Server(const Server &copy);										// copy constructor
		Server &operator=(const Server &assign);						// copy assignment operator
		~Server();														// destructor

		void    start();												// start server
};

#endif

#include "Server.hpp"
#include <iostream>

// ====================================================================
// Orthodox Canonical Form elements:
// ====================================================================

// constructor
Server::Server(int port, const std::string& password)
	: _port(port), _password(password)
{
	std::cout << "Server constructor called." << std::endl;
}

// copy constructor
Server::Server(const Server &copy)
    : _port(copy._port), _password(copy._password)
{
    std::cout << "Server copy constructor called." << std::endl;
}

// copy assignment operator
Server &Server::operator=(const Server &assign)
{
	std::cout << "Server copy assignment operator called." << std::endl;
	if (this != &assign)
	{
		_port = assign._port;
		_password = assign._password;
	}
	return *this;
}

// destructor
Server::~Server()
{
	std::cout << "Server destructor called." << std::endl;
}

// ====================================================================
// Functions:
// ====================================================================

// start server
void Server::start()
{
	std::cout << "Server starting..." << std::endl;
}

#include "Server.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}
	int port = atoi(argv[1]);
	std::string password = argv[2];
	if (port <= 0 || port > 65535)
	{
		std::cerr << "Invalid port number" << std::endl;
		return 1;
	}
	Server server(port, password);
	server.start();
	return 0;
}

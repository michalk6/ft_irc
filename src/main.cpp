#include "Server.hpp"
#include <iostream>			// for std::cerr

int main(int argc, char** argv) {

	try {
		std::string password;
		int port = Server::parseServerArguments(argc, argv, password);
		Server server(port, password);
		server.start();
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
	
}

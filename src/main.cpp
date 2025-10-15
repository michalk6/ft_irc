#include "Server.hpp"
#include <iostream>
#include <string>
#include <cstdlib>			// for std::exit, std::atoi
#include <stdexcept> 		// std::runtime_error, std::exception

// handler for failed allocations
void noMemoryHandler() {
	std::cerr << "Out of memory, cannot allocate more resources.\n";
	std::exit(1);
}

int main(int argc, char *argv[])
{
	// check command line arguments
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}

	// parse command line arguments
	int port = atoi(argv[1]);
	std::string password = argv[2];

	// check port number
	if (port <= 0 || port > 65535) {
		std::cerr << "Invalid port number" << std::endl;
		return 1;
	}

	// set handler for failed allocations
	std::set_new_handler(noMemoryHandler);

	// start server
	try {
		Server server(port, password);
		server.start();
	}
	catch (const std::runtime_error &e) {
		std::cerr << "Runtime error: " << e.what() << std::endl;
		return 1;
	}
	catch (const std::exception &e) {
		std::cerr << "Unexpected exception: " << e.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "Unknown error occurred while starting the server." << std::endl;
		return 1;
	}
	
	return 0;
}

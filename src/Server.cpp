#include "Server.hpp"
#include <iostream>			// for std::cout, std::cerr
#include <cstdlib>			// for std::strtol
#include <stdexcept>		// for std::runtime_error, std::invalid_argument
#include <cstring>			// for std::memset, std::strerror, strncmp
#include <cerrno>			// for errno, EINTR
#include <unistd.h>			// for close, STDIN_FILENO
#include <fcntl.h>			// for fcntl, O_NONBLOCK, F_SETFL
#include <netinet/in.h>		// for sockaddr_in, INADDR_ANY, htons
#include <sys/socket.h>		// for socket, setsockopt, bind, listen, accept, recv, send
#include <arpa/inet.h>		// for getsockname

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// 															PRIVATE:
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||


// ====================================================================
// private methods:
// ====================================================================

// create the listening socket
void Server::createSocket()
{
	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenFd == -1)
		throw std::runtime_error("socket() failed");
}

// set socket to non-blocking mode
void Server::setNonBlocking(int fd)
{
	// load existing file descriptor flags
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("fcntl(F_GETFL) failed");

	// set non-blocking mode flag
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("fcntl(F_SETFL) failed");
}

// set socket options
void Server::setSocketOptions()
{
	int opt = 1;
	if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw std::runtime_error("setsockopt() failed");
}

// bind the listening socket
void Server::bindSocket()
{
	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);

	if (bind(_listenFd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		int bind_errno = errno;
		close(_listenFd);
		throw std::runtime_error(std::string("bind() failed: ") + strerror(bind_errno));
	}

	// check actual port used by socket
	socklen_t addrlen = sizeof(addr);
	if (getsockname(_listenFd, (struct sockaddr*)&addr, &addrlen) == -1)
		throw std::runtime_error("getsockname() failed");

	int actual_port = ntohs(addr.sin_port);
	std::cout << "Using specified port: " << actual_port << std::endl;
	_port = actual_port;
}

// start listening for connections
void Server::startListening()
{
	if (listen(_listenFd, 10) == -1)
		throw std::runtime_error("listen() failed");

	struct pollfd listen_pfd;
	listen_pfd.fd = _listenFd;
	listen_pfd.events = POLLIN;
	_pfds.push_back(listen_pfd);

	struct pollfd stdin_pfd;
	stdin_pfd.fd = STDIN_FILENO;
	stdin_pfd.events = POLLIN;
	_pfds.push_back(stdin_pfd);

	std::cout << "Socket setup complete on port " << _port << std::endl;
}


// configure the listening socket
void Server::setupSocket()
{
	createSocket();
	setNonBlocking(_listenFd);
	setSocketOptions();
	bindSocket();
	startListening();
}

// handle new connection
void Server::handleNewConnection()
{
	struct sockaddr_in clientAddr;
	socklen_t addrlen = sizeof(clientAddr);
	int clientFd = accept(_listenFd, (struct sockaddr*)&clientAddr, &addrlen);
	if (clientFd == -1)
	{
		std::cerr << "accept() failed" << std::endl;
		return;
	}

	// non-blocking
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(clientFd);
		std::cerr << "fcntl() failed on client" << std::endl;
		return;
	}

	// add client to poll
	struct pollfd pfd;
	pfd.fd = clientFd;
	pfd.events = POLLIN;
	_pfds.push_back(pfd);

	std::cout << "New client connected (fd=" << clientFd << ")" << std::endl;
}

// handle existing connection
void Server::handleClientEvent(int i)
{
	char buf[512];
	int clientFd = _pfds[i].fd;
	int bytes = recv(clientFd, buf, sizeof(buf) - 1, 0);

	if (bytes <= 0)
	{
		if (bytes == 0)
			std::cout << "Client disconnected (fd=" << clientFd << ")" << std::endl;
		else
			std::cerr << "recv() error on fd " << clientFd << std::endl;

		close(clientFd);
		_pfds.erase(_pfds.begin() + i);
		return;
	}

	buf[bytes] = '\0';
	std::cout << "Received from fd=" << clientFd << ": " << buf << std::endl;

	// echo back (placeholder)
	send(clientFd, buf, bytes, 0);
}

// handle input from stdin (quit command)
void Server::handleStdinInput()
{
	char buf[16];
	int bytes_read = read(STDIN_FILENO, buf, sizeof(buf) - 1);

	if (bytes_read > 0)
	{
		buf[bytes_read] = '\0';
		if (strncmp(buf, "quit", 4) == 0)
		{
			std::cout << "Server shutting down..." << std::endl;
			_running = false;
		}
	}
}

// Main event loop. As long as the server is running, this loop controls network traffic.
void Server::eventLoop()
{
	std::cout << "Server listening. Type 'quit' to stop." << std::endl;

	while (_running)
	{
		// poll with timeout 1000 ms
		int ret = poll(&_pfds[0], _pfds.size(), 1000);
		if (ret == -1)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("poll() failed");
		}

		// handle events (if any) in _pfds
		for (int i = static_cast<int>(_pfds.size()) - 1; i >= 0; --i)
		{
			if (_pfds[i].revents & POLLIN)
			{
				if (_pfds[i].fd == _listenFd)
					handleNewConnection();
				else if (_pfds[i].fd == STDIN_FILENO)
					handleStdinInput();
				else
					handleClientEvent(i);
			}
		}
	}
}


// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// 															PUBLIC:
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||



// ====================================================================
// Orthodox Canonical Form elements:
// ====================================================================

// constructor 
//		(_pdfs()		- vector is default initialized to empty)
//		_listenFd = -1	- socket not created yet
Server::Server(int port, const std::string& password)
	: _port(port), _password(password), _listenFd(-1), _pfds(), _running(true) {}

// destructor
//		_pfds[0] = _listenFd (we don't need to close it separately)
Server::~Server()
{
	for (size_t i = 0; i < _pfds.size(); ++i)
	{
		if (_pfds[i].fd != -1)
			close(_pfds[i].fd);
	}
}

// ====================================================================
// public methods:
// ====================================================================

// start server
void Server::start()
{
	std::cout << "Server starting..." << std::endl;

	setupSocket();
	eventLoop();
}

// stop server
void Server::stop()
{
	std::cout << "Stopping server..." << std::endl;
	_running = false;
	
	// close all client connections
	for (size_t i = 0; i < _pfds.size(); ++i)
	{
		if (_pfds[i].fd != -1 && _pfds[i].fd != _listenFd && _pfds[i].fd != STDIN_FILENO)
		{
			close(_pfds[i].fd);
			_pfds[i].fd = -1;
		}
	}
	
	// close listening socket
	if (_listenFd != -1)
	{
		close(_listenFd);
		_listenFd = -1;
	}
}

// check if port is valid
bool Server::is_valid_port_string(const char* str) {
	if (!str || *str == '\0') return false;
	if (str[0] == '0' && std::strlen(str) > 1) return false;

	for (size_t i = 0; str[i]; ++i) {
		if (!std::isdigit(str[i])) return false;
	}
	return true;
}

// set port to the port given by user
void Server::setPortNumber(int p)
{
	if (p < 1 || p > 65535)
		throw std::invalid_argument("Port must be between 1 and 65535");
	
	_port = p;
}

// set port to the port given by user
int Server::parseServerArguments(int argc, char** argv, std::string& password) {
	if (argc != 3)
		throw std::invalid_argument(std::string("Usage: ") + argv[0] + " <port> <password>");

	std::string portStr = argv[1];

	// check if port number contains only digits
	for (std::string::size_type i = 0; i < portStr.size(); ++i) {
		if (!isdigit(portStr[i])) {
			throw std::invalid_argument("Port must contain only digits (no + or -)");
		}
	}

	// check if port number contains leading zeros
	if (portStr.size() > 1 && portStr[0] == '0') {
		throw std::invalid_argument("Port cannot have leading zeros");
	}

	// convert port number to long integer
	long port = std::strtol(portStr.c_str(), NULL, 10);
	if (port < 1 || port > 65535) {
		throw std::invalid_argument("Port must be a number between 1 and 65535");
	}

	password = argv[2];
	return static_cast<int>(port);
}

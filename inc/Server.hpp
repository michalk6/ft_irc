#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>		// for std::string
#include <vector>		// for std::vector
#include <poll.h>		// for pollfd, POLLIN
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
#include <cstdio>
#include <poll.h>			// for poll, pollfd
#include <sstream>
#include <vector>

class Server {

	private:
		int        				 _port;						// port
		std::string 			_password;					// password
		int						_listenFd;					// listening socket (to detect that someone is trying to connect)
		std::vector<pollfd>		_pfds;						// poll file descriptors (list of all sockets we want to monitor using poll())
		bool					_running;					// flag to check if server is running

		void createSocket();								// create the listening socket
		void setNonBlocking(int fd);						// set socket to non-blocking mode
		void setSocketOptions();							// set socket options
		void bindSocket();									// bind the listening socket
		void startListening();								// start listening for connections
		void setupSocket();									// configure the listening socket
		void handleNewConnection();							// handle new connection
		void handleClientEvent(int i);						// handle existing connection
		void handleStdinInput();							// handle input from stdin		
		void eventLoop();									// handle events (main loop)
		void handleModeCommand(int clientFd, const std::string &message);
		void handleKickCommand(int clientFd, const std::string &message);
		void handleInviteCommand(int clientFd, const std::string &message);
		void handleTopicCommand(int clientFd, const std::string &message);
		/* 	
			Socket is a system resource that cannot be safely copied.
			By copying _listenFd and _pfds, both objects will point to the same descriptors. 
			Closing one object will invalidate the other. 
			Adding it for keeping orthodox canonical form.
		*/
		Server(const Server &copy);							// copy constructor (we don't use it)
		Server &operator=(const Server &assign);			// copy assignment operator (we don't use it)

	public:
		Server(int port, const std::string& password);		// constructor
		~Server();											// destructor

		void    		start();															// start server
		void			stop();																// stop server
		bool			is_valid_port_string(const char* str);								// check if port is valid
		void			setPortNumber(int p);												// set port to the port given by user
		static int		parseServerArguments(int argc, char** argv, std::string& password);	// port validation and password parsing

};

#endif

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include <vector>			// for std::vector
#include <string>			// for std::string
#include <unistd.h>			// for close, STDIN_FILENO
#include <cstdlib>			// for std::strtol
#include <poll.h>			// for poll, pollfd, POLLIN
#include <sys/socket.h> 	// for socket, setsockopt, bind, listen, accept, recv, send
#include <sstream>			// for std::stringstream

class Server {

	private:
		int        				 _port;						// port
		std::string				_realname;					// realname
		std::string 			_password;					// password
		int						_listenFd;					// listening socket (to detect that someone is trying to connect)
		std::vector<pollfd>		_pfds;						// poll file descriptors (list of all sockets we want to monitor using poll())
		bool					_running;					// flag to check if server is running
		std::vector<Client*>	_clients;					// list of connected clients

		void createSocket();								// create the listening socket
		void setNonBlocking(int fd);						// set socket to non-blocking mode
		void setSocketOptions();							// set socket options
		void bindSocket();									// bind the listening socket
		void startListening();								// start listening for connections
		void setupSocket();									// configure the listening socket
		void handleNewConnection();							// handle new connection
		void handleClientEvent(int i);						// handle existing connection
		void handleStdinInput();							// handle input from stdin
		void handleJoinCommand(int clientFd, const std::string &message);										// handle join command
		void handleChannelMessage(int clientFd, const std::string &channelName, const std::string &msgContent); // handle channel message
		void eventLoop();									// handle events (main loop)

		void handleModeCommand(int clientFd, const std::string &message);			// handle mode command
		void handleKickCommand(int clientFd, const std::string &message);			// handle kick command
		void handleInviteCommand(int clientFd, const std::string &message);			// handle invite command
		void handleTopicCommand(int clientFd, const std::string &message);			// handle topic command
		void handleMsgCommand(int clientFd, const std::string &message);			//	handle msg command
		void handelePrivateMessage(int clientFd, const std::string &target,			// handle private message
			const std::string &msgContent);
		void addClient(Client *client, int clientFd);
		void handleNickCommand(int clientFd, const std::string &message);			// handle nick command
		void handleUserCommand(int clientFd, const std::string &message);			// handle user command
		void handlePassCommand(int clientFd, const std::string &message);			// handle password command
		void completeRegistration(Client *client);
		Client* findClientByFd(int clientFd);									// find client by fd
		std::vector<std::string> ft_split(const std::string &str, char delimiter);	// split string
		
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

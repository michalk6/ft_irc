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
#include <map>
#include <deque>
#include "ChannelMenager.hpp"

class Server {

	private:
		int							_port;						// port
		std::string					_realname;					// realname
		std::string 				_password;					// password
		int							_listenFd;					// listening socket (to detect that someone is trying to connect)
		std::vector<pollfd>			_pfds;						// poll file descriptors (list of all sockets we want to monitor using poll())
		bool						_running;					// flag to check if server is running
		std::vector<Client*>		_clients;					// list of connected clients
		std::vector<std::string>	_channels;					// list of channels
		ChannelManager				_channelManager;
		std::vector<int>			_clientsToRemove;			// list of clients that need to be removed

		// client event handling:    -----------------------------------------------------------------------------------------------------
		void 	handleClientEvent(int i);													// handle existing connection - main function
		void	handleClientDisconnect(int index, int clientFd, int bytes);
		void	cleanupDisconnectedClients() ;
		void	removeClientFromVector(int clientFd);
		void	processClientMessage(int clientFd, char* buf, int bytes);
		void	processSingleCommand(Client* client, int clientFd, const std::string& command);
		bool	handleCapabilityCommands(int clientFd, const std::vector<std::string>& tokens, const std::string& cmd);
		bool	handleAuthenticationCommands(int clientFd, const std::vector<std::string>& tokens, const std::string& cmd);
		void	handleRegisteredCommands(int clientFd, const std::vector<std::string>& tokens, const std::string& cmd, 
					const std::string& fullCommand);
		void	handlePingCommand(int clientFd, const std::vector<std::string>& tokens);
		void	sendNotRegisteredError(int clientFd);
		void	sendUnknownCommandError(int clientFd, const std::string& command);
		void	handleQuitCommand(int clientFd, const std::string &message);
		std::string trimCommand(const std::string& command);
		std::string joinTokens(const std::vector<std::string>& tokens);
		// --------------------------------------------------------------------------------------------------------------------------------


		void 	createSocket();									// create the listening socket
		void 	setNonBlocking(int fd);							// set socket to non-blocking mode
		void 	setSocketOptions();								// set socket options
		void 	bindSocket();									// bind the listening socket
		void 	startListening();								// start listening for connections
		void 	setupSocket();									// configure the listening socket
		void 	handleNewConnection();							// handle new connection
		void 	handleStdinInput();								// handle input from stdin
		void 	eventLoop();									// handle events (main loop)
		
		void	handleModeCommand(int clientFd, const std::string &message);			// handle mode command
		void	handleKickCommand(int clientFd, const std::string &message);			// handle kick command
		void	handleInviteCommand(int clientFd, const std::string &message);			// handle invite command
		void	handleTopicCommand(int clientFd, const std::string &message);			// handle topic command
		void	handleMsgCommand(int clientFd, const std::string &message);				// handle msg command
		void	addClient(Client *client, int clientFd);
		void	handleNickCommand(int clientFd, const std::string &message);			// handle nick command
		void	handleUserCommand(int clientFd, const std::string &message);			// handle user command
		void	handlePassCommand(int clientFd, const std::string &message);			// handle password command
		void	completeRegistration(Client *client);
		void	joindefaultChannel(int clientFd);
		void	handlePartCommand(int clientFd, const std::string &message);
		void	handleWhoCommand(int clientFd, const std::string &message);
		
		Client*	findClientByFd(int clientFd);										// find client by fd
		Client*	findClientByNickname(std::string const &nickname) const;
		
		// handle join command:    --------------------------------------------------------------------------------------------------------
		void 	handleJoinCommand(int clientFd, const std::string &message);						// handle join command - main function
		bool	isValidChannelName(const std::string &channelName);
		bool	validateJoinConditions(Client *client, Channel *channel, const std::vector<std::string> &tokens);
		void	joinClientToChannel(Client *client, Channel *channel, const std::string &channelName);
		void	sendTopicInfo(Client *client, Channel *channel, const std::string &channelName);
		void	sendNamesList(Client *client, Channel *channel, const std::string &channelName);
		void	sendError(int clientFd, const std::string &code, const std::string &message);
		Channel* getOrCreateChannel(const std::string &channelName);
		// --------------------------------------------------------------------------------------------------------------------------------

		void 	handleChannelMessage(int clientFd, const std::string &channelName, const std::string &msgContent);	// handle channel message
		void	handleChannelMode(int clientFd, const std::string &target, const std::vector<std::string> &tokens);
		void	handlePrivateMessage(int clientFd, const std::string &target, const std::string &msgContent);

		bool	setChannelMode(char mode, Client *client, Channel *channel, std::deque<std::string> &parameters);
		bool	unsetChannelMode(char mode, Client *client, Channel *channel, std::deque<std::string> &parameters);
		
		std::vector<std::string> ft_split(const std::string &str, char delimiter);	// split string

		// orthodox canonical form:
		/* 	
			Socket is a system resource that cannot be safely copied.
			By copying _listenFd and _pfds, both objects will point to the same descriptors. 
			Closing one object will invalidate the other. 
			Adding it for keeping orthodox canonical form.
		*/
		Server();											// default constructor
		Server(const Server &copy);							// copy constructor (we don't use it)
		Server &operator=(const Server &assign);			// copy assignment operator (we don't use it)

	public:
		// orthodox canonical form:
		Server(int port, const std::string& password);		// constructor
		~Server();											// destructor

		bool			is_valid_port_string(const char* str);								// check if port is valid
		void			setPortNumber(int p);												// set port to the port given by user
		static int		parseServerArguments(int argc, char** argv, std::string& password);	// port validation and password parsing

		// start server
		void    		start();
		
		// finish program
		void			stop();								// stop server
		void			shutdownGracefully();				// finish program and clean resources in case of out of memory

};

#endif

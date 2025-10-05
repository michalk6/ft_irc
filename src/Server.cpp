#include "Server.hpp"
#include "Channel.hpp"
#include <iostream>		// for std::cout, std::cerr
#include <stdexcept>	// for std::runtime_error, std::invalid_argument
#include <cstring>		// for std::memset, std::strerror, strncmp
#include <cerrno>		// for errno, EINTR
#include <fcntl.h>		// for fcntl, O_NONBLOCK, F_SETFL
#include <netinet/in.h> // for sockaddr_in, INADDR_ANY, htons
#include <arpa/inet.h>	// for getsockname

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
	_listenFd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET - IPv4, SOCK_STREAM - TCP, 0 - default
	std::cout << "Socket FD: " << _listenFd << std::endl;
	if (_listenFd == -1)
		throw std::runtime_error("socket() failed");
}

// set socket to non-blocking mode
void Server::setNonBlocking(int fd)
{
	// load existing file descriptor flags
	int flags = fcntl(fd, F_GETFL, 0); // F_GETFL - get file descriptor flags
	if (flags == -1)
		throw std::runtime_error("fcntl(F_GETFL) failed");

	// set non-blocking mode flag
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) // F_SETFL - set file descriptor flags
		throw std::runtime_error("fcntl(F_SETFL) failed");
}

// set socket options
void Server::setSocketOptions()
{
	int opt = 1;
	// SOL_SOCKET - socket level, SO_REUSEADDR - allow reuse of local addresses
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

	if (bind(_listenFd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		int bind_errno = errno;
		close(_listenFd);
		throw std::runtime_error(std::string("bind() failed: ") + strerror(bind_errno));
	}

	// check actual port used by socket
	socklen_t addrlen = sizeof(addr);
	if (getsockname(_listenFd, (struct sockaddr *)&addr, &addrlen) == -1)
		throw std::runtime_error("getsockname() failed");

	int actual_port = ntohs(addr.sin_port); // convert port number from network byte order to host byte order
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

void Server::setupSocket()
{
	createSocket();
	setNonBlocking(_listenFd);
	setSocketOptions();
	bindSocket();
	startListening();
}

void Server::handleNewConnection()
{
	struct sockaddr_in clientAddr;
	socklen_t addrlen = sizeof(clientAddr);
	int clientFd = accept(_listenFd, (struct sockaddr *)&clientAddr, &addrlen);
	if (clientFd == -1)
	{
		std::cerr << "accept() failed" << std::endl;
		return;
	}
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(clientFd);
		std::cerr << "fcntl() failed on client" << std::endl;
		return;
	}
	Client *newClient = new Client(clientFd, inet_ntoa(clientAddr.sin_addr));
	_clients.push_back(newClient);
	struct pollfd pfd;
	pfd.fd = clientFd;
	pfd.events = POLLIN;
	_pfds.push_back(pfd);

	std::cout << "New client connected (fd=" << clientFd << ")" << std::endl;
	addClient(new Client(clientFd, inet_ntoa(clientAddr.sin_addr)), clientFd);
}

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
void Server::handleWhoCommand(int clientFd, const std::string &message)
{
	std::vector<std::string> tokens = ft_split(message, ' ');
	if (tokens.size() < 2) {
		std::string response = ":server 461 WHO :Not enough parameters\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}
	std::string channelName = tokens[1];
	Channel* channel = _channelManager.getChannel(channelName);
	if (!channel) {
		std::string response = ":server 403 " + channelName + " :No such channel\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}
	for (std::map<int, Client*>::const_iterator it = channel->getMembers().begin(); it != channel->getMembers().end(); ++it) {
		Client* member = it->second;
		std::string reply = ":server 352 " + channelName + " " +
			member->getUsername() + " " +
			member->getHostname() + " server " +
			member->getNickname() + " H :0 " +
			member->getRealname() + "\r\n";
		send(clientFd, reply.c_str(), reply.length(), 0);
	}
	// std::string endReply = ":server 315 " + channelName + " :End of WHO list\r\n";
	// send(clientFd, endReply.c_str(), endReply.length(), 0);
}

void Server::handleJoinCommand(int clientFd, const std::string &message)
{
	Client *client = findClientByFd(clientFd);
	if (!client || !client->isRegistered())
	{
		std::string response = "451 :You have not registered\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	std::vector<std::string> tokens = ft_split(message, ' ');
	if (tokens.size() < 2)
	{
		std::string response = "461 JOIN :Not enough parameters\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	std::string channelName = tokens[1];
	if (channelName[0] != '#' && channelName[0] != '&')
	{
		std::string response = "479 " + client->getNickname() + " " + channelName + " :Invalid channel name\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}
	Channel *channel;
	if (!_channelManager.channelExists(channelName))
	{
		channel = _channelManager.createChannel(channelName);
		std::cout << "Created new channel: " << channelName << std::endl;
	}
	else
	{
		channel = _channelManager.getChannel(channelName);
		std::cout << "Found existing channel: " << channelName << std::endl;
	}

	if (channel->hasMember(clientFd))
	{
		std::cout << "Client already in channel" << std::endl;
		return;
	}
	channel->addMember(client);
	std::string joinMsg = client->getPrefix() + " JOIN " + channelName + "\r\n";
	channel->broadcast(joinMsg);
	if (!channel->getTopic().empty())
	{
		std::string topicMsg = "332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic();
		client->sendMessage(topicMsg);
	}
	else
	{
		std::string noTopicMsg = "331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n";
		send(clientFd, noTopicMsg.c_str(), noTopicMsg.length(), 0);
	}
	std::string names;
	for (std::map<int, Client *>::const_iterator it = channel->getMembers().begin(); it != channel->getMembers().end(); ++it)
	{
		if (!names.empty())
			names += " ";
		if (channel->isOperator(it->first))
		{
			names += "@";
		}
		names += it->second->getNickname();
	}
	std::string namesReply = "353 " + client->getNickname() + " = " + channelName + " :" + names + "\r\n";
	std::string endNames = "366 " + client->getNickname() + " " + channelName + " :End of /NAMES list\r\n";
	send(clientFd, namesReply.c_str(), namesReply.length(), 0);
	send(clientFd, endNames.c_str(), endNames.length(), 0);
	std::cout << "Client " << client->getNickname() << " joined channel " << channelName << std::endl;
	std::cout << "Channel " << channelName << " now has " << channel->getMemberCount() << " members" << std::endl;
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

// handle kick command
void Server::handleKickCommand(int clientFd, const std::string &message)
{
	(void)message;
	// TODO: Implement kick command
	std::string response = ":server 461 KICK :Not implemented yet\r\n";
	send(clientFd, response.c_str(), response.length(), 0);
}

// handle invite command
void Server::handleInviteCommand(int clientFd, const std::string &message)
{
	(void)message;
	// TODO: Implement invite command
	std::string response = ":server 461 INVITE :Not implemented yet\r\n";
	send(clientFd, response.c_str(), response.length(), 0);
}

// handle topic command
void Server::handleTopicCommand(int clientFd, const std::string &message)
{
	(void)message;
	// TODO: Implement topic command
	std::string response = ":server 461 TOPIC :Not implemented yet\r\n";
	send(clientFd, response.c_str(), response.length(), 0);
}

// add client
void Server::addClient(Client *client, int clientFd)
{
	(void)clientFd;
	if (client)
	{
		_clients.push_back(client);
		std::cout << "Client added to list (total: " << _clients.size() << ")" << std::endl;
	}
}

// handle nick command
void Server::handleNickCommand(int clientFd, const std::string &message)
{
	Client *client = findClientByFd(clientFd);
	if (!client)
		return;

	std::vector<std::string> tokens = ft_split(message, ' ');
	if (tokens.size() < 2)
	{
		std::string response = ":server 431 :No nickname given\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	std::string newNick = tokens[1];

	// check if nickname is already in use
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i]->getFd() != clientFd && _clients[i]->getNickname() == newNick)
		{
			std::string response = ":server 433 " + newNick + " :Nickname is already in use\r\n";
			send(clientFd, response.c_str(), response.length(), 0);
			return;
		}
	}

	client->setNickname(newNick);
	std::cout << "Client " << clientFd << " set nickname to: " << newNick << std::endl;

	// check if registration should be completed
	if (!client->isRegistered())
	{
		completeRegistration(client);
	}
}

// handle user command
void Server::handleUserCommand(int clientFd, const std::string &message)
{
	Client *client = findClientByFd(clientFd);
	if (!client)
		return;

	std::vector<std::string> tokens = ft_split(message, ' ');
	if (tokens.size() < 5)
	{
		std::string response = ":server 461 USER :Not enough parameters\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	client->setUsername(tokens[1]);
	client->setRealname(message.substr(message.find(tokens[4])));
	std::cout << "Client " << clientFd << " set username to: " << tokens[1] << std::endl;

	// check if registration should be completed
	if (!client->isRegistered())
	{
		completeRegistration(client);
	}
}

// handle password command
void Server::handlePassCommand(int clientFd, const std::string &message)
{
	Client *client = findClientByFd(clientFd);
	if (!client)
		return;

	std::vector<std::string> tokens = ft_split(message, ' ');
	if (tokens.size() < 2)
	{
		std::string response = ":server 461 PASS :Not enough parameters\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	std::string providedPassword = tokens[1];
	if (providedPassword == _password)
	{
		client->setPasswordVerified(true);
		std::cout << "Client " << clientFd << " provided correct password" << std::endl;
	}
	else
	{
		std::string response = ":server 464 :Password incorrect\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		close(clientFd);
		// Usuń klienta z listy
		for (size_t i = 0; i < _clients.size(); ++i)
		{
			if (_clients[i]->getFd() == clientFd)
			{
				delete _clients[i];
				_clients.erase(_clients.begin() + i);
				break;
			}
		}
	}
}

void Server::completeRegistration(Client *client)
{
	if (client &&
		client->isPasswordVerified() &&
		!client->getNickname().empty() &&
		!client->getUsername().empty())
	{
		client->setRegistered(true);

		// Send welcome message (without leading colon)
		std::string welcome = "001 " + client->getNickname() + " :Welcome to the Internet Relay Network " + 
					 client->getNickname() + "!" + client->getUsername() + "@" + client->getHostname() + "\r\n";
		client->sendMessage(welcome);

		// Auto-join default channel after successful registration
		//joindefaultChannel(client->getFd());

		std::cout << "Client " << client->getFd() << " (" << client->getNickname()
				  << ") successfully registered" << std::endl;
	}
}

// find client by fd
Client *Server::findClientByFd(int clientFd)
{
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i]->getFd() == clientFd)
		{
			return _clients[i];
		}
	}
	return NULL;
}

// split string
std::vector<std::string> Server::ft_split(const std::string &str, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);

	while (std::getline(tokenStream, token, delimiter))
	{
		if (!token.empty())
			tokens.push_back(token);
	}
	return tokens;
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
Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _listenFd(-1), _pfds(), _running(true) {}

// destructor
//		_pfds[0] = _listenFd (we don't need to close it separately)
Server::~Server()
{
	// close all sockets (file descriptors)
	for (size_t i = 0; i < _pfds.size(); ++i)
	{
		if (_pfds[i].fd != -1)
			close(_pfds[i].fd);
	}

	// delete all clients
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		delete _clients[i];
	}
	_clients.clear();

	_pfds.clear();
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

// check if port is valid
bool Server::is_valid_port_string(const char *str)
{
	if (!str || *str == '\0')
		return false;
	if (str[0] == '0' && std::strlen(str) > 1)
		return false;

	for (size_t i = 0; str[i]; ++i)
	{
		if (!std::isdigit(str[i]))
			return false;
	}
	return true;
}

// set port to the port given by user
int Server::parseServerArguments(int argc, char **argv, std::string &password)
{
	if (argc != 3)
		throw std::invalid_argument(std::string("Usage: ") + argv[0] + " <port> <password>");

	std::string portStr = argv[1];

	// check if port number contains only digits
	for (std::string::size_type i = 0; i < portStr.size(); ++i)
	{
		if (!isdigit(portStr[i]))
		{
			throw std::invalid_argument("Port must contain only digits (no + or -)");
		}
	}

	// check if port number contains leading zeros
	if (portStr.size() > 1 && portStr[0] == '0')
	{
		throw std::invalid_argument("Port cannot have leading zeros");
	}

	// convert port number to long integer
	long port = std::strtol(portStr.c_str(), NULL, 10);
	if (port < 1 || port > 65535)
	{
		throw std::invalid_argument("Port must be a number between 1 and 65535");
	}

	password = argv[2];
	return static_cast<int>(port);
}

// stop server
void Server::stop()
{
	_running = false;

	// close all clients
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i])
		{
			close(_clients[i]->getFd());
			delete _clients[i];
		}
	}
	_clients.clear();

	// close listening socket
	if (_listenFd != -1)
	{
		close(_listenFd);
		_listenFd = -1;
	}

	// clear pollfd vector (file descriptors)
	_pfds.clear();
}

// set port to the port given by user
void Server::setPortNumber(int p)
{
	if (p < 1 || p > 65535)
		throw std::invalid_argument("Port must be between 1 and 65535");
	_port = p;
}

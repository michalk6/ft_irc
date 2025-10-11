#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <ostream>
#include "Channel.hpp"
#include "ChannelMenager.hpp"

void Server::handleChannelMessage(int clientFd, const std::string &channelName, const std::string &msgContent)
{
	std::map<std::string, Channel *> channels = _channelManager.getChannels(); // ta sama mapa co w handleJoinCommand

	Client *sender = findClientByFd(clientFd);
	if (!sender)
		return;

	// check if channel exists
	if (channels.find(channelName) == channels.end())
	{
		std::string response = ":server 403 " + sender->getNickname() + " " + channelName + " :No such channel\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	Channel *channel = channels[channelName];

	// check if client is in channel
	std::map<int, Client *> members = channel->getMembers();
	if (members.find(clientFd) == members.end())
	{
		std::string response = ":server 442 " + sender->getNickname() + " " + channelName + " :You're not on that channel\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	// create full message (PRIVMSG)
	std::string fullMessage = sender->getPrefix() + " PRIVMSG " + channelName + " :" + msgContent + "\r\n";

	// send to all clients in channel except sender
	channel->broadcast(fullMessage, sender);
}

void Server::handlePrivateMessage(int clientFd, const std::string &target, const std::string &msgContent)
{
	// Find the target client by nickname
	Client *targetClient = NULL;
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i]->getNickname() == target)
		{
			targetClient = _clients[i];
			break;
		}
	}
	if (!targetClient)
	{
		std::string response = ":server 401 " + target + " :No such nick/channel\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}
	std::string senderPrefix;
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i]->getFd() == clientFd)
		{
			senderPrefix = _clients[i]->getPrefix();
			break;
		}
	}
	std::string fullMessage = senderPrefix + " PRIVMSG " + target + " :" + msgContent + "\r\n";
	targetClient->sendMessage(fullMessage);
}

std::vector<std::string> split(const std::string &str, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);

	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

void Server::handleMsgCommand(int clientFd, const std::string &message)
{
	std::vector<std::string> tokens = split(message, ' ');
	if (tokens.size() < 3)
	{
		std::string response = ":server 461 MSG :Not enough parameters\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	std::string target = tokens[1];
	std::string msgContent = message.substr(message.find(target) + target.length() + 1);

	if (target.length() > 512)
	{
		std::string response = ":server 412 :Target too long\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}
	if (target[0] == '#' || target[0] == '&')
		handleChannelMessage(clientFd, target, msgContent);
	else
		handlePrivateMessage(clientFd, target, msgContent);
}

void Server::handlePartCommand(int clientFd, const std::string &message)
{
	Client *client = findClientByFd(clientFd);
	if (!client || !client->isRegistered())
		return;

	std::vector<std::string> tokens = ft_split(message, ' ');
	if (tokens.size() < 2)
	{
		std::string response = ":server 461 PART :Not enough parameters\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	std::string channelName = tokens[1];
	std::string partMessage = (tokens.size() > 2) ? message.substr(message.find(channelName) + channelName.length() + 1) : client->getNickname();

	Channel *channel = _channelManager.getChannel(channelName);
	if (!channel)
	{
		std::string response = ":server 403 " + client->getNickname() + " " + channelName + " :No such channel\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	if (!channel->hasMember(clientFd))
	{
		std::string response = ":server 442 " + client->getNickname() + " " + channelName + " :You're not on that channel\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	// Send PART message to channel
	std::string partMsg = client->getPrefix() + " PART " + channelName + " :" + partMessage + "\r\n";
	channel->broadcast(partMsg);

	// Remove client from channel
	channel->removeMember(clientFd);

	// Remove channel if empty
	if (channel->getMemberCount() == 0)
	{
		_channelManager.removeChannel(channelName);
	}

	std::cout << "Client " << client->getNickname() << " left channel " << channelName << std::endl;
}

static std::deque<std::string> readParameters(const std::vector<std::string> &tokens) {
	std::deque<std::string> parameters;
	for (size_t i = 3; i < tokens.size(); i++)
		parameters.push_back(tokens[i]);
	return parameters;
}

static int convertLimitString(std::string const &str) {
	for (size_t i = 0; i < str.length(); i++)
		if (!std::isdigit(str[i]))
			return -1;
	long limit = std::strtol(str.c_str(), NULL, 10);
	if (limit > MAX_LIMIT) return -1;
	return limit;
}

bool Server::setChannelMode(char mode, Client *client, Channel *channel, std::deque<std::string> &parameters) {
	if (mode == 'i' || mode == 't') channel->setMode(mode);
	if (mode == 'k') {
		if (parameters.size() <= 0) {
			std::string response = ":server 461 " + client->getNickname() + " k :Not enough parameters\r\n";
			send(client->getFd(), response.c_str(), response.length(), 0);
			return false;
		}
		channel->setKey(parameters.front());
		parameters.pop_front();
	}
	if (mode == 'o') {
		if (parameters.size() <= 0) {
			std::string response = ":server 461 " + client->getNickname() + " o :Not enough parameters\r\n";
			send(client->getFd(), response.c_str(), response.length(), 0);
			return false;
		}
		Client *targetClient = findClientByNickname(parameters.front());
		parameters.pop_front();

		if (targetClient == NULL) {
			std::string response = ":server 401 " + client->getNickname() + " " + parameters.front() + " :No such nick\r\n";
			send(client->getFd(), response.c_str(), response.length(), 0);
			return false;
		}
		if (!channel->hasMember(targetClient->getFd())) {
			std::string response = ":server 441 " + client->getNickname() + " " + targetClient->getNickname() + " :User not on channel\r\n";
			send(client->getFd(), response.c_str(), response.length(), 0);
			return false;
		}
		int targetFd = targetClient->getFd();
		channel->addOperator(targetFd);
	}
	if (mode == 'l') {
		if (parameters.size() <= 0) {
			std::string response = ":server 461 " + client->getNickname() + " l :Not enough parameters\r\n";
			send(client->getFd(), response.c_str(), response.length(), 0);
			return false;
		}
		std::string limitStr = parameters.front();
		parameters.pop_front();
		int limit = convertLimitString(limitStr);
		if(limit <= 0) {
			std::string err = ":server 467 " + client->getNickname() + " " + channel->getName() + " :Invalid channel limit\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			return false;
		}
		channel->setUserLimit(limit);
	}
	return true;
}

bool Server::unsetChannelMode(char mode, Client *client, Channel *channel, std::deque<std::string> &parameters) {
	if (mode == 'i' || mode == 't') channel->unsetMode(mode);
	if (mode == 'k') {
		channel->setKey("");
	}
	if (mode == 'o') {
		if (parameters.size() <= 0) {
			std::string response = ":server 461 " + client->getNickname() + " o :Not enough parameters\r\n";
			send(client->getFd(), response.c_str(), response.length(), 0);
			return false;
		}
		Client *targetClient = findClientByNickname(parameters.front());
		parameters.pop_front();

		if (targetClient == NULL) {
			std::string response = ":server 401 " + client->getNickname() + " " + parameters.front() + " :No such nick\r\n";
			send(client->getFd(), response.c_str(), response.length(), 0);
			return false;
		}
		if (!channel->hasMember(targetClient->getFd())) {
			std::string response = ":server 441 " + client->getNickname() + " " + targetClient->getNickname() + " :User not on channel\r\n";
			send(client->getFd(), response.c_str(), response.length(), 0);
			return false;
		}
		int targetFd = targetClient->getFd();
		channel->removeOperator(targetFd);
	}
	if (mode == 'l') {
		channel->setUserLimit(0);
	}
	return true;
}

void Server::handleChannelMode(int clientFd, const std::string &target, const std::vector<std::string> &tokens) {
	Client *client = findClientByFd(clientFd);
	if (!client) return;
	if (!client->isRegistered()) {
		std::string response = ":server 451 " + client->getNickname() + " :You have not registered\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	if (!_channelManager.channelExists(target)) {
		std::string response = ":server 403 " + client->getNickname() + " " + target + " :No such channel\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}
	Channel *channel = _channelManager.getChannel(target);

	if (!channel->hasMember(clientFd)) {
		std::string response = ":server 442 " + client->getNickname() + " " + target + " :You're not on that channel\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}
	if (!channel->isOperator(clientFd)) {
		std::string response = ":server 482 " + client->getNickname() + " " + target + " :You're not channel operator\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	if (tokens.size() < 3) {
		std::string currentModes = channel->getModeString();
		std::string response = ":server 324 " + client->getNickname() + " " + target + " " + currentModes + "\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	bool adding = false;
	std::deque<std::string> parameters = readParameters(tokens);
	std::string modes = tokens[2];
	if (modes[0] != '-' && modes[0] != '+') {
		std::string response = ":server 472 " + client->getNickname() + " " + modes + " :is unknown mode char to me\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}
	std::string currentModes;
	for (size_t i = 0; i < modes.length(); i++) {
		if (modes[i] == '+') {
			currentModes += '+';
			adding = true;
		}
		else if (modes[i] == '-') {
			currentModes += '-';
			adding = false;
		}
		else if (modes[i] == 'i' || modes[i] == 't' || modes[i] == 'k' || modes[i] == 'o' || modes[i] == 'l') {
			if (adding) {
				if (setChannelMode(modes[i], client, channel, parameters))
					currentModes += modes[i];
			} else {
				if (unsetChannelMode(modes[i], client, channel, parameters))
					currentModes += modes[i];
			}
		} else {
			std::string response = ":server 472 " + client->getNickname() + " " + std::string(1, modes[i]) + " :is unknown mode char to me\r\n";
			send(clientFd, response.c_str(), response.length(), 0);
		}
	}
	std::string modeChangeMsg = client->getPrefix() + " MODE " + target + " " + currentModes;
	channel->broadcast(modeChangeMsg + "\r\n");
}

void Server::handleModeCommand(int clientFd, const std::string &message)
{
	std::vector<std::string> tokens = split(message, ' ');
	if (tokens.size() < 2)
	{
		std::string response = ":server 461 MODE :Not enough parameters\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	std::string target = tokens[1];

	if (target[0] == '#' || target[0] == '&')
	{
		handleChannelMode(clientFd, target, tokens);
	}
	else
	{
		std::string response = ":server 502 :User modes are not supported\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
	}
}

void Server::joindefaultChannel(int clientFd)
{
	const std::string defaultChannel = "#general";
	Client *client = findClientByFd(clientFd);
	if (!client)
		return;
	if (!client->isInChannel(defaultChannel))
	{
		if (client->isRegistered())
		{
			handleJoinCommand(clientFd, "JOIN " + defaultChannel);
		}
	}
}

void Server::handleClientEvent(int i)// to do chenachne parsing after reciving message and change else if to switch case
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

		_channelManager.removeClientFromAllChannels(clientFd); // Comment out for now

		for (size_t j = 0; j < _clients.size(); ++j)
		{
			if (_clients[j]->getFd() == clientFd)
			{
				delete _clients[j];
				_clients.erase(_clients.begin() + j);
				break;
			}
		}
		close(clientFd);
		_pfds.erase(_pfds.begin() + i);
		return;
	}
	buf[bytes] = '\0';
	std::string message(buf);
	Client *client = findClientByFd(clientFd);
	if (client)
	{
		client->appendBuffer(message);
		while (client->hasCompleteCommand())
		{
			std::string command = client->extractCommand();
			std::string trimmed = command;
			trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
			trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
			if (trimmed.empty())
				continue;
			std::cout << "Received command from " << client->getNickname() << ": " << command << std::endl;

			if (command.find("PASS") == 0)
				handlePassCommand(clientFd, command);
			else if (command.find("NICK") == 0)
				handleNickCommand(clientFd, command);
			else if (command.find("USER") == 0)
				handleUserCommand(clientFd, command);
			else if (command.find("QUIT") == 0)
			{
				// Handle QUIT
			}
			else if (command.find("WHO") == 0)
				handleWhoCommand(clientFd, command);
			else if (command.find("CAP LS 302") == 0)
			{
				std::cout << "Handling LS command" << std::endl;
				std::string capMessage = ":server : CAP * LS :multi-prefix away-notify extended-join account-notify";
				std::string output = capMessage + "\r\n";
				send(clientFd, output.c_str(), output.size(), 0);
			}
			else if(command.find("PING") == 0)
			{
				std::string response = "PONG" + command.substr(4) + "\r\n";
				send(clientFd, response.c_str(), response.length(), 0);
			}
			else if (command.find("JOIN") == 0)
				handleJoinCommand(clientFd, command);
			else if (command.find("MODE") == 0)
				handleModeCommand(clientFd, command);
			else if (command.find("PART") == 0)
				handlePartCommand(clientFd, command);
			else if (command.find("MSG") == 0 || command.find("PRIVMSG") == 0)
				handleMsgCommand(clientFd, command);
			else if (command.find("INVITE") == 0)
				handleInviteCommand(clientFd, command);
			else if (command.find("KICK") == 0)
				handleKickCommand(clientFd, command);
			else if (command.find("TOPIC") == 0)
				handleTopicCommand(clientFd, command);
			else
			{
				if (client->isRegistered())
				{
					if (command.find("JOIN") == 0)
						handleJoinCommand(clientFd, command);
					else if (command.find("PRIVMSG") == 0)
						handleMsgCommand(clientFd, command);
					else if (command.find("PART") == 0)
					{
						handlePartCommand(clientFd, command);
					}
					else
					{
						if (command.length() > 0 && command[0] != ':')
						{
							std::string response = "421 " + command + " :Unknown command\r\n";
							send(clientFd, response.c_str(), response.length(), 0);
						}
					}
				}
				else
				{
					std::string response = "451 :You have not registered\r\n";
					send(clientFd, response.c_str(), response.length(), 0);
				}
			}
		}
	}
}

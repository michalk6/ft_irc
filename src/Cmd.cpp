#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <ostream>
#include "Channel.hpp"
#include "ChannelMenager.hpp"

void Server::handleChannelNotice(int clientFd, const std::string &channelName, const std::string &msgContent)
{
	const std::map<std::string, Channel *> &channels = _channelManager.getChannels();

	Client *sender = findClientByFd(clientFd);
	if (!sender)
		return;

	// check if channel exists - use const_iterator
	std::map<std::string, Channel *>::const_iterator it = channels.find(channelName);
	if (it == channels.end())
		return;

	Channel *channel = it->second;

	// check if client is in channel
	const std::map<int, Client *> &members = channel->getMembers();
	if (members.find(clientFd) == members.end())
		return;

	std::string filtered = _bot.filterMessage(msgContent);
	// create full message (NOTICE)
	std::string fullMessage = sender->getPrefix() + " NOTICE " + channelName + " :" + filtered + "\r\n";

	// send to all clients in channel except sender
	channel->broadcast(fullMessage, sender);
}

void Server::handlePrivateNotice(int clientFd, const std::string &target, const std::string &msgContent)
{
	// Find the target client by nickname
	Client *targetClient = findClientByNickname(target);
	if (!targetClient)
		return;

	Client *sender = findClientByFd(clientFd);
	if (!sender)
		return;
	std::string senderPrefix = sender->getPrefix();

	std::string fullMessage = senderPrefix + " NOTICE " + target + " :" + msgContent + "\r\n";
	targetClient->sendMessage(fullMessage);
}

void Server::handleNoticeCommand(int clientFd, const std::string &message)
{
	std::vector<std::string> tokens = Server::ft_split(message, ' ');
	if (tokens.size() < 3)
		return;

	std::string target = tokens[1];
	std::string msgContent = message.substr(message.find(target) + target.length() + 1);
	if(msgContent[0]==':')
		msgContent = msgContent.substr(1);

	if (target.length() > 512)
		return;
	if (target[0] == '#' || target[0] == '&')
		handleChannelNotice(clientFd, target, msgContent);
	else
		handlePrivateNotice(clientFd, target, msgContent);
}

void Server::handleChannelMessage(int clientFd, const std::string &channelName, const std::string &msgContent)
{
	const std::map<std::string, Channel *> &channels = _channelManager.getChannels();

	Client *sender = findClientByFd(clientFd);
	if (!sender)
		return;

	// check if channel exists - use const_iterator
	std::map<std::string, Channel *>::const_iterator it = channels.find(channelName);
	if (it == channels.end())
	{
		std::string response = ":server 403 " + sender->getNickname() + " " + channelName + " :No such channel\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	Channel *channel = it->second;

	// check if client is in channel
	const std::map<int, Client *> &members = channel->getMembers();
	if (members.find(clientFd) == members.end())
	{
		std::string response = ":server 442 " + sender->getNickname() + " " + channelName + " :You're not on that channel\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	std::string filtered = _bot.filterMessage(msgContent);
	// create full message (PRIVMSG)
	std::string fullMessage = sender->getPrefix() + " PRIVMSG " + channelName + " :" + filtered + "\r\n";

	// send to all clients in channel except sender
	channel->broadcast(fullMessage, sender);
}

void Server::handlePrivateMessage(int clientFd, const std::string &target, const std::string &msgContent)
{
	// Find the target client by nickname
	Client *targetClient = findClientByNickname(target);
	if (!targetClient)
	{
		std::string response = ":server 401 " + target + " :No such nick/channel\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	Client *sender = findClientByFd(clientFd);
	if (!sender)
		return;
	std::string senderPrefix = sender->getPrefix();

	std::string fullMessage = senderPrefix + " PRIVMSG " + target + " :" + msgContent + "\r\n";
	targetClient->sendMessage(fullMessage);
}

void Server::handleMsgCommand(int clientFd, const std::string &message)
{
	std::vector<std::string> tokens = Server::ft_split(message, ' ');
	if (tokens.size() < 3)
	{
		std::string response = ":server 461 MSG :Not enough parameters\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
		return;
	}

	std::string target = tokens[1];
	std::string msgContent = message.substr(message.find(target) + target.length() + 1);
	if(msgContent[0]==':')
		msgContent = msgContent.substr(1);
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
		
		// get nickname before checking if user exists
		std::string targetNick = parameters.front();
		parameters.pop_front();

		Client *targetClient = findClientByNickname(targetNick);
		if (targetClient == NULL) {
			std::string response = ":server 401 " + client->getNickname() + " " + targetNick + " :No such nick\r\n";
			send(client->getFd(), response.c_str(), response.length(), 0);
			return false;
		}
		if (!channel->hasMember(targetClient->getFd())) {
			// FIX: Add channel name to error response
			std::string response = ":server 441 " + client->getNickname() + " " + targetNick + " " + channel->getName() + " :User not on channel\r\n";
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
		
		// get nickname before checking if user exists
		std::string targetNick = parameters.front();
		parameters.pop_front();

		Client *targetClient = findClientByNickname(targetNick);
		if (targetClient == NULL) {
			std::string response = ":server 401 " + client->getNickname() + " " + targetNick + " :No such nick\r\n";
			send(client->getFd(), response.c_str(), response.length(), 0);
			return false;
		}
		if (!channel->hasMember(targetClient->getFd())) {
			// FIX: Add channel name to error response
			std::string response = ":server 441 " + client->getNickname() + " " + targetNick + " " + channel->getName() + " :User not on channel\r\n";
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
	std::string modeParams;
	
	// Create a copy of parameters for building the message
	std::deque<std::string> messageParams = parameters;
	
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
				if (setChannelMode(modes[i], client, channel, parameters)) {
					currentModes += modes[i];
					// Add parameters for modes that require them
					if ((modes[i] == 'k' || modes[i] == 'o' || modes[i] == 'l') && !messageParams.empty()) {
						modeParams += " " + messageParams.front();
						messageParams.pop_front();
					}
				}
			} else {
				if (unsetChannelMode(modes[i], client, channel, parameters)) {
					currentModes += modes[i];
					// Add parameters for modes that require them (like o)
					if (modes[i] == 'o' && !messageParams.empty()) {
						modeParams += " " + messageParams.front();
						messageParams.pop_front();
					}
				}
			}
		} else {
			std::string response = ":server 472 " + client->getNickname() + " " + std::string(1, modes[i]) + " :is unknown mode char to me\r\n";
			send(clientFd, response.c_str(), response.length(), 0);
		}
	}
	
	std::string modeChangeMsg = client->getPrefix() + " MODE " + target + " " + currentModes + modeParams + "\r\n";
	channel->broadcast(modeChangeMsg);
}

void Server::handleModeCommand(int clientFd, const std::string &message)
{
	std::vector<std::string> tokens = Server::ft_split(message, ' ');
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

// ====================================================================
// client event handling:
// ====================================================================

void Server::handleClientEvent(int i)
{
	char buf[512];
	int clientFd = _pfds[i].fd;
	int bytes = recv(clientFd, buf, sizeof(buf) - 1, 0);

	if (bytes <= 0)
	{
		handleClientDisconnect(i, clientFd, bytes);
		return;
	}

	processClientMessage(clientFd, buf, bytes);
}

void Server::removeClientFromVector(int clientFd)
{
	for (size_t j = 0; j < _clients.size(); ++j)
	{
		if (_clients[j]->getFd() == clientFd)
		{
			delete _clients[j];
			_clients.erase(_clients.begin() + j);
			break;
		}
	}
}

void Server::handleClientDisconnect(int index, int clientFd, int bytes) {
	if (bytes == 0)
		std::cout << "Client disconnected (fd=" << clientFd << ")" << std::endl;
	else
		std::cerr << "recv() error on fd " << clientFd << std::endl;

	Client* disconnectedClient = findClientByFd(clientFd);
	
	if (disconnectedClient) {
		// 1. Remove client from all channels
		_channelManager.removeClientFromAllChannels(clientFd);
		
		// 2. Send QUIT message to all channels the client was in
		std::string quitMsg = disconnectedClient->getPrefix() + " QUIT :Client disconnected\r\n";
		const std::set<std::string>& channels = disconnectedClient->getChannels();
		for (std::set<std::string>::const_iterator it = channels.begin(); 
			it != channels.end(); ++it) {
			Channel* channel = _channelManager.getChannel(*it);
			if (channel) {
				channel->broadcast(quitMsg);
			}
		}
	}
	
	// 3. Close socket
	close(clientFd);
	
	// 4. Safe removal - mark for later cleanup
	_pfds[index].fd = -1;
	_clientsToRemove.push_back(clientFd);
}

// In the main loop, after processing all events:
void Server::cleanupDisconnectedClients() {
	// Remove from _pfds
	for (std::vector<pollfd>::iterator it = _pfds.begin(); it != _pfds.end(); ) {
		if (it->fd == -1) {
			it = _pfds.erase(it);
		} else {
			++it;
		}
	}
	
	// Remove from _clients and free memory
	for (std::vector<int>::iterator it = _clientsToRemove.begin(); 
		it != _clientsToRemove.end(); ++it) {
		removeClientFromVector(*it);
	}
	_clientsToRemove.clear();
}

void Server::processClientMessage(int clientFd, char* buf, int bytes)
{
	buf[bytes] = '\0';
	std::string message(buf);
	Client *client = findClientByFd(clientFd);
	
	if (!client)
		return;

	client->appendBuffer(message);
	
	while (client->hasCompleteCommand())
	{
		std::string command = client->extractCommand();
		std::string trimmed = trimCommand(command);
		
		if (trimmed.empty())
			continue;
			
		processSingleCommand(client, clientFd, trimmed);
		
		// Check if client still exists after command processing
		if (findClientByFd(clientFd) == NULL)
			return;
	}
}

std::string Server::trimCommand(const std::string& command)
{
	std::string trimmed = command;
	trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
	trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
	return trimmed;
}

void Server::processSingleCommand(Client* client, int clientFd, const std::string& command)
{
	// ignore server messages starting with ':'
	if (!command.empty() && command[0] == ':') {
		std::cout << "Ignoring server message: " << command << std::endl;
		return;
	}
	
	std::cout << "Received command from " << (client->getNickname().empty() ? "unknown" : client->getNickname()) << ": " << command << std::endl;

	std::vector<std::string> tokens = ft_split(command, ' ');
	std::string cmd = tokens.empty() ? "" : tokens[0];

	// add logging for MODE commands
	if (cmd == "MODE") {
		std::cout << "Processing MODE command for channel: " << (tokens.size() > 1 ? tokens[1] : "none") << std::endl;
	}

	if (handleCapabilityCommands(clientFd, tokens, cmd))
		return;
	
	if (handleAuthenticationCommands(clientFd, tokens, cmd))
		return;

	if (!client->isRegistered())
	{
		sendNotRegisteredError(clientFd);
		return;
	}

	handleRegisteredCommands(clientFd, tokens, cmd, command);
}

bool Server::handleCapabilityCommands(int clientFd, const std::vector<std::string>& tokens, const std::string& cmd)
{
	if (cmd != "CAP")
		return false;

	std::cout << "Handling CAP command" << std::endl;
	
	if (tokens.size() >= 2)
	{
		if (tokens[1] == "LS")
		{
			std::string response = "CAP * LS :\r\n";
			send(clientFd, response.c_str(), response.length(), 0);
		}
		else if (tokens[1] == "END")
		{
			std::cout << "CAP negotiation ended for client " << clientFd << std::endl;
		}
		else if (tokens[1] == "REQ")
		{
			std::string response = "CAP * NAK :\r\n";
			send(clientFd, response.c_str(), response.length(), 0);
		}
	}
	return true;
}

std::string Server::joinTokens(const std::vector<std::string>& tokens)
{
	if (tokens.empty()) return "";
	
	std::string result;
	for (size_t i = 0; i < tokens.size(); ++i)
	{
		if (i > 0) result += " ";
		result += tokens[i];
	}
	return result;
}

bool Server::handleAuthenticationCommands(int clientFd, const std::vector<std::string>& tokens, const std::string& cmd)
{
	if (cmd == "PASS")
	{
		handlePassCommand(clientFd, joinTokens(tokens)); // vector -> string
		return true;
	}
	else if (cmd == "NICK")
	{
		handleNickCommand(clientFd, joinTokens(tokens));
		return true;
	}
	else if (cmd == "USER")
	{
		handleUserCommand(clientFd, joinTokens(tokens));
		return true;
	}
	
	return false;
}
// #include <fstream>
// #include <sstream>
// #include <algorithm>

// // Base64 character set
// static const std::string base64_chars =
// 	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
// 	"abcdefghijklmnopqrstuvwxyz"
// 	"0123456789+/";

// // Base64 encode function (C++98 compatible)
// static std::string base64_encode(const std::string &in) {
// 	std::string out;
// 	int val = 0, valb = -6;

// 	for (std::string::const_iterator it = in.begin(); it != in.end(); ++it) {
// 		unsigned char c = *it;
// 		val = (val << 8) + c;
// 		valb += 8;
// 		while (valb >= 0) {
// 			out.push_back(base64_chars[(val >> valb) & 0x3F]);
// 			valb -= 6;
// 		}
// 	}
// 	if (valb > -6) {
// 		out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
// 	}
// 	while (out.size() % 4) {
// 		out.push_back('=');
// 	}
// 	return out;
// }

// // Base64 decode function (C++98 compatible)
// static std::string base64_decode(const std::string &in) {
// 	std::vector<int> T(256, -1);
// 	for (int i = 0; i < 64; i++) {
// 		T[(unsigned char)base64_chars[i]] = i;
// 	}

// 	std::string out;
// 	std::vector<int> buf(4);
// 	int pad_count = 0;

// 	for (size_t i = 0; i < in.size();) {
// 		int len = 0;
// 		for (int j = 0; j < 4 && i < in.size(); ++i) {
// 			unsigned char c = in[i];
// 			if (c == '=') {
// 				buf[j++] = 0;
// 				++pad_count;
// 				len++;
// 				break;
// 			}
// 			if (T[c] != -1) {
// 				buf[j++] = T[c];
// 				++len;
// 			}
// 		}
// 		if (len == 0) break;
// 		out.push_back((buf[0] << 2) | ((buf[1] & 0x30) >> 4));
// 		if (len > 2) out.push_back(((buf[1] & 0xf) << 4) | ((buf[2] & 0x3c) >> 2));
// 		if (len > 3) out.push_back(((buf[2] & 0x3) << 6) | buf[3]);
// 	}
// 	return out;
// }

// // ATOB wrapper
// static std::string ATOB(const std::string &s) {
// 	return base64_decode(s);
// }

// // Function to save decoded base64 to file
// static bool saveBase64ToFile(const std::string &outPath, const std::string &base64data) {
// 	std::string bin = ATOB(base64data);
// 	std::ofstream ofs(outPath.c_str(), std::ios::binary);
// 	if (!ofs.is_open()) return false;
// 	ofs.write(bin.data(), static_cast<std::streamsize>(bin.size()));
// 	return true;
// }

// void Server::handleSendCommand(int clientFd, const std::string &message)
// {
// 	Client *client = findClientByFd(clientFd);
// 	if (!client || !client->isRegistered())
// 		return ;
// 	std::vector<std::string> tokens = ft_split(message, ' ');
// 	if (tokens.size() < 2)
// 	{
// 		std::string response = ":server 461 SEND :Not enough parameters\r\n";
// 		send(clientFd, response.c_str(), response.length(), 0);
// 		return ;
// 	}
// 	std::string filePath = message.substr(message.find(' ') + 1);

// 	std::ifstream ifs(filePath.c_str(), std::ios::binary);
// 	if (!ifs.is_open())
// 	{
// 		std::string response = ":server 404 " + client->getNickname() + " " + filePath + " :No such file\r\n";
// 		send(clientFd, response.c_str(), response.length(), 0);
// 		return;
// 	}

// 	std::ostringstream ss;
// 	ss << ifs.rdbuf();
// 	std::string fileData = ss.str();
// 	ifs.close();

// 	// Encode to base64
// 	std::string encoded = base64_encode(fileData);

// 	// Determine filename to advertise (basename)
// 	std::string filename = filePath;
// 	size_t pos = filename.find_last_of("/\\");
// 	if (pos != std::string::npos) {
// 		filename = filename.substr(pos + 1);
// 	}

// 	// Send file data - replace with available Bot method or direct channel messaging
// 	// Since sendMessageToChannel doesn't exist, use handleChannelMessage instead
// 	const std::string CHANNEL_NAME = "#general";

// 	// Send in chunks to avoid huge single messages
// 	const size_t CHUNK_SIZE = 4000;
// 	for (size_t i = 0; i < encoded.size(); i += CHUNK_SIZE)
// 	{
// 		std::string chunk = encoded.substr(i, std::min(CHUNK_SIZE, encoded.size() - i));
// 		// Format: FILE <filename> :<base64_chunk>
// 		std::string fileMsg = "FILE " + filename + " :" + chunk;
// 		handleChannelMessage(clientFd, CHANNEL_NAME, fileMsg);
// 	}

// 	// Signal end of file
// 	std::string endMsg = "FILE_END " + filename;
// 	handleChannelMessage(clientFd, CHANNEL_NAME, endMsg);
// }

// void Server::handleFileCommand(Server *server, int clientFd, const std::string &message)
// {
// 	Client *client = server->findClientByFd(clientFd);
// 	if (!client || !client->isRegistered())
// 		return ;

// 	std::vector<std::string> tokens = ft_split(message, ' ');
// 	if (tokens.size() < 2)
// 		return ;

// 	std::string command = tokens[0];
// 	std::string filename = tokens[1];

// 	if (command == "FILE")
// 	{
// 		// Extract base64 data
// 		size_t pos = message.find(':');
// 		if (pos == std::string::npos)
// 			return ;
// 		std::string base64data = message.substr(pos + 1);

// 		// Save to temporary file
// 		std::string tempFilePath = "./received_" + filename;
// 		if (!saveBase64ToFile(tempFilePath, base64data))
// 		{
// 			std::cerr << "Failed to save file chunk for " << filename << std::endl;
// 			return ;
// 		}

// 		std::cout << "Received file chunk for " << filename << " from " << client->getNickname() << std::endl;
// 	}
// 	else if (command == "FILE_END")
// 	{
// 		std::cout << "Completed receiving file " << filename << " from " << client->getNickname() << std::endl;
// 	}
// }

void Server::handleRegisteredCommands(int clientFd, const std::vector<std::string>& tokens, const std::string& cmd, 
				const std::string& fullCommand)
{
	// Proste if/else zamiast mapy - bardziej niezawodne
	if (cmd == "PING") {
		handlePingCommand(clientFd, tokens);
	}
	// else if(cmd == "FILE" || cmd == "FILE_END") {
	// 	handleFileCommand(this, clientFd, fullCommand);
	// }
	// else if(cmd == "SEND") {
	// 	handleSendCommand(clientFd, fullCommand);
	// }
	else if (cmd == "JOIN") {
		handleJoinCommand(clientFd, fullCommand);
	}
	else if (cmd == "MODE") {
		handleModeCommand(clientFd, fullCommand);
	}
	else if (cmd == "PART") {
		handlePartCommand(clientFd, fullCommand);
	}
	else if (cmd == "MSG" || cmd == "PRIVMSG") {
		handleMsgCommand(clientFd, fullCommand);
	}
	else if (cmd == "NOTICE") {
		handleNoticeCommand(clientFd, fullCommand);
	}
	else if (cmd == "INVITE") {
		handleInviteCommand(clientFd, fullCommand);
	}
	else if (cmd == "KICK") {
		handleKickCommand(clientFd, fullCommand);
	}
	else if (cmd == "TOPIC") {
		handleTopicCommand(clientFd, fullCommand);
	}
	else if (cmd == "WHO") {
		handleWhoCommand(clientFd, fullCommand);
	}
	else if (cmd == "QUIT") {
		handleQuitCommand(clientFd, fullCommand);
	}
	else {
		sendUnknownCommandError(clientFd, cmd);
	}
}

void Server::handleQuitCommand(int clientFd, const std::string &message)
{
	Client *client = findClientByFd(clientFd);
	if (!client) return;

	std::string quitMessage = "Client quit";
	std::vector<std::string> tokens = ft_split(message, ' ');
	
	// Extract quit message if provided
	if (tokens.size() >= 2) {
		size_t pos = message.find(':');
		if (pos != std::string::npos) {
			quitMessage = message.substr(pos + 1);
		}
	}

	// Broadcast quit message to all channels the client is in
	const std::set<std::string>& clientChannels = client->getChannels();
	for (std::set<std::string>::const_iterator it = clientChannels.begin(); it != clientChannels.end(); ++it) {
		Channel *channel = _channelManager.getChannel(*it);
		if (channel) {
			std::string quitMsg = client->getPrefix() + " QUIT :" + quitMessage + "\r\n";
			channel->broadcast(quitMsg, client);
		}
	}

	// Remove client from all channels
	_channelManager.removeClientFromAllChannels(clientFd);

	// Send error response to client (optional)
	std::string response = "ERROR :Closing link: " + client->getNickname() + " [Quit: " + quitMessage + "]\r\n";
	send(clientFd, response.c_str(), response.length(), 0);

	// Close connection and remove client
	std::cout << "Client " << client->getNickname() << " quit: " << quitMessage << std::endl;
	
	// Find and remove from pfds
	for (size_t i = 0; i < _pfds.size(); ++i) {
		if (_pfds[i].fd == clientFd) {
			close(clientFd);
			_pfds.erase(_pfds.begin() + i);
			break;
		}
	}
	
	// Remove from clients vector
	removeClientFromVector(clientFd);
}

void Server::handlePingCommand(int clientFd, const std::vector<std::string>& tokens)
{
	std::string token = (tokens.size() > 1) ? tokens[1] : "";
	std::string response = "PONG :" + token + "\r\n";
	send(clientFd, response.c_str(), response.length(), 0);
}

void Server::sendNotRegisteredError(int clientFd)
{
	std::string response = "451 :You have not registered\r\n";
	send(clientFd, response.c_str(), response.length(), 0);
}

void Server::sendUnknownCommandError(int clientFd, const std::string& command)
{
	if (!command.empty() && command[0] != ':')
	{
		std::string response = "421 " + command + " :Unknown command\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
	}
}

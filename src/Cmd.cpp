
#include "Server.hpp"
#include "IRCClient.hpp"

// void Server::handleTopicCommand(int clientFd, const std::string &message)
// {
// }

// void Server::handleInviteCommand(int clientFd, const std::string &message)

// 	void Server::handleKickCommand(int clientFd, const std::string &message)


// void Server::handleChannelMessage(int clientFd, const std::string &target, const std::string &msgContent)


void Server::handelePrivateMessage(int clientFd, const std::string &target, const std::string &msgContent)
{
	// Find the target client by nickname
	IRCClient* targetClient = NULL;
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
	{
		// handleChannelMessage(clientFd, target, msgContent);//to do
	}
	else
	{
		// handlePrivateMessage(clientFd, target, msgContent);//to do
	}
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
		// handleChannelMode(clientFd, tokens);//to do
	}
	else
	{
		std::string response = ":server 502 :User modes are not supported\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
	}
}

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
	std::string message(buf);

	if (message.find("KICK") == 0)
	{
		// handleKickCommand(clientFd, message);
	}
	else if (message.find("INVITE") == 0)
	{
		// handleInviteCommand(clientFd, message);
	}
	else if (message.find("TOPIC") == 0)
	{
		// handleTopicCommand(clientFd, message);
	}
	else if (message.find("MODE") == 0)
	{
		handleModeCommand(clientFd, message);
	}
	else if (message.find("MSG") == 0)
	{
		handleMsgCommand(clientFd, message);
	}
	else
	{
		std::string response = ":server 421 " + message.substr(0, message.find(' ')) + " :Unknown command\r\n";
		send(clientFd, response.c_str(), response.length(), 0);
	}
}

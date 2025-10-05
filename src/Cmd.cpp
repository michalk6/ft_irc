#include "Server.hpp"
#include "Client.hpp"
#include <iostream>
#include <ostream>
#include "Channel.hpp"
#include "ChannelMenager.hpp"
// void Server::handleTopicCommand(int clientFd, const std::string &message)
// {
// }

// void Server::handleInviteCommand(int clientFd, const std::string &message)

// 	void Server::handleKickCommand(int clientFd, const std::string &message)

// void Server::handleChannelMessage(int clientFd, const std::string &target, const std::string &msgContent)

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
	{
		// handleChannelMessage(clientFd, target, msgContent);//to do
	}
	else
		handlePrivateMessage(clientFd, target, msgContent);//to do
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
			{
				handleMsgCommand(clientFd, command);
			}
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

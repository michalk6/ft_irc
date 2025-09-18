#include "IRCChannel.hpp"
#include "IRCClient.hpp"

IRCChannel::IRCChannel(const std::string &channelName)
	: name(channelName) {}

void IRCChannel::broadcast(const std::string &message, IRCClient *exclude) const
{
	for (std::map<int, IRCClient *>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (!exclude || it->second != exclude)
		{
			it->second->sendMessage(message);
		}
	}
}

bool IRCChannel::isOperator(int clientFd) const
{
	return operators.count(clientFd) > 0;
}

void IRCChannel::addMember(IRCClient *client)
{
	if (!client) return;
	members[client->getFd()] = client;
	if (operators.empty())
	{
		operators.insert(client->getFd());
	}
}

void IRCChannel::removeMember(int clientFd)
{
	members.erase(clientFd);
	operators.erase(clientFd);
}

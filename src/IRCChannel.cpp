#include "IRCChannel.hpp"
#include "IRCClient.hpp"

IRCChannel::IRCChannel(const std::string &channelName) : name(channelName) {}

void IRCChannel::broadcast(const std::string &message, IRCClient *exclude) const
{
	for (std::map<int, IRCClient *>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (exclude == NULL || it->second != exclude)
		{
			it->second->sendMessage(message);
		}
	}
}

bool IRCChannel::isOperator(int clientFd) const
{
	return operators.find(clientFd) != operators.end();
}

void IRCChannel::addMember(IRCClient *client)
{
	members[client->fd] = client;
	if (operators.empty())
	{
		operators.insert(client->fd);
	}
}

void IRCChannel::removeMember(int clientFd)
{
	members.erase(clientFd);
	operators.erase(clientFd);
}

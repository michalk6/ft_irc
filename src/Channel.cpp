#include "Channel.hpp"
#include "Client.hpp"

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// 															PUBLIC:
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

// ====================================================================
// Orthodox Canonical Form elements:
// ====================================================================

// constructor:
Channel::Channel(const std::string &channelName)
	: name(channelName) {}


// destructor:
Channel::~Channel() {}

// ====================================================================
// methods:
// ====================================================================

// broadcasts a message to all channel members except the specified excluded client
void Channel::broadcast(const std::string &message, Client *exclude) const
{
	for (std::map<int, Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (!exclude || it->second != exclude)
		{
			it->second->sendMessage(message);
		}
	}
}

// check if the client with the given file descriptor is a channel operator
bool Channel::isOperator(int clientFd) const
{
	return operators.count(clientFd) > 0;
}

// add a client to the channel and automatically promotes the first member to operator if the channel has no operators
void Channel::addMember(Client *client)
{
	if (!client) return;
	members[client->getFd()] = client;
	if (operators.empty())
	{
		operators.insert(client->getFd());
	}
}

// remove a client from both members and operators lists of the channel
void Channel::removeMember(int clientFd)
{
	members.erase(clientFd);
	operators.erase(clientFd);
}

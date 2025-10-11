// Channel.cpp - zaktualizuj:
#include "Channel.hpp"
#include "Client.hpp"
#include <algorithm>
#include <sstream>

Channel::Channel(const std::string &channelName)
	: name(channelName), topic(""), userLimit(0) {}

Channel::~Channel() {}

std::string Channel::getName() const {
	return this->name;
}

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

bool Channel::isOperator(int clientFd) const
{
	return operators.find(clientFd) != operators.end();
}

void Channel::addMember(Client *client)
{
	if (!client)
		return;
	members[client->getFd()] = client;
	client->addChannel(name);

	// First member becomes operator
	if (members.size() == 1)
	{
		addOperator(client->getFd());
	}
}

void Channel::removeMember(int clientFd)
{
	Client *client = members[clientFd];
	if (client)
	{
		client->removeChannel(name);
	}
	members.erase(clientFd);
	operators.erase(clientFd);
}

void Channel::addOperator(int clientFd)
{
	operators.insert(clientFd);
}

void Channel::removeOperator(int clientFd)
{
	operators.erase(clientFd);
}

bool Channel::hasMember(int clientFd) const
{
	return members.find(clientFd) != members.end();
}

std::vector<std::string> Channel::getMemberNicknames() const
{
	std::vector<std::string> nicknames;
	for (std::map<int, Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		nicknames.push_back(it->second->getNickname());
	}
	return nicknames;
}

size_t Channel::getMemberCount() const
{
	return members.size();
}

void Channel::setTopic(const std::string &newTopic)
{
	topic = newTopic;
}

const std::string &Channel::getTopic() const
{
	return topic;
}

void Channel::setKey(const std::string &key) {
	this->key = key;
}

std::string Channel::getKey() const {
	return this->key;
}

void Channel::setUserLimit(int userLimit) {
	this->userLimit = userLimit;
}

int Channel::getUserLimit() const {
	return this->userLimit;
}


void Channel::addInvitation(int fd) {
	invitations.insert(fd);
}

void Channel::removeInvitation(int fd) {
	invitations.erase(fd);
}

bool Channel::isInvited(int fd) const {
	return invitations.find(fd) != invitations.end();
}

void Channel::setMode(char mode)
{
	modes.insert(mode);
}

void Channel::unsetMode(char mode)
{
	modes.erase(mode);
}

bool Channel::hasMode(char mode) const
{
	return modes.find(mode) != modes.end();
}

std::string Channel::getModeString() const {
	std::string modes("+");
	std::string params;

	if (this->hasMode('i')) modes += 'i';
	if (this->hasMode('t')) modes += 't';
	if (!this->key.empty()) {
		modes += 'k';
		params += (" " + this->key);
	}
	if (this->userLimit > 0) {
		modes += 'l';
		std::stringstream ss;
		ss << this->userLimit;
		params += (" " + ss.str());
	}

	return (modes == "+") ? "" : modes + params;
}

bool Client::isInChannel(const std::string &channelName) const
{
	return _channels.find(channelName) != _channels.end();
}

const std::map<int, Client *> &Channel::getMembers() const {
	return members;
}

// ChannelManager.cpp
#include "ChannelMenager.hpp"
#include <algorithm>

ChannelManager::ChannelManager() {}

ChannelManager::~ChannelManager() {
	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
		delete it->second;
	}
	channels.clear();
}

Channel* ChannelManager::createChannel(const std::string& name) {
	if (channelExists(name))
		return getChannel(name);
	Channel* channel = new Channel(name);
	channels[name] = channel;
	return channel;
}

Channel* ChannelManager::getChannel(const std::string& name) {
	std::map<std::string, Channel*>::iterator it = channels.find(name);
	if (it != channels.end()) {
		return it->second;
	}
	return NULL;
}

bool ChannelManager::channelExists(const std::string& name) {
	return channels.find(name) != channels.end();
}

void ChannelManager::removeChannel(const std::string& name) {
	std::map<std::string, Channel*>::iterator it = channels.find(name);
	if (it != channels.end()) {
		delete it->second;
		channels.erase(it);
	}
}

void ChannelManager::removeClientFromAllChannels(int clientFd) {
	std::vector<std::string> toRemove;
	
	// collect all channels the client is in
	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
		if (it->second->hasMember(clientFd)) {
			it->second->removeMember(clientFd);
		}
	}
	
	// find empty channels
	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ) {
		if (it->second->getMemberCount() == 0) {
			toRemove.push_back(it->first);
			++it;
		} else {
			++it;
		}
	}
	
	// remove empty channels
	for (size_t i = 0; i < toRemove.size(); ++i) {
		removeChannel(toRemove[i]);
	}
}

std::vector<std::string> ChannelManager::getChannelNames() const {
	std::vector<std::string> names;
	for (std::map<std::string, Channel*>::const_iterator it = channels.begin(); it != channels.end(); ++it) {
		names.push_back(it->first);
	}
	return names;
}

const std::map<std::string, Channel*> &ChannelManager::getChannels() const {
	return channels;
}

// ChannelManager.hpp
#ifndef CHANNELMANAGER_HPP
#define CHANNELMANAGER_HPP

#include "Channel.hpp"
#include <map>
#include <string>

class ChannelManager {

	private:
		std::map<std::string, Channel*> channels;											// map of channels

		// orthodox canonical form:
		ChannelManager(const ChannelManager &other);										// copy constructor
		ChannelManager& operator=(const ChannelManager &other);								// copy assignment operator

	public:
		// orthodox canonical form:
		ChannelManager();																	// default constructor
		~ChannelManager();																	// destructor

		Channel*								createChannel(const std::string& name);		// create channel
		bool									channelExists(const std::string& name);		// check if channel exists
		Channel*								getChannel(const std::string& name);		// get channel
		std::vector<std::string>				getChannelNames() const;					// get channel names
		const std::map<std::string, Channel*>	&getChannels() const;						// get channels
		void									removeChannel(const std::string& name);		// remove channel
		void									removeClientFromAllChannels(int clientFd);	// remove client from all channels

};

#endif

// ChannelManager.hpp
#ifndef CHANNELMANAGER_HPP
#define CHANNELMANAGER_HPP

#include "Channel.hpp"
#include <map>
#include <string>

class ChannelManager {
private:
    std::map<std::string, Channel*> channels;

    ChannelManager(const ChannelManager&);
    ChannelManager& operator=(const ChannelManager&);

public:
    ChannelManager();
    ~ChannelManager();

    Channel* createChannel(const std::string& name);
    Channel* getChannel(const std::string& name);
    bool channelExists(const std::string& name);
    void removeChannel(const std::string& name);
    void removeClientFromAllChannels(int clientFd);
    std::vector<std::string> getChannelNames() const;
};

#endif

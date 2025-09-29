// Channel.hpp - zaktualizuj:
#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <set>
#include <vector>
#include "Client.hpp"

class Channel {

private:
    Channel();                                          // default constructor
    Channel(const Channel &copy);                       // copy constructor
    Channel &operator=(const Channel &other);           // copy assignment operator

public:
    std::string                 name;           // channel name
    std::string                 topic;          // channel topic
    std::map<int, Client*>      members;        // map of channel members
    std::set<int>               operators;      // set of channel operators
    std::string                 key;            // channel password/key
    std::set<char>              modes;          // channel modes
    int                         userLimit;      // user limit

    Channel(const std::string &channelName);    // constructor
    ~Channel();                                 // destructor

    void broadcast(const std::string &message, Client *exclude = NULL) const;   // broadcast
    bool isOperator(int clientFd) const;                                        // check if client is operator
    void addMember(Client *client);                                             // add member
    void removeMember(int clientFd);                                            // remove member
    void addOperator(int clientFd);                                             // add operator
    void removeOperator(int clientFd);                                          // remove operator
    bool hasMember(int clientFd) const;                                         // check if client is member
    std::vector<std::string> getMemberNicknames() const;                        // get all member nicknames
    size_t getMemberCount() const;                                              // get member count
    void setTopic(const std::string& newTopic);                                 // set topic
    const std::string& getTopic() const;                                        // get topic
    
    // Mode management
    void setMode(char mode);
    void unsetMode(char mode);
    bool hasMode(char mode) const;
};

#endif

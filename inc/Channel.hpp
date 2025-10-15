// Channel.hpp - zaktualizuj:
#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <set>
#include <vector>
#include "Client.hpp"

#define MAX_LIMIT 10000

class Channel {

	private:
		std::string					name;		  			// channel name
		std::string					topic;		 			// channel topic
		std::map<int, Client*>		members;	   			// map of channel members
		std::set<int>				operators;	 			// set of channel operators
		std::string					key;		   			// channel password/key
		std::set<char>				modes;		 			// channel modes
		int							userLimit;	 			// user limit
		std::set<int>				invitations;			// set of users by fd allowed to enter channel in invite mode

		// orthodox canonical form:
		Channel();											// default constructor
		Channel(const Channel &copy);						// copy constructor
		Channel &operator=(const Channel &other);			// copy assignment operator

	public:
		// orthodox canonical form:
		Channel(const std::string &channelName);									// constructor
		~Channel();																	// destructor

		std::string getName() const;
		void broadcast(const std::string &message, Client *exclude = NULL) const;	// broadcast
		bool isOperator(int clientFd) const;										// check if client is operator
		void addMember(Client *client);												// add member
		void removeMember(int clientFd);											// remove member
		void addOperator(int clientFd);												// add operator
		void removeOperator(int clientFd);											// remove operator
		bool hasMember(int clientFd) const;											// check if client is member
		std::vector<std::string> getMemberNicknames() const;						// get all member nicknames
		size_t getMemberCount() const;												// get member count
		void setTopic(const std::string& newTopic);									// set topic
		const std::string& getTopic() const;										// get topic
		void setKey(const std::string &key);
		std::string getKey() const;
		void setUserLimit(int userLimit);
		size_t getUserLimit() const;
		void addInvitation(int fd);
		void removeInvitation(int fd);
		bool isInvited(int fd) const;

		// Mode management
		void setMode(char mode);
		void unsetMode(char mode);
		bool hasMode(char mode) const;
		std::string getModeString() const;

		const std::map<int, Client *> &getMembers() const;

};

#endif

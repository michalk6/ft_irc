#ifndef Channel_HPP
#define Channel_HPP

#include <string>			// for std::string
#include <map>				// for std::map
#include <set>				// for std::set
#include "Client.hpp"

class Channel {

	private:
		Channel();																			// default constructor
		Channel(const Channel &copy);														// copy constructor
		Channel &operator=(const Channel &other);											// copy assignment operator

	public:
		std::string					name;			// channel name
		std::string					topic;			// channel topic
		std::map<int, Client *> 	members;		// map of channel members
		std::set<int>				operators;		// set of channel operators
		std::map<int, std::string>	modes;			// map of channel modes

		Channel(const std::string &channelName);												// constructor
		~Channel();																			// destructor

		void broadcast(const std::string &message, Client *exclude = NULL) const;				// broadcast
		bool isOperator(int clientFd) const;													// check if client is operator
		void addMember(Client *client);															// add member
		void removeMember(int clientFd);														// remove member

};

#endif

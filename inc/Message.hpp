#ifndef IRCMESSAGE_HPP
#define IRCMESSAGE_HPP

#include <string>
#include <vector>

class IRCMessage {

	private:
		// orthodox canonical form:
		IRCMessage();									// default constructor
		IRCMessage(const IRCMessage &copy);				// copy constructor
		IRCMessage &operator=(const IRCMessage &other);	// copy assignment operator

	public:
	 	// orthodox canonical form:
		IRCMessage(const std::string& raw);				// constructor
		~IRCMessage();									// destructor

		std::string 				prefix;				// client prefix
		std::string					command;			// command
		std::vector<std::string>	parameters;			// parameters

		void parse(const std::string& raw);				// parse raw message
};

#endif

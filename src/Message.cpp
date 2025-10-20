#include "Message.hpp"
#include <sstream>

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// 															PUBLIC:
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

// ====================================================================
// Orthodox Canonical Form elements:
// ====================================================================

// constructor
IRCMessage::IRCMessage(const std::string& raw) {
	parse(raw);
}

// destructor
IRCMessage::~IRCMessage() {}

// ====================================================================
// methods:
// ====================================================================

// parse a raw IRC message string into prefix, command, and parameters according to the IRC protocol format
void IRCMessage::parse(const std::string& raw) {
	std::istringstream iss(raw);
	std::string token;
	
	if (raw[0] == ':') {
		iss.get();
		std::getline(iss, prefix, ' ');
	}
	
	iss >> command;
	
	while (std::getline(iss, token, ' ')) {
		if (token.empty()) continue;
		if (token[0] == ':') {
			std::string lastParam;
			std::getline(iss, lastParam);
			token.erase(0, 1);
			parameters.push_back(token + lastParam);
			break;
		} else {
			parameters.push_back(token);
		}
	}
}

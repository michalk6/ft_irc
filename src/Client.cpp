#include "Client.hpp"

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// 															PUBLIC:
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

// ====================================================================
// Orthodox Canonical Form elements:
// ====================================================================

// constructor
Client::Client(int fd) : _fd(fd), _registered(false) {}

// destructor
Client::~Client() {}

// getters
int Client::getFd() const { return _fd; }
const std::string& Client::getNickname() const { return _nickname; }
const std::string& Client::getUsername() const { return _username; }
bool Client::isRegistered() const { return _registered; }

// setters
void Client::setNickname(const std::string& nick) { _nickname = nick; }
void Client::setUsername(const std::string& user) { _username = user; }
void Client::setRegistered(bool val) { _registered = val; }

// methods
void Client::appendBuffer(const std::string& data) {
	_recvBuffer += data;
}

bool Client::hasCompleteCommand() const {
	return _recvBuffer.find("\r\n") != std::string::npos;
}

std::string Client::extractCommand() {
	size_t pos = _recvBuffer.find("\r\n");
	if (pos == std::string::npos)
		return "";
	std::string cmd = _recvBuffer.substr(0, pos);
	_recvBuffer.erase(0, pos + 2);
	return cmd;
}

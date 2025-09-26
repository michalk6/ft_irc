#include "Client.hpp"
#include <sys/socket.h>
#include <ctime>

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// 															PUBLIC:
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

// ====================================================================
// Orthodox Canonical Form elements:
// ====================================================================

// constructor
Client::Client(int clientFd, const std::string &host) : _fd(clientFd), _hostname(host), _registered(false), _lastActivity(time(NULL)) {}

// destructor
Client::~Client() {}

// ====================================================================
// methods:
// ====================================================================

// getters
int Client::getFd() const { return _fd; }										// get client socket
const std::string& Client::getNickname() const { return _nickname; }			// get nickname
const std::string& Client::getUsername() const { return _username; }			// get username
bool Client::isRegistered() const { return _registered; }					// check if client is registered

// setters
void Client::setNickname(const std::string& nick) { _nickname = nick; }		// set nickname
void Client::setUsername(const std::string& user) { _username = user; }		// set username
void Client::setRegistered(bool val) { _registered = val; }					// set registred flag

// methods
void Client::appendBuffer(const std::string& data) {
	_recvBuffer += data;
}

// check if buffer contains a complete command
bool Client::hasCompleteCommand() const {
	return _recvBuffer.find("\r\n") != std::string::npos;
}

// extract complete command
std::string Client::extractCommand() {
	size_t pos = _recvBuffer.find("\r\n");
	if (pos == std::string::npos)
		return "";
	std::string cmd = _recvBuffer.substr(0, pos);
	_recvBuffer.erase(0, pos + 2);
	return cmd;
}

// get client prefix
std::string Client::getPrefix() const {
	return ":" + _nickname + "!" + _username + "@" + _hostname;
}

// send message
void Client::sendMessage(const std::string &message) const {
	std::string formatted = message + "\r\n";
	send(_fd, formatted.c_str(), formatted.length(), 0);
}

// -------------------------------------------------------registration

// check if password is verified
bool Client::isPasswordVerified() const {
	return _passwordVerified;
}

// set password verification flag
void Client::setPasswordVerified(bool verified) {
	_passwordVerified = verified;
}

// set realname
void Client::setRealname(const std::string& realname) {
	_realname = realname;
}

// get realname
const std::string& Client::getRealname() const {
	return _realname;
}

#include "IRCClient.hpp"
#include <sys/socket.h>
#include <ctime>

IRCClient::IRCClient(int clientFd, const std::string &host) : fd(clientFd), hostname(host), registered(false), lastActivity(time(NULL)) {}

IRCClient::~IRCClient() {}

// getters
int IRCClient::getFd() const { return fd; }
const std::string& IRCClient::getNickname() const { return nickname; }
const std::string& IRCClient::getUsername() const { return username; }
bool IRCClient::isRegistered() const { return registered; }

// setters
void IRCClient::setNickname(const std::string& nick) { nickname = nick; }
void IRCClient::setUsername(const std::string& user) { username = user; }
void IRCClient::setRegistered(bool val) { registered = val; }

// methods
void IRCClient::appendBuffer(const std::string& data) {
	recvBuffer += data;
}

bool IRCClient::hasCompleteCommand() const {
	return recvBuffer.find("\r\n") != std::string::npos;
}

std::string IRCClient::extractCommand() {
	size_t pos = recvBuffer.find("\r\n");
	if (pos == std::string::npos)
		return "";
	std::string cmd = recvBuffer.substr(0, pos);
	recvBuffer.erase(0, pos + 2);
	return cmd;
}

std::string IRCClient::getPrefix() const
{
	return ":" + nickname + "!" + username + "@" + hostname;
}

void IRCClient::sendMessage(const std::string &message) const
{
	std::string formatted = message + "\r\n";
	send(fd, formatted.c_str(), formatted.length(), 0);
}

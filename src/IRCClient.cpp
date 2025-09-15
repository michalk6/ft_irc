#include "IRCClient.hpp"
#include <sys/socket.h>
#include <ctime>

IRCClient::IRCClient(int clientFd, const std::string &host) : fd(clientFd), hostname(host), registered(false), lastActivity(time(NULL)) {}

std::string IRCClient::getPrefix() const
{
	return ":" + nickname + "!" + username + "@" + hostname;
}

void IRCClient::sendMessage(const std::string &message) const
{
	std::string formatted = message + "\r\n";
	send(fd, formatted.c_str(), formatted.length(), 0);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCClient.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlibucha <mlibucha@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 21:39:47 by e                 #+#    #+#             */
/*   Updated: 2025/09/14 20:25:47 by mlibucha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRCCLIENT_HPP
#define IRCCLIENT_HPP

#include <string>
#include <set>
#include <ctime>

class IRCClient
{
public:
	int fd;
	std::string nickname;
	std::string username;
	std::string realname;
	std::string hostname;
	bool registered;
	std::set<std::string> channels;
	time_t lastActivity;

	IRCClient(int clientFd, const std::string &host);
	std::string getPrefix() const;
	void sendMessage(const std::string &message) const;
};

#endif

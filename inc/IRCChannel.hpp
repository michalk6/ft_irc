/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCChannel.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlibucha <mlibucha@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 12:46:09 by mlibucha          #+#    #+#             */
/*   Updated: 2025/09/14 20:25:52 by mlibucha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRCCHANNEL_HPP
#define IRCCHANNEL_HPP

#include <string>
#include <map>
#include <set>
#include "IRCClient.hpp"

class IRCChannel
{
public:
	std::string name;
	std::string topic;
	std::map<int, IRCClient *> members;
	std::set<int> operators;
	std::map<int, std::string> modes;

	IRCChannel(const std::string &channelName);
	void broadcast(const std::string &message, IRCClient *exclude = NULL) const;
	bool isOperator(int clientFd) const;
	void addMember(IRCClient *client);
	void removeMember(int clientFd);
};

#endif

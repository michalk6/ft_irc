/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCMessage.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlibucha <mlibucha@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 12:47:01 by mlibucha          #+#    #+#             */
/*   Updated: 2025/09/14 20:26:54 by mlibucha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRCMESSAGE_HPP
#define IRCMESSAGE_HPP

#include <string>
#include <vector>

class IRCMessage
{
public:
	std::string prefix;
	std::string command;
	std::vector<std::string> parameters;
	
	IRCMessage(const std::string& raw);
	void parse(const std::string& raw);
};

#endif

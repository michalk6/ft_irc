/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCClient.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlibucha <mlibucha@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 21:39:47 by e                 #+#    #+#             */
/*   Updated: 2025/09/16 15:08:14 by mlibucha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRCCLIENT_HPP
#define IRCCLIENT_HPP

#include <string>
#include <set>
#include <ctime>

class IRCClient
{
	private:
		int fd;
		std::string nickname;
		std::string username;
		std::string realname;
		std::string hostname;
		bool		registered;
		std::string recvBuffer;
		std::set<std::string> channels;
		time_t lastActivity;
		IRCClient(const IRCClient &copy);
		IRCClient &operator=(const IRCClient &assign);
	public:
		IRCClient(int fd);
		~IRCClient();
		
		IRCClient(int clientFd, const std::string &host);
		std::string getPrefix() const;
		void sendMessage(const std::string &message) const;
		int			getFd() const;											// get client socket
			const		std::string& getNickname() const;						// get nickname
			const		std::string& getUsername() const;						// get username
			bool		isRegistered() const;									// check if client is registered

			void 		setNickname(const std::string& nick);					// set nickname
			void 		setUsername(const std::string& user);					// set username
			void 		setRegistered(bool val);								// set registered flag

			// buffering commands before we find a complete one (\r\n)
			void		appendBuffer(const std::string& data);					// append data to buffer
			bool		hasCompleteCommand() const;								// check if buffer contains a complete command
			std::string extractCommand();										// extract complete command

};

#endif

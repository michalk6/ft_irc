#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>
#include <ctime>

class Client {

private:
	int						_fd;						// client socket
	std::string				_nickname;					// nickname
	std::string				_username;					// username
	std::string				_realname;					// realname
	std::string				_hostname;					// hostname
	bool					_registered;				// flag to check if client is registered
	bool					_passwordVerified;			// flag to check if password is verified
	std::string				_recvBuffer;				// temporary buffer for recv()
	std::set<std::string>	_channels;					// joined channels
	time_t					_lastActivity;				// last active time

	// orthodox canonical form:
	Client();											// default constructor
	Client(const Client &copy);							// copy constructor
	Client &operator=(const Client &assign);			// copy assignment operator

public:
	// orthodox canonical form:
	Client(int clientFd, const std::string &host);		// constructor
	~Client();											// destructor

	// methods:
	std::string 	getPrefix() const;										// get client prefix
	void			sendMessage(const std::string &message) const;			// send message
	int				getFd() const;											// get client socket
	const			std::string& getNickname() const;						// get nickname
	const			std::string& getUsername() const;						// get username
	bool			isRegistered() const;									// check if client is registered
	void 			setNickname(const std::string& nick);					// set nickname
	void 			setUsername(const std::string& user);					// set username
	void 			setRegistered(bool val);								// set registered flag

	// buffering commands before we find a complete one (\r\n):
	void			appendBuffer(const std::string& data);					// append data to buffer
	bool			hasCompleteCommand() const;								// check if buffer contains a complete command
	std::string 	extractCommand();										// extract complete command

	// registration:
	bool			isPasswordVerified() const;								// check if password is verified
	void			setPasswordVerified(bool verified);						// set password verification flag
	void			setRealname(const std::string& realname);				// set realname
	const			std::string& getRealname() const;						// get realname
	void addChannel(const std::string& channelName);    // add channel to client's channel list
	void removeChannel(const std::string& channelName); // remove channel from client's channel list
	const std::set<std::string>& getChannels() const;  // get client's channels
	bool isInChannel(const std::string &channelName) const;
	const std::string& getHostname() const;
};

#endif

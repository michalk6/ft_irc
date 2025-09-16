#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

class Client {

	private:
		int		 	_fd;													// client socket
		std::string _nickname;												// nickname
		std::string _username;												// username
		std::string _hostname;												// hostname
		bool		_registered;											// flag to check if client is registered
		std::string _recvBuffer;											// temporary buffer for recv()

		Client(const Client &copy);											// copy constructor (we don't use it)
		Client &operator=(const Client &assign);							// copy assignment operator (we don't use it)

	public:
		Client(int fd);														// constructor
		~Client();															// destructor

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

#ifndef BOT_HPP
#define BOT_HPP

#include <string>
#include <set>

#define BANNED_PATH "config/banned_words.txt"
#define CENSOR_STRING "@#$%"

class Bot {
	public:
		Bot();
		~Bot();

		std::string filterMessage(std::string original);

	private:
		std::set<std::string> bannedWords;

		void loadBannedWords();
		void replaceAll(std::string &message, std::string const &toReplace, std::string const &replacement);

		Bot(std::set<std::string>);
		Bot &operator=(Bot const &rhs);
};

#endif

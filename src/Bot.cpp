#include "Bot.hpp"
#include <fstream>
#include <iostream>

Bot::Bot() {
	this->loadBannedWords();
}

Bot::~Bot() {}

std::string Bot::filterMessage(std::string original) {
	std::set<std::string>::iterator it = this->bannedWords.begin();
	std::set<std::string>::iterator ite = this->bannedWords.end();
	while (it != ite) {
		replaceAll(original, *it, CENSOR_STRING);
		++it;
	}
	return original;
}

void Bot::replaceAll(std::string &message, std::string const &toReplace, std::string const &replacement) {
	if (toReplace.empty()) return;
	std::size_t pos = 0;

	while ((pos = message.find(toReplace, pos)) != std::string::npos) {
		message.replace(pos, toReplace.length(), replacement);
		pos += replacement.size();
	}
}

void Bot::loadBannedWords() {
	std::ifstream ifs(BANNED_PATH);
	std::string buf;
	while (getline(ifs, buf))
		this->bannedWords.insert(buf);
}

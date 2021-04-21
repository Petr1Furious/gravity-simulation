#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

typedef long double ld;

struct Chat {
	static constexpr int BORDER = 4;
	static constexpr int FONT_SIZE = 20;
	std::vector<std::pair<ld, sf::String>> messages;
	std::vector<sf::String> sent;
	sf::String curMessage, oldMessage;
	bool typingNickname;
	sf::String curAutoPart;
	sf::String curAuto;
	bool typing;
	int selectedMessagePos;
	int posStartDrawing;
	int messagePos;
	ld lastCursorMoved;

	Chat();
	bool wordPart(sf::Uint32 c);
	sf::Uint32 lower(sf::Uint32 c);
	sf::String getCurrentWord();
	void putChar(sf::Uint32 c, std::vector<sf::String>nicknames = std::vector<sf::String>());
	void deleteSymbol();
	void putMessage(sf::String message);
	void send();
	void setTyping(bool flag);
	bool getTyping();
	sf::String getMessage();
	void goUp();
	void goDown();
	void scroll(int steps);
	void moveCursor(int steps);
	void moveCursorCtrl(int direction);
	void draw(sf::RenderWindow& window, const sf::Font& font);
};

std::vector<sf::String>parseCommand(sf::String command);
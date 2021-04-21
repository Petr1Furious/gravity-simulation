#include "Chat.h"
#include "Functions.h"

Chat::Chat() : typing(false), typingNickname(false) {}
bool Chat::wordPart(sf::Uint32 c) {
	return '0' <= c && c <= '9' || 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || c == '_' || 1040 <= c && c <= 1103 || c == 1105 || c == 1025;
}
sf::Uint32 Chat::lower(sf::Uint32 c) {
	if ('A' <= c && c <= 'Z') {
		return c - 'A' + 'a';
	}
	return c;
}
sf::String Chat::getCurrentWord() {
	sf::String res;
	for (int i = messagePos - 1; i >= -1; i--) {
		if (i == -1 || !wordPart(curMessage[i]) && curMessage[i] != '/')break;
		res += curMessage[i];
	}
	std::reverse(res.begin(), res.end());
	return res;
}
void Chat::putChar(sf::Uint32 c, std::vector<sf::String>nicknames) {
	lastCursorMoved = gettime();
	if (c == 13) {}
	else if (c == 8) {
		typingNickname = false;
		if (messagePos > 0) {
			curMessage.erase(messagePos-- - 1);
		}
	}
	else if (c == 127) {
		typingNickname = false;
		bool everWordPart = false;
		for (int i = messagePos - 1; i >= -1; i--) {
			if (i == -1 || everWordPart && !wordPart(curMessage[i])) {
				curMessage.erase(i + 1, messagePos - (i + 1) + 1);
				messagePos = i + 1;
				break;
			}
			if (wordPart(curMessage[i])) everWordPart = true;
		}
	}
	else if (c == 9) {
		if (!typingNickname) {
			typingNickname = true;
			curAutoPart.clear();
			curAuto.clear();
			curAutoPart = getCurrentWord();
		}
		std::sort(nicknames.begin(), nicknames.end());
		std::vector<sf::String>prefixOf;
		for (auto s : nicknames) {
			if (curAutoPart.getSize() > s.getSize()) continue;
			bool isPrefix = true;
			for (int i = 0; i < curAutoPart.getSize(); i++) {
				if (lower(s[i]) != lower(curAutoPart[i])) {
					isPrefix = false;
					break;
				}
			}
			if (isPrefix && !(curAutoPart.getSize() == 0 && !s.isEmpty() && s[0] == '/')) {
				prefixOf.push_back(s);
			}
		}
		if (!prefixOf.empty()) {
			bool found = false;
			for (int i = 0; i < prefixOf.size(); i++) {
				if (curAuto == prefixOf[i]) {
					curAuto = prefixOf[(i + 1) % (int)prefixOf.size()];
					found = true;
					break;
				}
			}
			if (!found)curAuto = prefixOf[0];
			for (int i = messagePos - 1; i >= -1; i--) {
				if (i == -1 || !wordPart(curMessage[i]) && curMessage[i] != '/') {
					curMessage.erase(i + 1, messagePos - i - 1);
					curMessage.insert(i + 1, curAuto);
					messagePos = i + 1 + curAuto.getSize();
					break;
				}
			}
		}
	}
	else {
		typingNickname = false;
		if (curMessage.getSize() < 150) {
			curMessage.insert(messagePos, c);
			messagePos++;
		}
	}
}
void Chat::deleteSymbol() {
	typingNickname = false;
	if (messagePos < curMessage.getSize()) {
		curMessage.erase(messagePos);
		lastCursorMoved = gettime();
	}
}
void Chat::putMessage(sf::String message) {
	messages.push_back(std::make_pair(gettime(), message));
	if (posStartDrawing == (int)messages.size() - 21) posStartDrawing++;
}
void Chat::send() {
	sent.push_back(curMessage);
}
void Chat::setTyping(bool flag) {
	typing = flag;
	if (!typing) {
		curMessage.clear();
	}
	else {
		typingNickname = false;
		selectedMessagePos = sent.size();
		posStartDrawing = std::max(0, (int)messages.size() - 20);
		messagePos = 0;
		lastCursorMoved = gettime();
	}
}
bool Chat::getTyping() {
	return typing;
}
sf::String Chat::getMessage() {
	sf::String returnMessage = curMessage;
	return returnMessage;
}
void Chat::goUp() {
	typingNickname = false;
	int oldPos = selectedMessagePos;
	if (selectedMessagePos == sent.size())oldMessage = curMessage;
	selectedMessagePos = std::max(0, selectedMessagePos - 1);
	if (selectedMessagePos != sent.size())curMessage = sent[selectedMessagePos];
	else curMessage = oldMessage;
	if (oldPos != selectedMessagePos) messagePos = curMessage.getSize();
	lastCursorMoved = gettime();
}
void Chat::goDown() {
	typingNickname = false;
	int oldPos = selectedMessagePos;
	selectedMessagePos = std::min((int)sent.size(), selectedMessagePos + 1);
	if (selectedMessagePos != sent.size()) curMessage = sent[selectedMessagePos];
	else curMessage = oldMessage;
	if (oldPos != selectedMessagePos) messagePos = curMessage.getSize();
	lastCursorMoved = gettime();
}
void Chat::scroll(int steps) {
	posStartDrawing = std::min(std::max(0, posStartDrawing - steps), std::max(0, (int)messages.size() - 20));
}
void Chat::moveCursor(int steps) {
	typingNickname = false;
	messagePos = std::max(0, std::min((int)curMessage.getSize(), messagePos + steps));
	lastCursorMoved = gettime();
}
void Chat::moveCursorCtrl(int direction) {
	typingNickname = false;
	while (true) {
		moveCursor(direction);
		if (messagePos == 0 || messagePos == curMessage.getSize()) break;
		if (!wordPart(curMessage[messagePos - 1]) && wordPart(curMessage[messagePos])) break;
	}
}
void Chat::draw(sf::RenderWindow& window, const sf::Font& font) {
	int start = (!typing ? (std::max(0, (int)messages.size() - 3)) : posStartDrawing);
	int cnt = (!typing ? cnt = std::min((int)messages.size(), 3) : std::min((int)messages.size() - posStartDrawing, 20));
	for (int i = start; i < start + cnt; i++) {
		int alpha = 191;
		ld dt = gettime() - messages[i].first;
		if (dt >= 6 && !typing) {
			alpha -= 191 * (dt - 6);
		}
		if (alpha <= 0) continue;
		sf::Text temp(messages[i].second, font, 20);
		temp.setPosition(20, window.getSize().y - 1 - (start + cnt - i + 1) * 30);
		temp.setFillColor(sf::Color(255, 255, 255, alpha));
		window.draw(temp);
	}
	if (typing) {
		sf::Text temp(curMessage, font, FONT_SIZE);
		temp.setPosition(20, window.getSize().y - 1 - 30);
		temp.setFillColor(sf::Color(255, 255, 255, 191));
		if ((int)((gettime() - lastCursorMoved) * 10) % 10 < 5) {
			sf::Text part(curMessage.substring(0, messagePos), font, FONT_SIZE);
			part.setPosition(20, window.getSize().y - 1 - 30);
			auto frect = part.getGlobalBounds();
			sf::RectangleShape tempRect;
			tempRect.setPosition(frect.left + frect.width, window.getSize().y - 1 - 26);
			tempRect.setSize(sf::Vector2f(3, 20));
			tempRect.setFillColor(sf::Color(255, 255, 255, 191));
			window.draw(tempRect);
		}
		window.draw(temp);
	}
}


std::vector<sf::String> parseCommand(sf::String command) {
	command += ' ';
	std::vector<sf::String>res;
	int pos = 0;
	for (int i = 0; i < command.getSize(); i++) {
		if (command[i] == ' ') {
			if (i != pos) res.push_back(command.substring(pos, i - pos));
			pos = i + 1;
		}
	}
	return res;
}
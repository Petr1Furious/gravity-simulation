#include "Menu.h"

Menu::Menu() {
	active = true;
}

void Menu::addText(const sf::Text& text) {
	t.push_back(text);
}

void Menu::addButton(const Button& button) {
	b.push_back(button);
}

void Menu::setText(const int& id, const sf::Text& text) {
	t[id] = text;
}

void Menu::setButton(const int& id, const Button& button) {
	b[id] = button;
}

void Menu::setButtonText(const int& id, const sf::String& s) {
	b[id].setText(s);
}

void Menu::setTextString(const int& id, const sf::String& s) {
	t[id].setString(s);
}

void Menu::tick(sf::RenderWindow& window) {
	for (auto& i : b) {
		i.tick(window);
	}
}

std::vector<bool> Menu::getPresses() const {
	std::vector<bool> res(b.size(), false);
	for (int i = 0; i < b.size(); i++) res[i] = b[i].isPressed();
	return res;
}

std::vector<bool> Menu::getHolds() const {
	std::vector<bool> res(b.size(), false);
	for (int i = 0; i < b.size(); i++) res[i] = b[i].isHeld();
	return res;
}

bool Menu::isActive() const {
	return active;
}

void Menu::setActive(const bool& newActive) {
	active = newActive;
	for (auto& i : b) i.setActive(newActive);
}

void Menu::draw(sf::RenderWindow& window, const sf::Font& font) const {
	for (const auto& i : t) window.draw(i);
	for (const auto& i : b) i.draw(window, font);
}
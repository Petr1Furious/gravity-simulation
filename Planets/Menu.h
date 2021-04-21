#pragma once
#include <SFML/Graphics.hpp>
#include "Button.h"

struct Menu {
private:
	std::vector<sf::Text> t;
	std::vector<Button> b;
	bool active;
public:
	Menu();
	void addText(const sf::Text& text);
	void addButton(const Button& button);
	void setText(const int& id, const sf::Text& text);
	void setButton(const int& id, const Button& button);
	void setButtonText(const int& id, const sf::String& s);
	void setTextString(const int& id, const sf::String& s);
	void tick(sf::RenderWindow& window);
	std::vector<bool> getPresses() const;
	std::vector<bool> getHolds() const;
	bool isActive() const;
	void setActive(const bool& nowActive = true);
	void draw(sf::RenderWindow& window, const sf::Font& font) const;
};
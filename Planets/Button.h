#pragma once
#include <SFML/Graphics.hpp>
#include "UIelement.h"

class Button : UIelement {
	sf::String text;
	bool justHeld;
	bool justPressed;
	bool justReleased;
	bool hoveredOver;
	bool lastMouseState;
	int outlineThickness;
	sf::Color outlineColor;
	sf::Color fillColor;
	sf::Color textColor;
	int fontSize;
	bool active;

public:
	Button();
	Button(const int& X, const int& Y, const int& Width, const int& Height);
	Button(const sf::String& Text);
	Button(const int& X, const int& Y, const int& Width, const int& Height, const sf::String& Text);
	void setPosition(const int& X, const int& Y);
	sf::Vector2i getPosition() const;
	void setSize(const int& Width, const int& Height);
	sf::Vector2i getSize() const;
	void tick(sf::RenderWindow& window);
	bool isPressed() const;
	bool isReleased() const;
	bool isHoveredOver() const;
	bool isHeld() const;
	int getOutlineThickness() const;
	void setOutlineThickness(int OutlineThickness);
	sf::Color getOutlineColor() const;
	void setOutlineColor(const sf::Color& OutlineColor);
	sf::Color getFillColor() const;
	void setFillColor(const sf::Color& FillColor);
	sf::Color getTextColor() const;
	void setTextColor(const sf::Color& TextColor);
	int getFontSize() const;
	void setFontSize(int FontSize);
	void setText(const sf::String& Text);
	sf::String getText() const;
	bool isActive() const;
	void setActive(bool nowActive = true);
	void draw(sf::RenderWindow& window, const sf::Font& font) const;
};
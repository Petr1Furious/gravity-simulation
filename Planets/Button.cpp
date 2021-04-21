#include "Button.h"
#include "Functions.h"

Button::Button() {
	x = 0;
	y = 0;
	width = 0;
	height = 0;
	text = sf::String();
	justHeld = false;
	justPressed = false;
	justReleased = false;
	hoveredOver = false;
	lastMouseState = false;
	outlineThickness = 0;
	fillColor = sf::Color::White;
	outlineColor = sf::Color::White;
	textColor = sf::Color::White;
	active = true;
}

Button::Button(const int& X, const int& Y, const int& Width, const int& Height) {
	x = X;
	y = Y;
	width = Width;
	height = Height;
	text = sf::String();
	justHeld = false;
	justPressed = false;
	justReleased = false;
	hoveredOver = false;
	lastMouseState = false;
	outlineThickness = 0;
	fillColor = sf::Color::White;
	outlineColor = sf::Color::White;
	textColor = sf::Color::White;
	active = true;
}

Button::Button(const sf::String& Text) {
	x = 0;
	y = 0;
	width = 0;
	height = 0;
	text = Text;
	justHeld = false;
	justPressed = false;
	justReleased = false;
	hoveredOver = false;
	lastMouseState = false;
	outlineThickness = 0;
	fillColor = sf::Color::White;
	outlineColor = sf::Color::White;
	textColor = sf::Color::White;
	active = true;
}

Button::Button(const int& X, const int& Y, const int& Width, const int& Height, const sf::String& Text) {
	x = X;
	y = Y;
	width = Width;
	height = Height;
	text = Text;
	justHeld = false;
	justPressed = false;
	justReleased = false;
	hoveredOver = false;
	lastMouseState = false;
	outlineThickness = 0;
	fillColor = sf::Color::White;
	outlineColor = sf::Color::White;
	textColor = sf::Color::White;
	active = true;
}

void Button::setPosition(const int& X, const int& Y) {
	x = X;
	y = Y;
}

sf::Vector2i Button::getPosition() const {
	return sf::Vector2i(x, y);
}

void Button::setSize(const int& Width, const int& Height) {
	width = Width;
	height = Height;
}

sf::Vector2i Button::getSize() const {
	return sf::Vector2i(width, height);
}

void Button::tick(sf::RenderWindow& window) {
	auto cur = sf::Mouse::getPosition(window);
	int mousex = cur.x, mousey = cur.y;
	bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);

	hoveredOver = x <= mousex && mousex < x + width && y <= mousey && mousey < y + height && active;
	justPressed = !lastMouseState && hoveredOver && mousePressed;
	if (justPressed) justHeld = true;
	justReleased = justHeld && !mousePressed;
	if (!mousePressed) justHeld = false;

	lastMouseState = mousePressed;
}

bool Button::isPressed() const {
	return justPressed;
}

bool Button::isReleased() const {
	return justReleased;
}

bool Button::isHoveredOver() const {
	return hoveredOver;
}

bool Button::isHeld() const {
	return justHeld;
}

int Button::getOutlineThickness() const {
	return outlineThickness;
}

void Button::setOutlineThickness(int OutlineThickness) {
	outlineThickness = OutlineThickness;
}

sf::Color Button::getOutlineColor() const {
	return outlineColor;
}

void Button::setOutlineColor(const sf::Color& OutlineColor) {
	outlineColor = OutlineColor;
}

sf::Color Button::getFillColor() const {
	return fillColor;
}

void Button::setFillColor(const sf::Color& FillColor) {
	fillColor = FillColor;
}

sf::Color Button::getTextColor() const {
	return textColor;
}

void Button::setTextColor(const sf::Color& TextColor) {
	textColor = TextColor;
}

int Button::getFontSize() const {
	return fontSize;
}

void Button::setFontSize(int FontSize) {
	fontSize = FontSize;
}

void Button::setText(const sf::String& Text) {
	text = Text;
}

sf::String Button::getText() const {
	return text;
}

bool Button::isActive() const {
	return active;
}

void Button::setActive(bool newActive) {
	active = newActive;
}

void Button::draw(sf::RenderWindow& window, const sf::Font& font) const {
	if (active) {
		sf::RectangleShape rs;
		rs.setFillColor(fillColor);
		rs.setOutlineColor(outlineColor);
		rs.setOutlineThickness(-outlineThickness);
		rs.setPosition((sf::Vector2f)sf::Vector2i(x, y));
		rs.setSize((sf::Vector2f)sf::Vector2i(width, height));
		window.draw(rs);

		sf::Text t(text, font, fontSize);
		t.setFillColor(textColor);
		auto bounds = t.getGlobalBounds();
		t.setPosition((int)(x + width / 2 - (bounds.left + bounds.width / 2)), (int)(y + height / 2 - (bounds.top + bounds.height / 2)));
		window.draw(t);
	}
}
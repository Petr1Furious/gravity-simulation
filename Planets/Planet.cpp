#include "Planet.h"
#include "Functions.h"

int curIdCounter = 0;

planet::planet() {}
planet::planet(ld _x, ld _y, ld _vx, ld _vy, ld _r, ld _m) :
	x(_x), y(_y), vx(_vx), vy(_vy), r(_r), m(_m), name(sf::String()) {
	id = curIdCounter++;
}
planet::planet(ld _x, ld _y, ld _vx, ld _vy, ld _r, ld _m, sf::String _name) :
	x(_x), y(_y), vx(_vx), vy(_vy), r(_r), m(_m), name(_name) {
	id = curIdCounter++;
}

void planet::updateCoords(ld dt) {
	x += vx * dt;
	y += vy * dt;
}

void planet::updateVelocity(ld dt, const std::vector<planet>& p) {
	ld ax = 0, ay = 0;
	for (const auto& s : p) {
		if (s == (*this)) continue;
		ld d2 = dist2(x, y, s.x, s.y);
		ld d3 = d2 * sqrt(d2);
		ax += s.m * (s.x - x) / d3;
		ay += s.m * (s.y - y) / d3;
	}
	ax *= planet::G;
	ay *= planet::G;
	vx += ax * dt;
	vy += ay * dt;
}

void planet::draw(sf::RenderWindow& window, ld cx, ld cy, ld scale) {
	int drawR = std::max(1, (int)(r / scale));
	sf::CircleShape c(drawR);
	c.setFillColor(sf::Color(172, 174, 216));
	c.setOrigin(drawR, drawR);
	c.setPosition(getdrawx(x, cx, scale, window.getSize().x), getdrawy(y, cy, scale, window.getSize().y));
	window.draw(c);
}

bool planet::operator==(const planet& s) const {
	return x == s.x && y == s.y && vx == s.vx && vy == s.vy && r == s.r && m == s.m;
}

bool collide(const planet& p1, const planet& p2) {
	return (dist(p1.x, p1.y, p2.x, p2.y) <= (p1.r + p2.r));
}

planet operator+(const planet& p1, const planet& p2) {
	if (p1.m == 0 && p2.m == 0) {
		return planet(p1.x, p1.y, (p1.vx + p2.vx) / 2, (p1.vy + p2.vy) / 2, pow(p1.r * p1.r * p1.r + p2.r * p2.r * p2.r, 1.f / 3.f), 0);
	}
	ld px = p1.vx * p1.m + p2.vx * p2.m;
	ld py = p1.vy * p1.m + p2.vy * p2.m;
	ld m = p1.m + p2.m;
	ld x = (p1.x * p1.m + p2.x * p2.m) / m;
	ld y = (p1.y * p1.m + p2.y * p2.m) / m;
	std::string name;
	if (p1.m > p2.m) {
		name = p1.name;
	}
	else {
		name = p2.name;
	}
	return planet(x, y, px / m, py / m, pow(p1.r * p1.r * p1.r + p2.r * p2.r * p2.r, (ld)1 / (ld)3), m, name);
}
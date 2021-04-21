#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

typedef long double ld;

extern int curIdCounter;
class planet {
public:
	static constexpr ld G = (ld)6.67408e-11;
	static constexpr ld DENSITY = 5510;
	ld x, y;
	ld vx, vy;
	ld r, m;
	int id;
	sf::String name;

	planet();
	planet(ld X, ld Y, ld Vx, ld Vy, ld R, ld M);
	planet(ld X, ld Y, ld Vx, ld Vy, ld R, ld M, sf::String Name);

	void updateCoords(ld dt);
	void updateVelocity(ld dt, const std::vector<planet>& p);
	void draw(sf::RenderWindow& window, ld cx, ld cy, ld scale);
	bool operator==(const planet& s) const;
};

bool collide(const planet& p1, const planet& p2);

planet operator+(const planet& p1, const planet& p2);
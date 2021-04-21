#pragma once

#include <vector>
#include <SFML/Graphics.hpp>

typedef long double ld;

struct tracePoint {
	ld x, y;
	ld t;
	tracePoint();
	tracePoint(ld x, ld y, ld t);
};

struct trace {
	ld m;
	std::vector<tracePoint> v;

	trace();

	void insertPoint(tracePoint t);

	void draw(sf::RenderWindow& window, ld cx, ld cy, ld scale, const trace& sTrace = trace(), ld sx = 0, ld sy = 0);

	bool operator<(const trace& t) const;
};
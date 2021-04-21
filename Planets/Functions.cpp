#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <sstream>
#include <string>

#include "Functions.h"

sf::Clock timer__;
ld gettime() {
	return (ld)timer__.getElapsedTime().asMicroseconds() / (ld)1e6;
}

template <typename T>
std::string to_string(const T& x, int n = 6) {
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << x;
	return out.str();
}

std::string to_string(const ld& x) {
	std::ostringstream out;
	out << x;
	return out.str();
}

ld dist2(ld x1, ld y1, ld x2, ld y2) {
	return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

ld dist(ld x1, ld y1, ld x2, ld y2) {
	return std::sqrt(dist2(x1, y1, x2, y2));
}

ld getdrawx(ld x, ld cx, ld scale, ld wx) {
	return (x - cx) / scale + wx / 2;
}

ld getdrawy(ld y, ld cy, ld scale, ld wy) {
	return (-y + cy) / scale + wy / 2;
}

ld getrealx(ld x, ld cx, ld scale, ld wx) {
	return (x - wx / 2) * scale + cx;
}

ld getrealy(ld y, ld cy, ld scale, ld wy) {
	return -((y - wy / 2) * scale - cy);
}

template <typename T>
float getWidth<T>(const T& t) {
	auto temp = t.getGlobalBounds();
	return temp.width;
}

template <typename T>
float getHeight<T>(const T& t) {
	auto temp = t.getGlobalBounds();
	return temp.height;
}

template float getWidth(const sf::Text&);
template float getHeight(const sf::Text&);

sf::String formatTime(ld time) {
	std::vector<std::pair<ld, std::string>> t = { {1e-9, "ns"}, {1e-6, "µs"}, {1e-3, "ms"}, {1, "s"}, {60, "m"}, {3600, "h"}, {86400, "d"}, {31558149.984, "y"}, {31558149.984e3, "Ky"}, {31558149.984e6, "My"}, {31558149.984e9, "Gy"}, {31558149.984e12, "Ty"} };
	int j = std::max(0, (int)(std::upper_bound(t.begin(), t.end(), std::make_pair(time, std::string(0, ' '))) - t.begin()) - 1);
	return to_string(time / t[j].first, 3) + " " + t[j].second;
}

sf::String formatDist(ld dist) {
	std::vector<std::pair<ld, std::string>> t = { {1e-9, "nm"}, {1e-6, "µm"}, {1e-3, "mm"}, {1, "m"}, {1e3, "km"}, {1e6, "Mm"}, {1e9, "Gm"}, {1e12, "Tm"}, {25902068371240.l, "ld"}, {9460730472580800.l, "ly"} };
	int j = std::max(0, (int)(std::upper_bound(t.begin(), t.end(), std::make_pair(dist, std::string(0, ' '))) - t.begin()) - 1);
	return to_string(dist / t[j].first, 3) + " " + t[j].second;
}

ld getVolume(const ld& r) {
	return (ld)4 / 3 * M_PI * r * r * r;
}

sf::String getName(const std::vector<sf::String>& lines) {
	int mx = 0;
	for (auto s : lines) mx = std::max(mx, (int)s.getSize());
	sf::String res;
	for (auto s : lines) {
		res += std::string((mx - (int)s.getSize()), ' ') + s + "\n";
	}
	return res;
}

void drawLineSegment(sf::RenderWindow& window, ld x1, ld y1, ld x2, ld y2, const sf::Color& color) {
	sf::Vertex vertices[2];
	vertices[0] = sf::Vertex(sf::Vector2f(x1, y1), color);
	vertices[1] = sf::Vertex(sf::Vector2f(x2, y2), color);
	window.draw(vertices, 2, sf::Lines);
}

void drawArrow(sf::RenderWindow& window, ld x1, ld y1, ld x2, ld y2, sf::Color color) {
	drawLineSegment(window, x1, y1, x2, y2, color);
	ld d = dist(x1, y1, x2, y2);
	if (d > 4) {
		sf::CircleShape temp(4, 3);
		temp.setOrigin(4, 4);
		temp.setRotation(std::atan2(y2 - y1, x2 - x1) / M_PI * 180 + 90);
		temp.setPosition(x2, y2);
		temp.setFillColor(color);
		window.draw(temp);
	}
}


void DSU::clear() {
	p.clear();
	p.shrink_to_fit();
}
void DSU::extend(int n) {
	n++;
	if (p.size() < n) {
		int from = p.size();
		p.resize(n);
		for (int i = from; i < n; i++) {
			p[i] = i;
		}
	}
}
void DSU::resize(int n) {
	extend(n);
	p.resize(n);
}
int DSU::get(int v) {
	extend(v);
	return p[v] == v ? v : p[v] = get(p[v]);
}
void DSU::unite(int v, int u) {
	extend(v);
	extend(u);
	p[get(u)] = get(v);
}
void DSU::update(std::vector<std::pair<int, int>> v) {
	for (auto i : v) {
		unite(i.first, i.second);
	}
}


TPS::TPS(ld timeDifference) : timeDifference(timeDifference) {}
void TPS::clear() {
	mtx.lock();
	while (!updateTimes.empty()) updateTimes.pop();
	mtx.unlock();
}
void TPS::addUpdateTime() {
	mtx.lock();
	updateTimes.push(gettime());
	while (updateTimes.back() - updateTimes.front() >= timeDifference) updateTimes.pop();
	mtx.unlock();
}
ld TPS::getUpdatesPerSecond() {
	mtx.lock();
	if (updateTimes.size() <= 1) {
		mtx.unlock();
		return 0;
	}
	ld res = ((ld)updateTimes.size() - 1) / (updateTimes.back() - updateTimes.front());
	mtx.unlock();
	return res;
}
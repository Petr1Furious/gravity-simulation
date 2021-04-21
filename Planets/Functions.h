#pragma once

#include <SFML/Graphics.hpp>
#include <mutex>
#include <queue>

typedef long double ld;

extern sf::Clock timer__;
ld gettime();

template <typename T>
std::string to_string(const T& x, int n);
std::string to_string(const ld& x);
ld dist2(ld x1, ld y1, ld x2, ld y2);
ld dist(ld x1, ld y1, ld x2, ld y2);
ld getdrawx(ld x, ld cx, ld scale, ld wx);
ld getdrawy(ld y, ld cy, ld scale, ld wy);
ld getrealx(ld x, ld cx, ld scale, ld wx);
ld getrealy(ld y, ld cy, ld scale, ld wy);
template <typename T>
float getWidth(const T& t);
template <typename T>
float getHeight(const T& t);
sf::String formatTime(ld time);
sf::String formatDist(ld time);
ld getVolume(const ld& r);
sf::String getName(const std::vector<sf::String>& lines);
void drawLineSegment(sf::RenderWindow& window, ld x1, ld y1, ld x2, ld y2, const sf::Color& color);
void drawArrow(sf::RenderWindow& window, ld x1, ld y1, ld x2, ld y2, sf::Color color);

struct DSU {
	std::vector<int> p;
	void clear();
	void extend(int n);
	void resize(int n);
	int get(int v);
	void unite(int v, int u);
	void update(std::vector<std::pair<int, int>> v);
};
struct TPS {
	ld timeDifference;
	std::mutex mtx;
	TPS(ld timeDifference);
	std::queue<ld> updateTimes;
	void clear();
	void addUpdateTime();
	ld getUpdatesPerSecond();
};
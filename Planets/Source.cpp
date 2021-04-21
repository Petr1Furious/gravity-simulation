#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <Windows.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <atomic>
#include <thread>
#include <random>
#include <chrono>
#include <deque>
#include <cmath>
#include <mutex>
#include <queue>
#include <map>
#include <set>

#include "Button.h"
#include "Functions.h"
#include "Psystem.h"
#include "Chat.h"
#include "Traces.h"
#include "Menu.h"
typedef long double ld;
using namespace sf;

//std::mt19937_64 rnd(std::chrono::high_resolution_clock::now().time_since_epoch().count());
std::mt19937_64 rnd(0);
long long randnum(long long l, long long r) {
	return rnd() % (r - l + 1) + l;
}

struct gamestate {
private:
	psystem s;
	bool opened;
	std::mutex mtx;
	std::mutex mtx2;
	std::vector<std::pair<planet, int>> toChange;
	std::vector<int> toDelete;
	bool changeAll;
	psystem changeTo;
	TPS t;
public:
	ld cx, cy, scale;
	ld timeSpeed;
	bool realtime;
	bool paused;
	bool fullScreenMode;
	DSU d;
	int selectedId;
	ld createDensity;
	bool negativeMass;
	bool drawLines;
	bool showNames;
	ld timeToPredict;
	bool relativeTraces;
	bool didUpdate;
	std::mutex timeMtx;
	ld timer;

	gamestate() : s(), timeSpeed(1), opened(true), changeAll(false), realtime(true), paused(false), fullScreenMode(false), selectedId(-1), t(1),
		createDensity(planet::DENSITY), negativeMass(false), drawLines(false), showNames(false), timeToPredict(5), relativeTraces(true), didUpdate(false), timer(0) {}

	void clear() {
		toChange.clear();
		toDelete.clear();
		t.clear();
	}
	void addUpdateTime() {
		t.addUpdateTime();
	}
	ld getUpdatesPerSecond() {
		return t.getUpdatesPerSecond();
	}
	bool isOpen()const {
		return opened;
	}
	void close() {
		opened = false;
	}
	void open() {
		opened = true;
	}
	void lock() {
		mtx.lock();
	}
	void unlock() {
		mtx.unlock();
	}
	psystem getPlanets()const {
		return s;
	}
	void setPlanets(const psystem& news) {
		mtx2.lock();
		changeAll = true;
		changeTo = news;
		mtx2.unlock();
	}
	void setPlanetsLocked(const psystem& newS) {
		s = newS;
	}
	void insertPlanet(const planet& p, int relativeTo = -1) {
		mtx2.lock();
		toChange.push_back(std::make_pair(p, relativeTo));
		mtx2.unlock();
	}
	void deletePlanet(int id) {
		mtx2.lock();
		toDelete.push_back(id);
		mtx2.unlock();
	}
	void updateActions() {
		mtx2.lock();
		if (changeAll) {
			s = changeTo;
		}
		else if (!toChange.empty() || !toDelete.empty()) {
			for (auto& i : toDelete) i = d.get(i);
			std::set<int> del, noAdd;
			for (auto i : toChange) del.insert(i.first.id);
			for (auto i : toDelete) del.insert(i), noAdd.insert(i);
			int it = (int)s.s.size() - 1;
			for (int i = (int)s.s.size() - 1; i >= 0; i--) {
				if (del.find(s.s[i].id) != del.end()) {
					std::swap(s.s[i], s.s[it--]);
				}
			}
			while ((int)s.s.size() - 1 > it) {
				s.s.pop_back();
			}
			for (auto i : toChange) {
				if (noAdd.find(i.first.id) == noAdd.end()) {
					if (i.second != -1) {
						int curId = d.get(i.second);
						for (auto j : s.getPlanets()) {
							if (j.id == curId) {
								i.first.x += j.x;
								i.first.y += j.y;
								i.first.vx += j.vx;
								i.first.vy += j.vy;
								break;
							}
						}
					}
					s.insertPlanet(i.first);
				}
			}
		}
		toChange.clear();
		toDelete.clear();
		changeAll = false;
		changeTo.clear();
		mtx2.unlock();
	}
	void updateRecentlyCollided(std::vector<std::pair<int, int>> v) {
		for (auto i : v) {
			d.unite(i.first, i.second);
		}
	}
};

void calculations(gamestate* p) {
	ld lastUpdated = gettime();
	p->lock();
	psystem last = p->getPlanets();
	p->unlock();
	while (p->isOpen()) {
		ld dt = gettime() - lastUpdated;
		lastUpdated = gettime();
		p->lock();
		last = p->getPlanets();
		p->unlock();
		ld curtime;
		p->timeMtx.lock();
		if (p->realtime) {
			curtime = dt * p->timeSpeed;
		}
		else {
			curtime = p->timeSpeed;
		}
		bool curPaused = p->paused;
		if (!curPaused) {
			p->timer += curtime;
		}
		p->timeMtx.unlock();
		std::vector<std::pair<int, int>> temp;
		if (!curPaused) {
			temp = last.updateCollided();
			last.updateVelocity(curtime);
			last.updateCoords(curtime);
		}
		else Sleep(1);
		p->lock();
		p->setPlanetsLocked(last);
		p->updateActions();
		p->addUpdateTime();
		p->updateRecentlyCollided(temp);
		p->didUpdate = true;
		p->unlock();
	}
}

template<class T>
void writeToFile(std::ofstream& fileOut, const T& a) {
	char* temp = (char*)&a;
	fileOut.write(temp, sizeof(a));
}
void saveToFile(String filename, gamestate* p) {
	std::ofstream fileOut((std::string)filename, std::ios::binary);
	writeToFile(fileOut, curIdCounter);
	writeToFile(fileOut, p->scale);
	writeToFile(fileOut, p->cx);
	writeToFile(fileOut, p->cy);
	writeToFile(fileOut, p->realtime);
	writeToFile(fileOut, p->timeSpeed);
	writeToFile(fileOut, p->selectedId);
	writeToFile(fileOut, p->createDensity);
	writeToFile(fileOut, p->negativeMass);
	writeToFile(fileOut, p->drawLines);
	writeToFile(fileOut, p->showNames);
	writeToFile(fileOut, p->timeToPredict);
	writeToFile(fileOut, p->relativeTraces);
	std::vector<planet> planets = p->getPlanets().getPlanets();
	int sz = planets.size();
	writeToFile(fileOut, sz);
	for (auto i : planets) {
		int szName = i.name.getSize();
		writeToFile(fileOut, szName);
		for (auto j : i.name) writeToFile(fileOut, j);
		writeToFile(fileOut, i.id);
		writeToFile(fileOut, i.m);
		writeToFile(fileOut, i.r);
		writeToFile(fileOut, i.vx);
		writeToFile(fileOut, i.vy);
		writeToFile(fileOut, i.x);
		writeToFile(fileOut, i.y);
	}
	fileOut.close();
}
template<class T>
void readFromFile(std::ifstream& fileIn, T& a) {
	char* temp = new char[sizeof(a)];
	fileIn.read(temp, sizeof(a));
	memcpy(&a, temp, sizeof(a));
}
void loadFromFile(String filename, gamestate* p) {
	p->d.clear();
	std::ifstream fileIn((std::string)filename, std::ios::binary);
	if (fileIn.is_open()) {
		readFromFile(fileIn, curIdCounter);
		readFromFile(fileIn, p->scale);
		readFromFile(fileIn, p->cx);
		readFromFile(fileIn, p->cy);
		readFromFile(fileIn, p->realtime);
		readFromFile(fileIn, p->timeSpeed);
		readFromFile(fileIn, p->selectedId);
		readFromFile(fileIn, p->createDensity);
		readFromFile(fileIn, p->negativeMass);
		readFromFile(fileIn, p->drawLines);
		readFromFile(fileIn, p->showNames);
		readFromFile(fileIn, p->timeToPredict);
		readFromFile(fileIn, p->relativeTraces);
		int sz;
		readFromFile(fileIn, sz);
		std::vector<planet> planets(sz);
		for (auto& i : planets) {
			int szName;
			readFromFile(fileIn, szName);
			i.name.clear();
			for (int j = 0; j < szName; j++) {
				Uint32 c;
				readFromFile(fileIn, c);
				i.name += c;
			}
			readFromFile(fileIn, i.id);
			readFromFile(fileIn, i.m);
			readFromFile(fileIn, i.r);
			readFromFile(fileIn, i.vx);
			readFromFile(fileIn, i.vy);
			readFromFile(fileIn, i.x);
			readFromFile(fileIn, i.y);
			if (fileIn.fail()) {
				std::cout << "WRONG!!!\n";
				throw(0);
			}
		}
		p->setPlanets(psystem(planets));
		fileIn.close();
	}
}

Text drawSelected(RenderWindow& window, gamestate* p, const std::vector<planet> planets, planet toDraw, planet selected, ld curTimeSpeed, const Font& font,
	bool drawPrediction = false, bool drawBox = false, bool drawDistance = false) {
	if (toDraw.id == selected.id) {
		selected = planet(0, 0, 0, 0, 0, 0);
	}
	if (drawPrediction) {
		auto prediction = getPredictions(planets, toDraw, 250, curTimeSpeed, -1, p->timeToPredict);
		ld oldx = toDraw.x, oldy = toDraw.y;
		for (auto i : prediction) {
			ld curx = i.first, cury = i.second;
			drawLineSegment(window, getdrawx(oldx, p->cx, p->scale, window.getSize().x), getdrawy(oldy, p->cy, p->scale, window.getSize().y),
				getdrawx(curx, p->cx, p->scale, window.getSize().x), getdrawy(cury, p->cy, p->scale, window.getSize().y), Color(0, 255, 0, 63));
			oldx = curx, oldy = cury;
		}
	}

	drawArrow(window, getdrawx(toDraw.x, p->cx, p->scale, window.getSize().x), getdrawy(toDraw.y, p->cy, p->scale, window.getSize().y),
		getdrawx(toDraw.x + (toDraw.vx - selected.vx) * curTimeSpeed, p->cx, p->scale, window.getSize().x), getdrawy(toDraw.y + (toDraw.vy - selected.vy) * curTimeSpeed, p->cy, p->scale, window.getSize().y),
		Color(255, 0, 0, 127));
	ld drawx = getdrawx(toDraw.x, p->cx, p->scale, window.getSize().x);
	ld drawy = getdrawy(toDraw.y, p->cy, p->scale, window.getSize().y);

	if (drawBox) {
		ld delta = toDraw.r / p->scale + 5;
		drawLineSegment(window, drawx - delta, drawy - delta, drawx - delta, drawy + delta, Color(255, 255, 255, 255));
		drawLineSegment(window, drawx - delta, drawy - delta, drawx + delta, drawy - delta, Color(255, 255, 255, 255));
		drawLineSegment(window, drawx - delta, drawy + delta, drawx + delta, drawy + delta, Color(255, 255, 255, 255));
		drawLineSegment(window, drawx + delta, drawy - delta, drawx + delta, drawy + delta, Color(255, 255, 255, 255));
	}

	std::vector<String> lines;
	if (toDraw.name.getSize()) lines.push_back(toDraw.name);
	lines.push_back("m=" + to_string(toDraw.m) + " kg");
	lines.push_back("r=" + formatDist(toDraw.r));
	lines.push_back("v=" + formatDist(dist(toDraw.vx, toDraw.vy, selected.vx, selected.vy)) + "/s");
	if (drawDistance) {
		lines.push_back("d=" + formatDist(dist(toDraw.x, toDraw.y, selected.x, selected.y)));
	}
	Text text(getName(lines), font, 10);
	text.setOrigin(getWidth(text) / 2, 0);
	text.setFillColor(Color(255, 255, 255, 255));
	text.setOutlineColor(Color(255, 255, 255, 255));
	text.setPosition((int)drawx, (int)(drawy + toDraw.r / p->scale + 7));
	return text;
}

bool textPriorityComp(const std::pair<std::pair<int, ld>, Text>& a, const std::pair<std::pair<int, ld>, Text>& b) {
	return a.first > b.first;
}

void runSimulation(RenderWindow& window, gamestate* p) {
	Thread calcThread(calculations, p);
	window.setVerticalSyncEnabled(true);

	p->cx = 0;
	p->cy = 0;
	p->scale = 1e9;
	p->insertPlanet(planet(0, 0, 0, 0, 695e6, 1.989e30, "Sun"));
	p->insertPlanet(planet(57.909e9, 0, 0, -47.87e3, 2.4e6, 3.302e23, "Mercury"));
	p->insertPlanet(planet(108.21e9, 0, 0, -35.02e3, 6.1e6, 4.869e24, "Venus"));
	p->insertPlanet(planet(152.10e9, 0, 0, -29.29e3, 6.371e6, 5.9724e24, "Earth"));
	p->insertPlanet(planet(152.10e9 + 363228e3, 0, 0, -29.29e3 - 1.082e3, 1737.4e3, 7.342e22, "Moon"));
	p->insertPlanet(planet(227.94e9, 0, 0, -24.13e3, 3.4e6, 6.419e23, "Mars"));
	p->insertPlanet(planet(778.41e9, 0, 0, -13.07e3, 71e6, 1.899e27, "Jupiter"));
	p->insertPlanet(planet(1429.4e9, 0, 0, -9.67e3, 60e6, 5.685e26, "Saturn"));
	p->insertPlanet(planet(2871.0e9, 0, 0, -6.84e3, 26e6, 8.685e25, "Uranus"));
	p->insertPlanet(planet(4444.449e9, 0, 0, -5.5e3, 25e6, 1.024e26, "Neptune"));
	p->timeSpeed = 10 * 86400;
	p->showNames = true;
	p->drawLines = true;

	/*p->cx = 0;
	p->cy = 0;
	p->scale = 1;
	p->insertPlanet(planet(0, 0, 100, 0, 10, getVolume(10) * planet::DENSITY, "1"));
	p->timeSpeed = 1;
	p->showNames = true;*/

	p->updateActions();
	psystem start__ = p->getPlanets();
	start__.updateCollided();
	p->setPlanets(start__);
	p->paused = true;
	calcThread.launch();
	bool fullScreenPressed = false;
	Font font;
	font.loadFromFile("Resources\\font.ttf");

	bool locked = false;
	int lockeddrawx, lockeddrawy;
	ld lockedrealx, lockedrealy;
	bool choosingSelected;

	int oldSelectedId = -1;

	int creationStep = -1;
	ld creationx, creationy, creationr, creationvx, creationvy;

	ld lastButtonUpdated = gettime();
	/*Uint8* lightPlanetsPixels = new Uint8[VideoMode::getFullscreenModes()[0].width * VideoMode::getFullscreenModes()[0].height * 4];
	for (int i = 0; i < VideoMode::getFullscreenModes()[0].width * VideoMode::getFullscreenModes()[0].height * 4; i++) lightPlanetsPixels[i] = 0;*/

	ld lastSelectedx, lastSelectedy;

	Chat chat;

	bool enterPressed = false;

	TPS fps(1);

	Text updatesPerSecondText;

	Text dataText;
	dataText.setCharacterSize(10);
	dataText.setFont(font);
	dataText.setPosition(window.getSize().x - 1, 0);
	dataText.setFillColor(Color::White);
	dataText.setOrigin(170 - 1, 0);

	std::map<int, trace> traces;

	bool oldPaused = true;

	bool wasPaused = false;
	ld notPausedUpdatesPerSecond = 1;

	std::vector<Keyboard::Key> keysToHold;
	{
		keysToHold.push_back(Keyboard::Comma);
		keysToHold.push_back(Keyboard::Period);
		keysToHold.push_back(Keyboard::Subtract);
		keysToHold.push_back(Keyboard::Add);
		keysToHold.push_back(Keyboard::LBracket);
		keysToHold.push_back(Keyboard::RBracket);
	}

	std::vector<Keyboard::Key> keysToPress;
	{
		keysToPress.push_back(Keyboard::L);
		keysToPress.push_back(Keyboard::R);
		keysToPress.push_back(Keyboard::N);
		keysToPress.push_back(Keyboard::M);
		keysToPress.push_back(Keyboard::Z);
		keysToPress.push_back(Keyboard::Q);
		keysToPress.push_back(Keyboard::J);
	}

	Menu menu;
	menu.setActive(true);

	{
		Text text;
		Button button;

		std::vector<String> s;
		s.push_back((String)"Draw traces (L)");
		s.push_back((String)"Relative traces (R)");
		s.push_back((String)"Draw names (N)");
		s.push_back((String)"Negative mass (M)");
		s.push_back((String)"Time mode (Z)");
		s.push_back((String)"Reset velocity (Q)");
		s.push_back((String)"Reset timer (J)");

		int start = 50;
		for (int i = 0; i < s.size(); i++) {
			text.setPosition(50, start);
			text.setFont(font);
			text.setString(s[i]);
			text.setCharacterSize(12);
			menu.addText(text);

			button.setOutlineThickness(1);
			button.setOutlineColor(Color::White);
			button.setFillColor(Color::Black);
			button.setPosition(200, start);
			button.setFontSize(12);
			button.setSize(100, 20);
			button.setText("");
			menu.addButton(button);

			start += 40;
		}

		s.clear();
		s.push_back("");
		s.push_back("");
		s.push_back("");

		start = 50;
		for (int i = 0; i < s.size(); i++) {
			text.setPosition(350, start);
			text.setFont(font);
			text.setString(s[i]);
			text.setCharacterSize(12);
			menu.addText(text);

			button.setOutlineThickness(1);
			button.setOutlineColor(Color::White);
			button.setFillColor(Color::Black);
			button.setPosition(500, start);
			button.setFontSize(12);
			button.setSize(100, 20);
			button.setText("-");
			menu.addButton(button);

			button.setPosition(610, start);
			button.setText("+");
			menu.addButton(button);

			start += 40;
		}
	}

	while (window.isOpen()) {
		if (!p->paused) notPausedUpdatesPerSecond = p->getUpdatesPerSecond();
		p->timeMtx.lock();
		ld curTimeSpeed = p->timeSpeed;
		if (!p->realtime) {
			curTimeSpeed *= notPausedUpdatesPerSecond;
		}
		ld curTimer = p->timer;
		p->timeMtx.unlock();

		if (Keyboard::isKeyPressed(Keyboard::Enter) && chat.getTyping()) {
			if (!enterPressed && chat.getTyping()) {
				String curS = chat.getMessage();
				if (!curS.isEmpty()) {
					if (curS[0] == '/') {
						bool executed = false;
						String message;
						std::vector<String> parsed = parseCommand(curS);
						if (parsed[0] == (String)"/save") {
							if (parsed.size() == 2) {
								String filename = parsed[1];
								bool dotFound = false;
								for (auto i : filename) {
									if (i == '.') {
										dotFound = true;
										break;
									}
								}
								filename += String(".planets");
								saveToFile(filename, p);
							}
							else {
								message = "Wrong argument quantity";
							}
						}
						if (parsed[0] == (String)"/load") {
							if (parsed.size() == 2) {
								String filename = parsed[1];
								bool dotFound = false;
								for (auto i : filename) {
									if (i == '.') {
										dotFound = true;
										break;
									}
								}
								filename += String(".planets");
								ld tempTimeSpeed;
								p->timeMtx.lock();
								p->lock();
								loadFromFile(filename, p);
								p->didUpdate = false;
								p->d.clear();
								p->unlock();
								p->timeMtx.unlock();
								traces.clear();
								wasPaused = false;
								menu.setActive(true);
								oldPaused = true;
								p->paused = true;
								while (!p->didUpdate) {
									Sleep(1);
								}
							}
							else {
								message = "Wrong argument quantity";
							}
						}
						if (parsed[0] == (String)"/random") {
							p->cx = 0;
							p->cy = 0;
							p->scale = 1e3;
							p->showNames = false;
							p->drawLines = false;
							ld maxSpeed = 10;

							std::vector<planet> randomPlanets;

							for (int i = 0; i < 1000; i++) {
								ld angle = randnum(0, 1e6) / 1e6 * 2 * M_PI;
								ld len = randnum(0, p->scale * 1000);
								ld speedAngle = randnum(0, 1e6) / 1e6 * 2 * M_PI;
								ld speed = randnum(0, maxSpeed * 1000) / (ld)1000;
								ld r = randnum(1000, 2000);
								randomPlanets.push_back(planet(len * cos(angle), len * sin(angle), speed * cos(speedAngle), speed * sin(speedAngle), r, getVolume(r) * planet::DENSITY));
							}
							for (int i = 0; i < 1000; i++) {
								ld angle = randnum(0, 1e6) / 1e6 * 2 * M_PI;
								ld len = randnum(0, p->scale * 1000);
								ld speedAngle = randnum(0, 1e6) / 1e6 * 2 * M_PI;
								ld speed = randnum(0, maxSpeed * 1000) / (ld)1000;
								randomPlanets.push_back(planet(len * cos(angle), len * sin(angle), speed * cos(speedAngle), speed * sin(speedAngle), 0, 0));
							}

							p->lock();
							p->setPlanets(randomPlanets);
							p->didUpdate = false;
							traces.clear();
							p->timeMtx.lock();
							p->timeSpeed = 3600;
							p->realtime = true;
							wasPaused = false;
							menu.setActive(true);
							oldPaused = true;
							p->paused = true;
							p->d.clear();
							p->timeMtx.unlock();
							p->unlock();
							while (!p->didUpdate) {
								Sleep(1);
							}
						}
						if (parsed[0] == (String)"/clear") {
							p->lock();
							p->setPlanets(psystem());
							traces.clear();
							p->timeMtx.lock();
							p->timeSpeed = 1;
							p->realtime = true;
							oldPaused = true;
							p->paused = true;
							wasPaused = false;
							menu.setActive(true);
							p->d.clear();
							p->timeMtx.unlock();
							p->cx = 0;
							p->cy = 0;
							p->scale = 1;
							p->unlock();
						}
					}
					else {
						chat.putMessage(curS);
					}
				}
				chat.send();
				chat.setTyping(false);
			}
			enterPressed = true;
		}
		else enterPressed = false;

		window.clear();
		p->lock();
		psystem psys = p->getPlanets();
		ld timePlanetsTaken = gettime();
		DSU d = p->d;
		p->unlock();
		std::vector<planet> planets = psys.getPlanets();

		if (p->selectedId != -1) p->selectedId = d.get(p->selectedId);
		if (p->selectedId != -1) {
			bool found = false;
			for (auto i : planets) {
				if (i.id == p->selectedId) {
					found = true;
				}
			}
			if (!found) {
				p->selectedId = -1;
			}
		}

		planet selected;

		bool foundOldSelected = false;
		for (auto i : planets) {
			if (i.id == oldSelectedId) {
				foundOldSelected = true;
				ld dx = i.x - lastSelectedx;
				ld dy = i.y - lastSelectedy;
				p->cx += dx;
				p->cy += dy;
				lockedrealx += dx;
				lockedrealy += dy;
				if (creationStep != -1) {
					creationx += dx;
					creationy += dy;
				}
			}
		}
		if (!foundOldSelected && oldSelectedId != -1 && p->d.get(oldSelectedId) == p->selectedId) {
			for (auto i : planets) {
				if (i.id == p->selectedId) {
					ld dx = i.x - lastSelectedx;
					ld dy = i.y - lastSelectedy;
					p->cx += dx;
					p->cy += dy;
					lockedrealx += dx;
					lockedrealy += dy;
					if (creationStep != -1) {
						creationx += dx;
						creationy += dy;
					}
				}
			}
		}
		for (auto i : planets) {
			if (i.id == p->selectedId) {
				selected = i;
				lastSelectedx = i.x;
				lastSelectedy = i.y;
			}
		}
		oldSelectedId = p->selectedId;

		if (window.hasFocus()) {
			if (locked) {
				ld mousex = getrealx(Mouse::getPosition(window).x, p->cx, p->scale, window.getSize().x),
					mousey = getrealy(Mouse::getPosition(window).y, p->cy, p->scale, window.getSize().y);
				p->cx -= mousex - lockedrealx;
				p->cy -= mousey - lockedrealy;
			}
		}

		std::vector<int> tracesToDelete;
		for (auto i : traces) {
			if (d.get(i.first) != i.first) {
				traces[d.get(i.first)] = std::max(traces[d.get(i.first)], i.second);
				tracesToDelete.push_back(i.first);
			}
		}
		for (auto i : tracesToDelete) traces.erase(i);

		// std::vector<std::pair<int, int>> lightPlanetsCoords;
		int lightCount = 0, heavyCount = 0;
		if (p->drawLines) {
			ld curtime = gettime();
			for (auto i : planets) {
				if (!p->paused || p->paused && !oldPaused) {
					traces[i.id].insertPoint(tracePoint(i.x, i.y, curtime));
				}
				traces[i.id].m = i.m;
			}
		}
		oldPaused = p->paused;
		trace sTrace;
		ld sx = 0, sy = 0;
		if (p->drawLines && p->selectedId != -1 && p->relativeTraces) {
			sx = selected.x;
			sy = selected.y;
			sTrace = traces[selected.id];
		}

		std::vector<std::pair<std::pair<int, ld>, Text>> planetTextToDraw;

		int toDrawId = -1;
		if (p->selectedId != -1) {
			planet toDraw;
			toDraw.id = -1;
			ld mx;
			ld mousex = getrealx(Mouse::getPosition(window).x, p->cx, p->scale, window.getSize().x),
				mousey = getrealy(Mouse::getPosition(window).y, p->cy, p->scale, window.getSize().y);
			for (auto i : planets) {
				if (i.id == selected.id || dist(mousex, mousey, i.x, i.y) > 50 * p->scale) continue;
				if (toDraw.id == -1 || i.m > mx) {
					mx = i.m;
					toDraw = i;
				}
			}
			if (toDraw.id != -1) {
				planetTextToDraw.push_back(std::make_pair(std::make_pair(1, toDraw.m), drawSelected(window, p, planets, toDraw, selected, curTimeSpeed, font, false, false, true)));
				toDrawId = toDraw.id;
			}

			planetTextToDraw.push_back(std::make_pair(std::make_pair(2, selected.m), drawSelected(window, p, planets, selected, selected, curTimeSpeed, font, true, true, false)));
		}

		if (p->drawLines) {
			for (auto i : planets) {
				traces[i.id].draw(window, p->cx, p->cy, p->scale, sTrace, sx, sy);
			}
		}
		for (auto i : planets) {
			if (i.m != 0) {
				heavyCount++;
				i.draw(window, p->cx, p->cy, p->scale);
			}
			else {
				lightCount++;
				int drawx = getdrawx(i.x, p->cx, p->scale, window.getSize().x), drawy = getdrawy(i.y, p->cy, p->scale, window.getSize().y);
				if (0 <= drawx && drawx < window.getSize().x && 0 <= drawy && drawy < window.getSize().y) {
					/*lightPlanetsCoords.push_back(std::make_pair(drawx, drawy));
					Uint32 color = Color(172, 174, 216).toInteger();
					for (int i = 0; i < 4; i++) {
						lightPlanetsPixels[((drawx + drawy * window.getSize().x) << 2) + i] = ((color >> (24 - 8 * i)) & 255);
					}*/
					CircleShape temp(1, 5);
					temp.setPosition(drawx, drawy);
					temp.setFillColor(Color(172, 174, 216));
					window.draw(temp);
				}
			}
			if (i.id != p->selectedId && i.id != toDrawId) {
				if (p->showNames) {
					Text nameText;
					nameText.setFont(font);
					nameText.setCharacterSize(10);
					nameText.setString(i.name);
					nameText.setOrigin((int)getWidth(nameText) / 2, 0);
					nameText.setPosition((int)getdrawx(i.x, p->cx, p->scale, window.getSize().x), (int)(getdrawy(i.y, p->cy, p->scale, window.getSize().y) + i.r / p->scale + 7));
					planetTextToDraw.push_back(std::make_pair(std::make_pair(0, i.m), nameText));
				}
			}
		}

		if (creationStep == 0) {
			ld dx = getdrawx(creationx, p->cx, p->scale, window.getSize().x);
			ld dy = getdrawy(creationy, p->cy, p->scale, window.getSize().y);
			ld curr = dist(dx, dy, Mouse::getPosition(window).x, Mouse::getPosition(window).y);
			CircleShape tempPlanet;
			tempPlanet.setOrigin(curr, curr);
			tempPlanet.setRadius(curr);
			tempPlanet.setPosition(dx, dy);
			tempPlanet.setFillColor(Color(172, 174, 216, 63));
			window.draw(tempPlanet);
		}
		if (creationStep == 1) {
			ld curx = getrealx(Mouse::getPosition(window).x, p->cx, p->scale, window.getSize().x);
			ld cury = getrealy(Mouse::getPosition(window).y, p->cy, p->scale, window.getSize().y);
			int drawx = getdrawx(creationx, p->cx, p->scale, window.getSize().x);
			int drawy = getdrawy(creationy, p->cy, p->scale, window.getSize().y);
			ld r = creationr / p->scale;

			ld vx = (curx - creationx) / curTimeSpeed;
			ld vy = (cury - creationy) / curTimeSpeed;
			if (p->selectedId != -1) {
				vx += selected.vx;
				vy += selected.vy;
			}
			int curSelectedId = p->selectedId;
			if (!p->relativeTraces) curSelectedId = -1;
			auto prediction = getPredictions(planets, planet(creationx, creationy, vx, vy, creationr, getVolume(creationr) * p->createDensity * (p->negativeMass ? -1 : 1)), 1000, curTimeSpeed, curSelectedId, p->timeToPredict);
			ld oldx = creationx, oldy = creationy;
			Color lineColor;
			if (planets.size() <= 20) {
				lineColor = Color(255, 255, 0, 127);
			}
			else {
				lineColor = Color(255, 255, 0, 63);
			}
			for (auto i : prediction) {
				ld curx = i.first, cury = i.second;
				drawLineSegment(window, getdrawx(oldx, p->cx, p->scale, window.getSize().x), getdrawy(oldy, p->cy, p->scale, window.getSize().y),
					getdrawx(curx, p->cx, p->scale, window.getSize().x), getdrawy(cury, p->cy, p->scale, window.getSize().y), lineColor);
				oldx = curx, oldy = cury;
			}

			CircleShape tempPlanet;
			tempPlanet.setOrigin(r, r);
			tempPlanet.setRadius(r);
			tempPlanet.setPosition(drawx, drawy);
			tempPlanet.setFillColor(Color(172, 174, 216, 63));
			window.draw(tempPlanet);
			drawArrow(window, drawx, drawy, Mouse::getPosition(window).x, Mouse::getPosition(window).y, Color::Red);
		}
		if (creationStep != -1) {
			ld drawx = getdrawx(creationx, p->cx, p->scale, window.getSize().x);
			ld drawy = getdrawy(creationy, p->cy, p->scale, window.getSize().y);
			ld r = ((creationStep == 0) ? (dist(drawx, drawy, Mouse::getPosition(window).x, Mouse::getPosition(window).y) * p->scale) : creationr);
			std::vector<String> lines;
			lines.push_back("m=" + (p->negativeMass ? "-" : std::string()) + to_string(getVolume(r) * p->createDensity) + " kg");
			lines.push_back("r=" + formatDist(r));
			ld vx = creationx - getrealx(Mouse::getPosition(window).x, p->cx, p->scale, window.getSize().x);
			ld vy = creationy - getrealy(Mouse::getPosition(window).y, p->cy, p->scale, window.getSize().y);
			if (p->selectedId != -1) {
				vx -= selected.vx;
				vy -= selected.vy;
			}
			vx /= p->timeSpeed;
			vy /= p->timeSpeed;
			ld v = ((creationStep == 0) ? 0 : dist(0, 0, vx, vy));
			lines.push_back("v=" + formatDist(v) + "/s\n");
			if (p->selectedId != -1) {
				lines.push_back("d=" + formatDist(dist(selected.x, selected.y, creationx, creationy)));
			}

			Text text(getName(lines), font, 10);
			text.setOrigin(getWidth(text) / 2, 0);
			text.setFillColor(Color(255, 255, 255, 255));
			text.setOutlineColor(Color(255, 255, 255, 255));
			text.setPosition((int)getdrawx(creationx, p->cx, p->scale, window.getSize().x), (int)getdrawy(creationy - r, p->cy, p->scale, window.getSize().y) + 7);
			planetTextToDraw.push_back(std::make_pair(std::make_pair(3, getVolume(r) * p->createDensity), text));
		}

		std::sort(planetTextToDraw.begin(), planetTextToDraw.end(), textPriorityComp);

		std::vector<std::pair<FloatRect, int>> textData(planetTextToDraw.size());
		for (int i = 0; i < planetTextToDraw.size(); i++) {
			auto cur = planetTextToDraw[i].second;
			FloatRect bounds = cur.getGlobalBounds();

			textData[i].first = bounds;

			textData[i].second = 255;
			for (int j = 0; j < i; j++) {
				ld x1 = bounds.left + bounds.width / 2, y1 = bounds.top + bounds.height / 2;
				ld x2 = textData[j].first.left + textData[j].first.width / 2, y2 = textData[j].first.top + textData[j].first.height / 2;
				ld d = dist(x1, y1, x2, y2) - dist(0, 0, bounds.width, bounds.height) / 2 - dist(0, 0, textData[j].first.width, textData[j].first.height) / 2;
				int cur = 255 - textData[j].second + d / 40 * 255;
				cur = std::max(cur, 0);
				cur = std::min(cur, 255);
				textData[i].second = std::min(textData[i].second, cur);
			}
		}

		for (int i = 0; i < planetTextToDraw.size(); i++) {
			planetTextToDraw[i].second.setFillColor(Color(255, 255, 255, textData[i].second));

			window.draw(planetTextToDraw[i].second);
		}

		if (p->paused) {
			Text data2;
			data2.setCharacterSize(10);
			data2.setFont(font);
			data2.setString("Pause");
			window.draw(data2);
		}
		chat.draw(window, font);

		static ld showUpdateTime = gettime() - 1;
		if (gettime() - showUpdateTime >= 0.2) {
			showUpdateTime = gettime();
			updatesPerSecondText.setFont(font);
			updatesPerSecondText.setCharacterSize(10);
			updatesPerSecondText.setString(to_string(notPausedUpdatesPerSecond, 1) + " UPS\n" + to_string(fps.getUpdatesPerSecond(), 1) + " FPS");
			updatesPerSecondText.setOrigin(getWidth(updatesPerSecondText) - 1, getHeight(updatesPerSecondText) - 1);
			updatesPerSecondText.setPosition(window.getSize().x - 1 - 4, window.getSize().y - 1 - 4);
		}
		window.draw(updatesPerSecondText);

		fps.addUpdateTime();

		menu.tick(window);
		if (menu.isActive()) {
			RectangleShape rect;
			rect.setSize((Vector2f)window.getSize());
			rect.setFillColor(Color(0, 0, 0, 127));
			window.draw(rect);
			menu.draw(window, font);
		}

		static ld fps20Time = gettime() - 1;
		if (gettime() - fps20Time >= 0.05) {
			fps20Time = gettime();
			std::string data1string;
			data1string += "Time speed:\n";
			if (p->realtime) {
				data1string += formatTime(p->timeSpeed) + "/s\n";
			}
			else {
				data1string += formatTime(notPausedUpdatesPerSecond * p->timeSpeed) + "/s (" + formatTime(p->timeSpeed) + "/U)\n";
			}
			data1string += "Scale: " + formatDist(p->scale) + "/pixel\n";
			data1string += "Objects count: " + std::to_string(planets.size()) + " (" + std::to_string(heavyCount) + "+" + std::to_string(lightCount) + ")\n";
			data1string += "Create density: " + (p->negativeMass ? "-" : std::string()) + to_string(p->createDensity, 0) + " kg/m^3\n";
			data1string += "Real time to predict: " + formatTime(p->timeToPredict) + "\n";
			data1string += (String)"Relative traces: " + (p->relativeTraces ? "On" : "Off") + "\n";
			data1string += "Timer: " + formatTime(curTimer) + "\n";
			dataText.setString(data1string);
		}
		window.draw(dataText);

		window.display();


		std::vector<Keyboard::Key> pressedKeys;

		ld buttondt = gettime() - lastButtonUpdated;
		lastButtonUpdated = gettime();
		if (window.hasFocus() && Keyboard::isKeyPressed(Keyboard::F11)) {
			if (!fullScreenPressed) {
				p->fullScreenMode = !p->fullScreenMode;
				if (!p->fullScreenMode) {
					window.create(VideoMode(960, 540), "Planets", sf::Style::Titlebar | sf::Style::Close);
				}
				else {
					window.create(VideoMode::getFullscreenModes()[0], "Planets", sf::Style::Fullscreen);
				}
				dataText.setPosition(window.getSize().x - 1, 0);
			}
			fullScreenPressed = true;
		}
		else fullScreenPressed = false;
		if (window.hasFocus() && !chat.getTyping()) {
			for (auto i : keysToHold) {
				if (Keyboard::isKeyPressed(i)) {
					pressedKeys.push_back(i);
				}
			}
		}

		if (locked) {
			int curx = Mouse::getPosition(window).x;
			int cury = Mouse::getPosition(window).y;
			if (dist(curx, cury, lockeddrawx, lockeddrawy) > 10) {
				choosingSelected = false;
			}
		}

		std::vector<int> keyCodes;

		Event event;
		while (window.pollEvent(event)) {
			static bool tPressed = false;
			if (event.type == Event::Closed) {
				window.close();
			}
			else if (event.type == Event::TextEntered) {
				Uint32 c = event.text.unicode;
				if (chat.getTyping()) {
					if (!tPressed) {
						chat.putChar(c);
					}
					else {
						tPressed = false;
					}
				}
			}
			else if (event.type == Event::MouseWheelMoved) {
				if (window.hasFocus()) {
					if (!menu.isActive()) {
						if (!chat.getTyping()) {
							int cur = event.mouseWheel.delta;
							ld mScale = 1;
							for (int i = 0; i < abs(cur); i++) {
								mScale *= (ld)1.1;
							}
							ld mousex = Mouse::getPosition(window).x, mousey = Mouse::getPosition(window).y;
							ld tx = p->cx + (mousex - window.getSize().x / 2) * p->scale, ty = p->cy + (window.getSize().y / 2 - mousey) * p->scale;
							ld vx = -(mousex - window.getSize().x / 2), vy = -(window.getSize().y / 2 - mousey);
							if (cur < 0) {
								p->scale *= mScale;
							}
							else {
								p->scale /= mScale;
							}
							p->cx = tx + vx * p->scale;
							p->cy = ty + vy * p->scale;
						}
						else {
							int dpos = event.mouseWheel.delta;
							if (chat.getTyping()) {
								chat.scroll(dpos);
							}
						}
					}
				}
			}
			else if (event.type == Event::KeyPressed) {
				if (window.hasFocus()) {
					if (chat.getTyping()) {
						if (event.key.code == Keyboard::Up) {
							chat.goUp();
						}
						if (event.key.code == Keyboard::Down) {
							chat.goDown();
						}
						if (event.key.code == Keyboard::Left) {
							if (!Keyboard::isKeyPressed(Keyboard::LControl) && !Keyboard::isKeyPressed(Keyboard::RControl)) {
								chat.moveCursor(-1);
							}
							else chat.moveCursorCtrl(-1);
						}
						if (event.key.code == Keyboard::Right) {
							if (!Keyboard::isKeyPressed(Keyboard::LControl) && !Keyboard::isKeyPressed(Keyboard::RControl)) {
								chat.moveCursor(1);
							}
							else chat.moveCursorCtrl(1);
						}
						if (event.key.code == Keyboard::Delete) {
							chat.deleteSymbol();
						}
						if (event.key.code == Keyboard::Escape) {
							chat.setTyping(false);
						}
					}
					else {
						if (event.key.code == Keyboard::Slash) {
							if (!menu.isActive()) {
								chat.setTyping(true);
							}
						}
						else if (event.key.code == Keyboard::T) {
							if (!menu.isActive()) {
								tPressed = true;
								chat.setTyping(true);
							}
						}
						else if (event.key.code == Keyboard::Escape) {
							if (creationStep != -1) {
								creationStep = -1;
							}
							else {
								if (!menu.isActive()) {
									p->timeMtx.lock();
									wasPaused = p->paused;
									p->paused = true;
									p->timeMtx.unlock();
								}
								else {
									p->timeMtx.lock();
									p->paused = p->paused && wasPaused;
									p->timeMtx.unlock();
								}
								menu.setActive(!menu.isActive());
							}
						}
						if (std::find(keysToPress.begin(), keysToPress.end(), event.key.code)
							!= keysToPress.end()) {
							pressedKeys.push_back(event.key.code);
						}
						if (event.key.code == Keyboard::P) {
							p->timeMtx.lock();
							p->paused = !p->paused;
							p->timeMtx.unlock();
						}
					}
				}
			}
			else if (event.type == Event::MouseButtonPressed) {
				if (window.hasFocus()) {
					if (!menu.isActive()) {
						if (event.mouseButton.button == Mouse::Left && !Keyboard::isKeyPressed(Keyboard::LAlt)) {
							locked = true;
							lockeddrawx = Mouse::getPosition(window).x;
							lockeddrawy = Mouse::getPosition(window).y;
							lockedrealx = getrealx(lockeddrawx, p->cx, p->scale, window.getSize().x);
							lockedrealy = getrealy(lockeddrawy, p->cy, p->scale, window.getSize().y);
							choosingSelected = true;
						}
						if (Keyboard::isKeyPressed(Keyboard::LAlt) && event.mouseButton.button == Mouse::Left || event.mouseButton.button == Mouse::Middle) {
							ld curx = getrealx(Mouse::getPosition(window).x, p->cx, p->scale, window.getSize().x);
							ld cury = getrealy(Mouse::getPosition(window).y, p->cy, p->scale, window.getSize().y);
							if (creationStep == -1) {
								creationx = curx;
								creationy = cury;
								creationStep = 0;
							}
							else if (creationStep == 0) {
								creationr = dist(curx, cury, creationx, creationy);
								creationStep = 1;
							}
							else if (creationStep == 1) {
								creationvx = (curx - creationx) / curTimeSpeed;
								creationvy = (cury - creationy) / curTimeSpeed;
								if (p->selectedId != -1) {
									p->insertPlanet(planet(creationx - selected.x, creationy - selected.y, creationvx, creationvy,
										creationr, getVolume(creationr) * p->createDensity * (p->negativeMass ? -1 : 1)), p->selectedId);
								}
								else {
									p->insertPlanet(planet(creationx, creationy, creationvx, creationvy, creationr, getVolume(creationr) * p->createDensity * (p->negativeMass ? -1 : 1)));
								}
								creationStep = -1;
							}
						}
						if (event.mouseButton.button == Mouse::Right) {
							ld curx = getrealx(Mouse::getPosition(window).x, p->cx, p->scale, window.getSize().x);
							ld cury = getrealy(Mouse::getPosition(window).y, p->cy, p->scale, window.getSize().y);
							ld maxdist = p->scale * 20;
							for (auto i : planets) {
								if (dist(curx, cury, i.x, i.y) <= maxdist + i.r) {
									p->deletePlanet(i.id);
								}
							}
						}
					}
				}
			}
			else if (event.type == Event::MouseButtonReleased) {
				if (window.hasFocus()) {
					if (event.mouseButton.button == Mouse::Left && !Keyboard::isKeyPressed(Keyboard::LAlt)) {
						if (locked) {
							locked = false;
							if (choosingSelected && !Keyboard::isKeyPressed(Keyboard::LAlt)) {
								ld maxm;
								int curx = Mouse::getPosition(window).x;
								int cury = Mouse::getPosition(window).y;
								int selectedid = -1;
								for (int i = 0; i < planets.size(); i++) {
									ld drawx = getdrawx(planets[i].x, p->cx, p->scale, window.getSize().x);
									ld drawy = getdrawy(planets[i].y, p->cy, p->scale, window.getSize().y);
									ld d = dist(curx, cury, drawx, drawy) - planets[i].r / p->scale;
									if (d < 20) {
										if (selectedid == -1 || planets[i].m > maxm) {
											selectedid = i;
											maxm = planets[i].m;
										}
									}
								}
								p->selectedId = ((selectedid == -1) ? -1 : planets[selectedid].id);
							}
						}
					}
				}
			}
		}

		menu.setButtonText(0, p->drawLines ? "On" : "Off");
		menu.setButtonText(1, p->relativeTraces ? "On" : "Off");
		menu.setButtonText(2, p->showNames ? "On" : "Off");
		menu.setButtonText(3, p->negativeMass ? "On" : "Off");
		menu.setButtonText(4, p->realtime ? "Real" : "Accurate");
		std::vector<bool> menuPresses = menu.getPresses();
		std::vector<bool> menuHolds = menu.getHolds();
		for (int i = 0; i < keysToPress.size(); i++) {
			if (menuPresses[i]) {
				pressedKeys.push_back(keysToPress[i]);
			}
		}
		{
			String temp = "Time speed: \n";
			if (p->realtime) {
				temp += formatTime(p->timeSpeed) + "/s\n";
			}
			else {
				temp += formatTime(notPausedUpdatesPerSecond * p->timeSpeed) + "/s (" + formatTime(p->timeSpeed) + "/U)\n";
			}
			menu.setTextString(keysToPress.size() + 0, temp);
		}
		menu.setTextString(keysToPress.size() + 1, "Create density: \n" + (p->negativeMass ? "-" : std::string()) + to_string(p->createDensity, 0) + " kg/m^3\n");
		menu.setTextString(keysToPress.size() + 2, "Real time to predict: \n" + formatTime(p->timeToPredict) + "\n");
		for (int i = 0; i < keysToHold.size(); i++) {
			if (menuHolds[keysToPress.size() + i]) {
				pressedKeys.push_back(keysToHold[i]);
			}
		}

		for (auto i : pressedKeys) {
			if (i == Keyboard::Comma) {
				p->timeMtx.lock();
				p->timeSpeed /= pow(3, buttondt);
				p->timeMtx.unlock();
			}
			if (i == Keyboard::Period) {
				p->timeMtx.lock();
				p->timeSpeed *= pow(3, buttondt);
				p->timeMtx.unlock();
			}
			if (i == Keyboard::LBracket) {
				p->timeToPredict /= pow(2, buttondt);
			}
			if (i == Keyboard::RBracket) {
				p->timeToPredict *= pow(2, buttondt);
			}
			if (i == Keyboard::Add) {
				p->createDensity *= pow(4, buttondt);
			}
			if (i == Keyboard::Subtract) {
				p->createDensity /= pow(4, buttondt);
			}

			if (i == Keyboard::L) {
				p->drawLines = !p->drawLines;
				traces.clear();
			}
			if (i == Keyboard::R) {
				p->relativeTraces = !p->relativeTraces;
			}
			if (i == Keyboard::N) {
				p->showNames = !p->showNames;
			}
			if (i == Keyboard::M) {
				p->negativeMass = !p->negativeMass;
			}
			if (i == Keyboard::Z) {
				p->timeMtx.lock();
				p->realtime = !p->realtime;
				ld cur = notPausedUpdatesPerSecond;
				if (cur < 1) cur = 1;
				if (!p->realtime) {
					p->timeSpeed /= cur;
				}
				else {
					p->timeSpeed *= cur;
				}
				p->timeMtx.unlock();
			}
			if (i == Keyboard::Q) {
				if (p->selectedId != -1) {
					for (auto& i : planets) {
						i.x -= selected.x;
						i.y -= selected.y;
						i.vx -= selected.vx;
						i.vy -= selected.vy;
					}
					selected.x = 0;
					selected.y = 0;
					selected.vx = 0;
					selected.vy = 0;
					p->setPlanets(planets);
					oldPaused = true;
					if (!traces.empty()) {
						traces.clear();
					}
				}
			}
			if (i == Keyboard::J) {
				p->timeMtx.lock();
				p->timer = 0;
				p->timeMtx.unlock();
			}
		}
	}
	p->close();
	calcThread.wait();
}
int main() {
	RenderWindow window;
	gamestate* p = new gamestate();
	p->fullScreenMode = false;
	if (!p->fullScreenMode) {
		window.create(VideoMode(960, 540), "Planets", sf::Style::Titlebar | sf::Style::Close);
	}
	else {
		window.create(VideoMode::getFullscreenModes()[0], "Planets", sf::Style::Fullscreen);
	}
	window.setVerticalSyncEnabled(true);
	p->clear();
	window.clear();
	window.display();
	runSimulation(window, p);
	return 0;
}
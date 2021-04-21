#include "Psystem.h"
#include "Functions.h"
#include <set>

std::pair<int, int> psystem::findCollision1() {
	std::vector<std::pair<ld, int>> v;
	for (int i = 0; i < s.size(); i++) {
		v.push_back({ s[i].x - s[i].r, -i - 1 });
		v.push_back({ s[i].x + s[i].r, i });
	}
	std::sort(v.begin(), v.end());
	std::set<std::pair<ld, int>> setik;
	for (auto i : v) {
		int id = ((i.second >= 0) ? i.second : (-i.second - 1));
		if (i.second < 0) {
			auto it = setik.lower_bound(std::make_pair(s[id].y, -1));
			if (it != setik.end() && collide(s[it->second], s[id])) {
				return std::make_pair(it->second, id);
			}
			if (it != setik.begin())it--;
			if (it != setik.end() && collide(s[it->second], s[id])) {
				return std::make_pair(it->second, id);
			}
			setik.insert(std::make_pair(s[id].y, id));
		}
		else {
			setik.erase(std::make_pair(s[id].y, id));
			auto it = setik.lower_bound(std::make_pair(s[id].y, -1));
			if (it != setik.end() && collide(s[it->second], s[id])) {
				return std::make_pair(it->second, id);
			}
			if (it != setik.begin())it--;
			if (it != setik.end() && collide(s[it->second], s[id])) {
				return std::make_pair(it->second, id);
			}
		}
	}
	return std::make_pair(-1, -1);
}
std::pair<int, int> psystem::findCollision2() {
	std::vector<int> heavy;
	for (int i = 0; i < s.size(); i++) {
		if (s[i].m != 0) {
			heavy.push_back(i);
		}
	}
	for (int i = 0; i < s.size(); i++) {
		for (auto j : heavy) {
			if (i != j && collide(s[i], s[j])) {
				return std::make_pair(i, j);
			}
		}
	}
	return std::make_pair(-1, -1);
}
psystem::psystem() : s(std::vector<planet>()) {}
psystem::psystem(std::vector<planet>s) : s(s) {}
void psystem::clear() {
	s.clear();
}
void psystem::deletePlanet(int i) {
	s.erase(s.begin() + i);
}
void psystem::insertPlanet(const planet& p) {
	s.push_back(p);
}
void psystem::updateCoords(ld dt) {
	for (auto& p : s) {
		p.updateCoords(dt);
	}
}
void psystem::updateVelocity(ld dt) {
	std::vector<int> heavy;
	heavy.reserve(s.size());
	for (int i = 0; i < s.size(); i++) {
		if (s[i].m != 0) heavy.push_back(i);
	}
	int startPos = 0;
	ld Gdt = planet::G * dt;
	for (int i = 0; i < s.size(); i++) {
		if (startPos != heavy.size() && i == heavy[startPos]) {
			startPos++;
		}
		if (s[i].m != 0) {
			for (int k = startPos; k < heavy.size(); k++) {
				int j = heavy[k];
				ld d2 = dist2(s[i].x, s[i].y, s[j].x, s[j].y);
				ld d3 = d2 * sqrt(d2);
				ld Gdtd3 = Gdt / d3;
				ld tx = Gdtd3 * (s[j].x - s[i].x);
				ld ty = Gdtd3 * (s[j].y - s[i].y);
				s[i].vx += tx * s[j].m;
				s[i].vy += ty * s[j].m;
				s[j].vx += tx * -s[i].m;
				s[j].vy += ty * -s[i].m;
			}
		}
		else {
			for (const auto& j : heavy) {
				ld d2 = dist2(s[i].x, s[i].y, s[j].x, s[j].y);
				ld d3 = d2 * sqrt(d2);
				ld Gdtd3 = Gdt / d3;
				ld tx = Gdtd3 * (s[j].x - s[i].x);
				ld ty = Gdtd3 * (s[j].y - s[i].y);
				s[i].vx += tx * s[j].m;
				s[i].vy += ty * s[j].m;
			}
		}
	}
}
std::vector<std::pair<int, int>> psystem::updateCollided() {
	std::vector<std::pair<int, int>> recentlyCollided;
	int n = 0, m = 0;
	for (auto i : s) {
		if (i.m != 0) {
			n++;
		}
		else {
			m++;
		}
	}
	while (true) {
		std::pair<int, int> temp;
		if (4 * log2(n + m) <= n) {
			temp = findCollision1();
		}
		else {
			temp = findCollision2();
		}
		if (temp.first == -1) break;
		if (temp.first < temp.second) std::swap(temp.first, temp.second);
		s.push_back(s[temp.first] + s[temp.second]);
		recentlyCollided.push_back(std::make_pair(s.back().id, s[temp.first].id));
		recentlyCollided.push_back(std::make_pair(s.back().id, s[temp.second].id));
		deletePlanet(temp.first);
		deletePlanet(temp.second);
	}
	return recentlyCollided;
}
std::vector<planet> psystem::getPlanets() const {
	return s;
}

std::vector<std::pair<ld, ld>> getPredictions(psystem s, planet p, int iterations, ld timeSpeed, int selectedId, ld timeToPredict) {
	ld startSelectedx = 0, startSelectedy = 0;
	if (selectedId != -1) {
		for (auto i : s.getPlanets()) {
			if (i.id == selectedId) {
				startSelectedx = i.x;
				startSelectedy = i.y;
				break;
			}
		}
	}
	if (s.getPlanets().size() > 20) {
		std::vector<std::pair<ld, int>> leveOlon;
		for (auto i : s.getPlanets()) {
			ld d2 = dist2(i.x, i.y, p.x, p.y);
			ld d3 = d2 * sqrt(d2);
			ld ax = p.m * (i.x - p.x) / d3;
			ld ay = p.m * (i.y - p.y) / d3;
			std::pair<ld, int> cur = std::make_pair(ax * ax + ay * ay, i.id);
			for (int j = 0; j < leveOlon.size(); j++) {
				if (cur.first > leveOlon[j].first) {
					std::swap(cur, leveOlon[j]);
				}
			}
			leveOlon.push_back(cur);
			if (leveOlon.size() > 20) leveOlon.pop_back();
		}
		std::vector<planet> toPredict;
		for (auto i : s.getPlanets()) {
			bool found = false;
			if (i.id == selectedId) {
				found = true;
			}
			else {
				for (auto j : leveOlon) {
					if (j.second == i.id) {
						found = true;
						break;
					}
				}
			}
			if (found) {
				toPredict.push_back(i);
			}
		}
		s.clear();
		for (auto i : toPredict) s.insertPlanet(i);
	}
	for (int i = 0; i < s.getPlanets().size(); i++) {
		if (s.s[i].id == p.id) {
			s.s.erase(s.s.begin() + i);
			break;
		}
	}

	s.insertPlanet(p);
	std::vector<std::pair<ld, ld>> res;
	DSU d;
	int id = p.id;
	res.push_back(std::make_pair(p.x, p.y));
	for (int i = 0; i < iterations; i++) {
		auto temp = s.updateCollided();
		for (auto i : temp) d.unite(i.first, i.second);
		s.updateVelocity(timeToPredict * timeSpeed / iterations);
		s.updateCoords(timeToPredict * timeSpeed / iterations);
		id = d.get(id);
		ld selectedx = 0, selectedy = 0;
		if (selectedId != -1) {
			selectedId = d.get(selectedId);
			for (auto j : s.getPlanets()) {
				if (j.id == selectedId) {
					selectedx = j.x;
					selectedy = j.y;
					break;
				}
			}
		}
		for (auto j : s.getPlanets()) {
			if (j.id == id) {
				res.push_back(std::make_pair(j.x - selectedx + startSelectedx, j.y - selectedy + startSelectedy));
				break;
			}
		}
	}
	return res;
}
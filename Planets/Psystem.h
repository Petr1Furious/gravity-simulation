#pragma once

#include "Planet.h"

struct psystem {
private:
	std::pair<int, int> findCollision1();
	std::pair<int, int> findCollision2();
public:
	std::vector<planet> s;
	psystem();
	psystem(std::vector<planet>s);
	void clear();
	void deletePlanet(int i);
	void insertPlanet(const planet& p);
	void updateCoords(ld dt);
	void updateVelocity(ld dt);
	std::vector<std::pair<int, int>> updateCollided();
	std::vector<planet> getPlanets() const;
};

std::vector<std::pair<ld, ld>> getPredictions(psystem s, planet p, int iterations, ld timeSpeed, int selectedId = -1, ld timeToPredict = 1);
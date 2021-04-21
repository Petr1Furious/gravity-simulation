#include "Traces.h"
#include "Functions.h"

tracePoint::tracePoint() {}
tracePoint::tracePoint(ld x, ld y, ld t) : x(x), y(y), t(t) {}

trace::trace() {}

void trace::insertPoint(tracePoint t) {
	if (v.size() >= 2) {
		/*ld v1x = t.x - v.back().x, v1y = t.y - v.back().y;
		ld v2x = v.back().x - v[(int)v.size() - 2].x, v2y = v.back().y - v[(int)v.size() - 2].y;
		ld sn = v1x * v2x + v1y * v2y;
		ld cs = v1x * v2y - v2x * v1y;
		if (abs(atan2(cs, sn)) <= M_PI / 128) v.pop_back();*/
		// if (t.t - v.back().t > (ld)1 / 20) v.pop_back();
	}
	v.push_back(t);
}

void trace::draw(sf::RenderWindow& window, ld cx, ld cy, ld scale, const trace& sTrace, ld sx, ld sy) {
	sf::VertexArray t(sf::LineStrip, v.size());
	int tpos = 0;

	if (!sTrace.v.empty()) {
		int pos = 0;
		for (const auto& i : v) {
			while (pos < sTrace.v.size() - 1 && sTrace.v[pos + 1].t < i.t) pos++;
			if (pos != sTrace.v.size() - 1 && sTrace.v[pos].t < i.t && i.t <= sTrace.v[pos + 1].t) {
				ld vx = sTrace.v[pos + 1].x - sTrace.v[pos].x, vy = sTrace.v[pos + 1].y - sTrace.v[pos].y;
				ld lw = (i.t - sTrace.v[pos].t) / (sTrace.v[pos + 1].t - sTrace.v[pos].t);
				vx *= lw;
				vy *= lw;
				ld curx = sTrace.v[pos].x + vx, cury = sTrace.v[pos].y + vy;
				t[tpos].color = sf::Color(255, 255, 255, 31);
				t[tpos++] = sf::Vertex(sf::Vector2f(getdrawx(sx - curx + i.x, cx, scale, window.getSize().x), getdrawy(sy - cury + i.y, cy, scale, window.getSize().y)));
			}
		}
		t.resize(tpos);
	}
	else {
		for (const auto& i : v) {
			t[tpos].color = sf::Color(255, 255, 255, 31);
			t[tpos++] = sf::Vertex(sf::Vector2f(getdrawx(i.x, cx, scale, window.getSize().x), getdrawy(i.y, cy, scale, window.getSize().y)));
		}
	}

	window.draw(t);
}

bool trace::operator<(const trace& t) const {
	return m < t.m;
}
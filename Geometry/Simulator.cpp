#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
using namespace std;
const double EPS = 1e-9;
const double INF = 1e9;
struct v2d {
	double x = 0, y = 0;
	v2d() :x(0), y(0) {};
	v2d(double x, double y) : x(x), y(y) {};
	double mag() {
		return hypot(x, y);
	}
	v2d normalize() {
		double mymag = mag();
		return v2d(x / mymag, y / mymag);
	}
	double operator*(v2d b) const {
		return x * b.x + y * b.y;
	}
};

v2d operator*(double b, v2d a) {
	return v2d{ a.x * b,a.y * b };
}
v2d operator*(v2d a, double b) {
	return v2d{ a.x * b,a.y * b };
}
v2d operator+(v2d a, v2d b) {
	return v2d{ a.x + b.x,a.y + b.y };
}
v2d operator-(v2d a, v2d b) {
	return v2d{ a.x - b.x,a.y - b.y };
}

bool comp_lenient(double a, double b) {//a<b
	return a < b + EPS;
}

bool comp_strict(double a, double b) {//a<b
	return a + EPS < b;
}
struct Collision;
struct Segment;
struct Ray {
	v2d ori;
	v2d dir;
	Ray(v2d ori, v2d _dir, bool normalize = true) : ori(ori) {
		dir = normalize ? _dir.normalize() : _dir;
	};
	Ray() {};
	void advance(double dt) {
		ori = ori + dir * dt;
	}
	bool intersect(const Ray& r, double& a, double& b) const;
	bool intersect(const Segment& s, Collision& collision);
};

struct Segment {
	v2d p1;
	v2d p2;
	v2d normal;
	Ray asRay;
	double length;
	int id;
	Segment(const v2d& p1, const v2d& p2, int id) :p1(p1), p2(p2), id(id) {
		normal = v2d(p1.y - p2.y, p2.x - p1.x).normalize();
		asRay = Ray(p1, p2 - p1);
		length = (p1 - p2).mag();
	}
	v2d reflect(const v2d& ray) const {
		v2d reflected = ray - 2.0 * normal * (normal * ray);
		return reflected;
	}
};

struct Collision {
	int id_ball = -1;
	int id_segment = -1;
	Ray continued;
	Ray reflected;
	double t = 0;
	Collision() {}
};
bool Ray::intersect(const Ray& r, double& a, double& b) const {
	//solve
	//ori.x+a*dir.x=r.ori.x+b*r.dir.x;
	//ori.y+a*dir.y=r.ori.y+b*r.dir.y;
	double dx = ori.x - r.ori.x;
	double dy = ori.y - r.ori.y;
	double crossp = r.dir.x * dir.y - dir.x * r.dir.y;
	if (comp_lenient(abs(crossp), 0)) {
		a = b = 0;
		return false;
	}
	a = (r.dir.y * dx - r.dir.x * dy) / crossp;
	b = (dir.y * dx - dir.x * dy) / crossp;
	return (comp_lenient(0, a) && comp_lenient(0, b));
}
bool Ray::intersect(const Segment& s, Collision& collision) {
	double a = 0, b = 0;
	bool success = intersect(s.asRay, a, b);
	if (success && comp_lenient(b, s.length)) {
		v2d pt = ori + dir * a;
		collision.continued = Ray(pt, dir, false);
		collision.reflected = Ray(pt, s.reflect(dir), false);
		collision.t = a;
		return true;
	}
	return false;
}
typedef Ray Ball;

const int SCREEN_BIT = (1 << 20);
const int BOTTOM_BIT = (1 << 21);
const double SIMULTANEOUS_EPS = 1e-7;
const double BALL_INTERVAL = 1;

bool is_screen(int id) {
	return id & SCREEN_BIT;
}
bool is_bottom(int id) {
	return id & BOTTOM_BIT;
}

struct GameState {
	int lives[25];
	Ray gun;
	vector<Ball> balls;
	vector<Segment> segments;
	vector<Collision> collisions;
	double time_to_next_ball = 0;
	int balls_shot = 0;
	int total_balls = 0;
	int amount_objects = 0;
	bool dirty = true;
	GameState() {
		memset(lives, 0, sizeof lives);
	}

	void printState() {
		for (int i = 0; i < amount_objects; ++i) {
			if (i)cout << " ";
			cout << max(lives[i], 0);
		}
		cout << endl;
	}

	Collision getNearestCollision(Ball ball, int id_ball) {
		double mint = INF;
		Collision collision;
		Collision cur_collision;
		for (const Segment& s : segments) {
			if (!is_screen(s.id) && lives[s.id] <= 0) continue;
			bool intersected = ball.intersect(s, cur_collision);
			if (intersected && comp_strict(0, cur_collision.t)) {
				if (mint > cur_collision.t) {//we want exact comparison here without EPS
					mint = cur_collision.t;
					collision = cur_collision;
					collision.id_ball = id_ball;
					collision.id_segment = s.id;
				}
			}
		}
		return collision;
	}
	void simulateStep();
};

void GameState::simulateStep() {
	if (dirty) {
		collisions = vector<Collision>();
		for (int id_ball = 0; id_ball < balls.size(); ++id_ball) {
			Ball ball = balls[id_ball];
			Collision collision = getNearestCollision(ball, id_ball);
			collisions.push_back(collision);
		}
		dirty = false;
	}
	double mint = INF;
	for (int i = 0; i < collisions.size(); ++i) {
		mint = min(mint, collisions[i].t);
	}
	if (balls_shot < total_balls && time_to_next_ball < mint) {
		++balls_shot;
		for (int i = 0; i < balls.size(); ++i) {
			balls[i].advance(time_to_next_ball);
			collisions[i].t -= time_to_next_ball;
		}
		balls.push_back(Ball(gun));
		collisions.push_back(getNearestCollision(balls.back(), (int)balls.size() - 1));
		time_to_next_ball = BALL_INTERVAL;
		return;
	}
	vector<bool> ball_death(collisions.size());
	double max_processed_t = -INF;
	for (int i = 0; i < collisions.size(); ++i) {
		if (collisions[i].t < mint + SIMULTANEOUS_EPS) {
			max_processed_t = max(max_processed_t, collisions[i].t);
			if (is_bottom(collisions[i].id_segment)) {
				ball_death[i] = true;
			}
			else if (!is_screen(collisions[i].id_segment)) {
				lives[collisions[i].id_segment] = max(0, lives[collisions[i].id_segment] - 1);
			}
		}
	}
	for (int i = 0; i < collisions.size(); ++i) {
		if (ball_death[i]) continue;
		double dt = 0;
		if (collisions[i].t < mint + SIMULTANEOUS_EPS) {
			if (!is_screen(collisions[i].id_segment) && lives[collisions[i].id_segment] <= 0) {
				dirty = true;
				balls[i] = collisions[i].continued;
			}
			else {
				balls[i] = collisions[i].reflected;
			}
			dt = max_processed_t - collisions[i].t;
			balls[i].advance(dt);
			collisions[i] = getNearestCollision(balls[i], i);
		}
		else {
			dt = max_processed_t;
			balls[i].advance(dt);
			collisions[i].t -= dt;
		}
	}
	vector<Ball> next_balls;
	vector<Collision> next_collisions;
	for (int i = 0; i < collisions.size(); ++i) {
		if (!ball_death[i]) {
			next_balls.push_back(balls[i]);
			next_collisions.push_back(collisions[i]);
		}
	}
	balls = next_balls;
	collisions = next_collisions;
	time_to_next_ball -= max_processed_t;
}

GameState readState(std::istream& cin) {
	GameState gameState = GameState();
	int w, h, n, m;
	double l, r, s;
	cin >> w >> h >> n >> m >> l >> r >> s;
	Segment bottom_wall = Segment(v2d(0, 0), v2d(w, 0), SCREEN_BIT | BOTTOM_BIT);
	Segment right_wall = Segment(v2d(w, 0), v2d(w, h), SCREEN_BIT);
	Segment top_wall = Segment(v2d(w, h), v2d(0, h), SCREEN_BIT);
	Segment left_wall = Segment(v2d(0, h), v2d(0, 0), SCREEN_BIT);
	gameState.segments = { bottom_wall,right_wall,top_wall,left_wall };
	vector<vector<v2d>> objects(m);
	for (int i = 0; i < m; ++i) {
		int p;
		cin >> p;
		for (int j = 0; j < p; ++j) {
			double x, y;
			cin >> x >> y;
			objects[i].push_back(v2d{ x,y });
		}
		cin >> gameState.lives[i];
		for (int j = 0; j < p; ++j) {
			gameState.segments.push_back(
				Segment(
					objects[i][j],
					objects[i][(j + 1) % p],
					i
				)
			);
		}
	}
	gameState.gun = Ray(v2d(l, 0), v2d(r, s));
	gameState.balls = { Ball(gameState.gun) };
	gameState.balls_shot = 1;
	gameState.time_to_next_ball = BALL_INTERVAL;
	gameState.total_balls = n;;
	gameState.amount_objects = m;

	return gameState;
}

int main() {
	GameState gameState = readState(std::cin);
	while (gameState.balls_shot < gameState.total_balls || gameState.balls.size()) {
		gameState.simulateStep();
	}
	gameState.printState();
}
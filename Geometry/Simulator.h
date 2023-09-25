// File: GameSimulation.h

#ifndef GAMESIMULATION_H
#define GAMESIMULATION_H

#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>

const double EPS = 1e-9;
const double INF = 1e9;
const int SCREEN_BIT = (1 << 20);
const int BOTTOM_BIT = (1 << 21);
const double SIMULTANEOUS_EPS = 1e-7;

struct v2d {
    double x, y;
    v2d();
    v2d(double x, double y);
    double mag();
    v2d normalize();
    double operator*(v2d b) const;
};

v2d operator*(double b, v2d a);
v2d operator*(v2d a, double b);
v2d operator+(v2d a, v2d b);
v2d operator-(v2d a, v2d b);

bool comp_lenient(double a, double b);
bool comp_strict(double a, double b);

struct Collision;
struct Segment;

struct Ray {
    v2d ori, dir;
    Ray();
    Ray(v2d ori, v2d _dir, bool normalize = true);
    void advance(double dt);
    bool intersect(const Segment &s, Collision& collision);
};

struct Segment {
    v2d p1, p2;
    v2d normal;
    Ray asRay;
    double length;
    int object_id;
    Segment(const v2d& p1, const v2d& p2, int id);
    v2d reflect(v2d &ray) const;
};

struct Collision {
    int ball_index, object_id;
    Ray continued, reflected;
    double t;
    Collision();
};

bool is_screen(int id);
bool is_bottom(int id);

typedef Ray Ball;

struct GameState {
    int lives[25];
    Ray gun;
    std::vector<Ball> balls;
    std::vector<Segment> segments;
    std::vector<Collision> collisions;
    double time_to_next_ball;
    int balls_shot, total_balls, amount_objects;
    bool dirty;
    GameState();
    void printState();
    void simulateStep();
};
GameState readState(std::istream& cin);

#endif  // GAMESIMULATION_H

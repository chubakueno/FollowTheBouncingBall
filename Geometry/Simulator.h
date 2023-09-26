#pragma once

#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>

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

typedef Ray Ball;

struct GameState {
    int lives[25];
    Ray gun;
    std::vector<Ball> balls;
    std::vector<Segment> segments;
    std::vector<Collision> collisions;
    int amount_objects;
    bool dirty;
    GameState();
    void printState();
    void simulateStep();
};
GameState readState(std::istream& cin);

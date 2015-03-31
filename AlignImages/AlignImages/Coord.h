#include <string>
#define PI 3.14159265
using namespace std;

#pragma once
class Coord{
public:
    float x;
    float y;
public:
    Coord(float _x, float _y){
        x = _x;
        y = _y;
    }
    Coord(float _x, float _y, float angle){
        float r = sqrtf(powf(_x,2) + powf(_y, 2));
        float ang = atan2(_y , _x);
        ang += (angle * PI/180);
        x = r * cosf(ang);
        y = r * sinf(ang);
    }
    Coord(){
        y = 0;
        x = 0;
    }
};

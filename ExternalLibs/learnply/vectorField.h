#ifndef VECTORFIELD_H
#define VECTORFIELD_H

#include <math.h>

class Point {
public:
    float posx, posy;
    float d;
    Point(float x, float y, float cd) {
        posx = x;
        posy = y;
        //d = cd;
    }

    Point(float x, float y) {
        posx = x;
        posy = y;
        //d = cd;
    }

    void setD(float cd)
    {
        d = cd;
    }

    virtual void getVec(float x, float y, float* vx, float* vy) {

    }
};

class Source : public Point {
public:
    Source(float x, float y, float cd) : Point(x, y, cd) {
    };
    void getVec(float x, float y, float* vx, float* vy)
    {
        float vecx, vecy;
        float power = -((x - posx) * (x - posx) + (y - posy) * (y - posy));
        vecx = (x - posx) * pow(d, power);
        vecy = (y - posy) * pow(d, power);
        *vx = vecx;
        *vy = vecy;
    }
};

class Saddle : public Point {
public:
    Saddle(float x, float y, float cd) : Point(x, y, cd) {

    }
    void getVec(float x, float y, float* vx, float* vy)
    {
        float vecx, vecy;
        float power = -((x - posx) * (x - posx) + (y - posy) * (y - posy));
        vecx = (posx - x) * pow(d, power);
        vecy = (y - posy) * pow(d, power);
        *vx = vecx;
        *vy = vecy;
    }
};

class Sink : public Point {
public:
    Sink(float x, float y, float cd) :Point(x, y, cd) {

    }
    void getVec(float x, float y, float* vx, float* vy)
    {
        float vecx, vecy;
        double power = -((x - posx) * (x - posx) + (y - posy) * (y - posy));
        vecx = (posx - x) * pow(d, power);
        vecy = (posy - y) * pow(d, power);
        //printf("vx is: %f, vy is: %f\n", vecx, vecy);
        *vx = vecx;
        *vy = vecy;
    }
};

class Center : public Point {
public:
    Center(float x, float y, float cd) :Point(x, y, cd) {

    }
    void getVec(float x, float y, float* vx, float* vy)
    {
        float vecx, vecy;
        float power = -((x - posx) * (x - posx) + (y - posy) * (y - posy));
        vecx = (y - posy) * pow(d, power);
        vecy = (posx - x) * pow(d, power);
        *vx = vecx;
        *vy = vecy;
    }
};

class Dipole : public Point {
public:
    Dipole(float x, float y, float cd) :Point(x, y, cd) {

    }
    void getVec(float x, float y, float* vx, float* vy)
    {
        float vecx, vecy;
        float power = -((x - posx) * (x - posx) + (y - posy) * (y - posy));
        vecx = ((x - posx) * (x - posx) -
            (y - posy) * (y - posy)) * pow(d, power);
        vecy = 2 * (x - posx) * (y - posy) * pow(d, power);
        *vx = vecx;
        *vy = vecy;
    }
};

class Monkey : public Point {
public:
    Monkey(float x, float y, float cd) :Point(x, y, cd) {

    }
    void getVec(float x, float y, float* vx, float* vy)
    {
        float vecx, vecy;
        float power = -((x - posx) * (x - posx) + (y - posy) * (y - posy));
        vecx = 2 * (x - posx) * (y - posy) * pow(d, power);
        vecy = ((x - posx) * (x - posx) -
            (y - posy) * (y - posy)) * pow(d, power);
        *vx = vecx;
        *vy = vecy;
    }
};

#endif // VECTORFIELD_H

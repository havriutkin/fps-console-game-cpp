#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include <math.h>

/*
    Representation of a line in 3d space using parameter t:
        (x0, y0, z0) + a * t
*/

class Vector3D 
{
private:
    float fX;
    float fY;
    float fZ;
    float fLength;
public:
    Vector3D();

    Vector3D(float x, float y, float z);

    void normalize();

    float getLength() const;

    float dotProduct(const Vector3D& v3fVector);

    Vector3D operator+(const Vector3D& v3fVector);

    Vector3D operator-(const Vector3D& v3fVector);
    Vector3D operator*(const float& fScalar);

    Vector3D operator/(const float& fScalar);

    float& operator[](int index);

    bool operator==(const Vector3D& v3fVector);
};

class Line
{
private:
    Vector3D v3Point;
    Vector3D v3Direction;

public:
    Line();

    Line(Vector3D point, Vector3D direction);

    Vector3D getOwnPoint() const;

    Vector3D getDirection() const;

    // Returns point for given parameter t
    Vector3D getPoint(float t);

    // Returns parameter t for given Point, returns 0 if doesn't lie on the line
    float getParameter(Vector3D point);
};

// ax + by + cz + d = 0
class Plane
{
private:
    float a, b, c, d;
    Vector3D normal;
public:
    Plane();

    Plane(Vector3D point, Vector3D normal);

    // Returns point of intersection and parameter t. If doesn't intersect returns 0,0,0 0
    std::pair<Vector3D, float> getLineIntersection(Line line) const;

};


#endif
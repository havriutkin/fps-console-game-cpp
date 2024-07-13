#include "geometry.h"


Vector3D::Vector3D() 
{
    this->fX = 0;
    this->fY = 0;
    this->fZ = 0;
    this->fLength = 0;
}

Vector3D::Vector3D(float x, float y, float z)
{
    this->fX = x;
    this->fY = y;
    this->fZ = z;
    this->fLength = sqrtf(x*x + y*y + z*z);
}

void Vector3D::normalize()
{
    if (this->fLength != 0)
    {
        this->fX /= this->fLength;
        this->fY /= this->fLength;
        this->fZ /= this->fLength;
        this->fLength = 1.0;
    }
}

float Vector3D::getLength() const 
{
    return this->fLength;
};

float Vector3D::dotProduct(const Vector3D& v3fVector)
{
    return this->fX * v3fVector.fX + this->fY * v3fVector.fY + this->fZ * v3fVector.fZ;
}

Vector3D Vector3D::operator+(const Vector3D& v3fVector)
{
    return Vector3D(this->fX + v3fVector.fX, this->fY + v3fVector.fY, this->fZ + v3fVector.fZ);
}

Vector3D Vector3D::operator-(const Vector3D& v3fVector)
{
    return Vector3D(this->fX - v3fVector.fX, this->fY - v3fVector.fY, this->fZ - v3fVector.fZ);
}

Vector3D Vector3D::operator*(const float& fScalar)
{
    return Vector3D(this->fX * fScalar, this->fY * fScalar, this->fZ * fScalar);
}

Vector3D Vector3D::operator/(const float& fScalar)
{
    return Vector3D(this->fX / fScalar, this->fY / fScalar, this->fZ / fScalar);
}

float& Vector3D::operator[](int index)
{
    if (index == 0)
    {
        return this->fX;
    }
    else if (index == 1)
    {
        return this->fY;
    }
    else if (index == 2)
    {
        return this->fZ;
    }
    else
    {
        return this->fX;
    }
}

bool Vector3D::operator==(const Vector3D& v3fVector)
{
    return this->fX == v3fVector.fX && this->fY == v3fVector.fY && this->fZ == v3fVector.fZ;
}


Line::Line()
{
    this->v3Point = Vector3D(0.0, 0.0, 0.0);
    this->v3Direction = Vector3D(0.0, 0.0, 0.0);
}

Line::Line(Vector3D point, Vector3D direction)
{
    this->v3Point = point;
    this->v3Direction = direction;
    this->v3Direction.normalize();
}

Vector3D Line::getOwnPoint() const { return this->v3Point; }

Vector3D Line::getDirection() const { return this->v3Direction; }

// Returns point for given parameter t
Vector3D Line::getPoint(float t = 0)
{
    Vector3D v3fResult;

    for (int i = 0; i < 3; i++)
    {
        v3fResult[i] = v3Point[i] + v3Direction[i] * t;
    }

    return v3fResult;
}

// Returns parameter t for given Point, returns 0 if doesn't lie on the line
float Line::getParameter(Vector3D point)
{
    Vector3D v3fDifference = point - v3Point;
    float fDotProduct = v3fDifference.dotProduct(v3Direction);

    if (fDotProduct == 0)
    {
        return 0;
    }
    else
    {
        return abs(fDotProduct / v3Direction.dotProduct(v3fDifference) );
    }
}



Plane::Plane()
{
    this->a = 0;
    this->b = 0;
    this->c = 0;
    this->d = 0;
}

Plane::Plane(Vector3D point, Vector3D normal)
{
    this->normal = normal;
    this->a = normal[0];
    this->b = normal[1];
    this->c = normal[2];
    this->d = -1 * (normal[0] * point[0] + normal[1] * point[1] + normal[2] * point[2]);
}

// Returns point of intersection and parameter t. If doesn't intersect returns 0,0,0 0
std::pair<Vector3D, float> Plane::getLineIntersection(Line line) const
{
    Vector3D lineDir = line.getDirection();
    float dotProd = lineDir.dotProduct(this->normal);

    // Line lies on plane or is parallel to plane
    if (std::abs(dotProd) < 1e-6) {
        return { Vector3D(0, 0, 0), 0 };
    }

    Vector3D linePoint = line.getOwnPoint();
    float numerator = linePoint.dotProduct(this->normal) + this->d;
    float denominator = lineDir.dotProduct(this->normal);

    float parameter = -numerator / denominator;
    Vector3D point = line.getPoint(parameter);

    return { point, parameter };
}



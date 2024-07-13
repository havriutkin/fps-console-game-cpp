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
    Vector3D() 
    {
        this->fX = 0;
        this->fY = 0;
        this->fZ = 0;
        this->fLength = 0;
    }

    Vector3D(float x, float y, float z)
    {
        this->fX = x;
        this->fY = y;
        this->fZ = z;
        this->fLength = sqrtf(x*x + y*y + z*z);
    }

    void normalize()
    {
        if (this->fLength != 0)
        {
            this->fX /= this->fLength;
            this->fY /= this->fLength;
            this->fZ /= this->fLength;
            this->fLength = 1.0;
        }
    }

    float dotProduct(const Vector3D& v3fVector)
    {
        return this->fX * v3fVector.fX + this->fY * v3fVector.fY + this->fZ * v3fVector.fZ;
    }

    Vector3D operator+(const Vector3D& v3fVector)
    {
        return Vector3D(this->fX + v3fVector.fX, this->fY + v3fVector.fY, this->fZ + v3fVector.fZ);
    }

    Vector3D operator-(const Vector3D& v3fVector)
    {
        return Vector3D(this->fX - v3fVector.fX, this->fY - v3fVector.fY, this->fZ - v3fVector.fZ);
    }

    Vector3D operator*(const float& fScalar)
    {
        return Vector3D(this->fX * fScalar, this->fY * fScalar, this->fZ * fScalar);
    }

    Vector3D operator/(const float& fScalar)
    {
        return Vector3D(this->fX / fScalar, this->fY / fScalar, this->fZ / fScalar);
    }

    float& operator[](int index)
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

    bool operator==(const Vector3D& v3fVector)
    {
        return this->fX == v3fVector.fX && this->fY == v3fVector.fY && this->fZ == v3fVector.fZ;
    }
};

class Line
{
private:
    Vector3D v3Point;
    Vector3D v3Direction;

public:
    Line()
    {
        this->v3Point = Vector3D(0.0, 0.0, 0.0);
        this->v3Direction = Vector3D(0.0, 0.0, 0.0);
    }

    Line(Vector3D point, Vector3D direction)
    {
        this->v3Point = point;
        this->v3Direction = direction;
        this->v3Direction.normalize();
    }

    // Returns point for given parameter t
    std::vector<float> getPoint(float t = 0)
    {
        std::vector<float> v3fResult(3);

        for (int i = 0; i < 3; i++)
        {
            v3fResult[i] = v3Point[i] + v3Direction[i] * t;
        }

        return v3fResult;
    }

    // Returns parameter t for given Point, returns 0 if doesn't lie on the line
    float getParameter(Vector3D point)
    {
        Vector3D v3fDifference = point - v3Point;
        float fDotProduct = v3fDifference.dotProduct(v3Direction);

        if (fDotProduct == 0)
        {
            return 0;
        }
        else
        {
            return abs(v3Direction.dotProduct(v3fDifference) / fDotProduct);
        }
    }
};

#endif
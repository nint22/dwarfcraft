/***************************************************************
 
 Magi3 Lite - Multiplatform OpenGL Interface
 Copyright 2010 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: Vector2.h
 Desc: Template class for a two-dimensional tupel.
 
***************************************************************/

// Inclusion guard
#ifndef __VECTOR2_H_
#define __VECTOR2_H_

#include <math.h>

// Vector2 class; A two-dimensional vector class
template <typename Type> class Vector2
{
public:

    // Constructor
    Vector2()
    {
        x = (Type)0;
        y = (Type)0;
    }

    // Constructor
    Vector2(Type x, Type y)
    {
        this->x = x;
        this->y = y;
    }

    // Copy constructor
    Vector2(const Vector2 &obj)
    {
        x = obj.x;
        y = obj.y;
    }

    // Deconstructor
    ~Vector2()
    {
        // Nothing to do...
    }

    // Overloaded '=' operator
    Vector2 operator= (const Vector2 &obj)
    {
        x = obj.x;
        y = obj.y;
        return *this;
    }

    // Overloaded '+' operator
    Vector2 operator+ (const Vector2 &obj)
    {
        return Vector2<Type>(x + obj.x, y + obj.y);
    }

    // Overloaded '-' operator
    Vector2 operator- (const Vector2 &obj)
    {
        return Vector2<Type>(x - obj.x, y - obj.y);
    }

    // Overloaded '*' operator
    Vector2 operator* (const Vector2 &obj)
    {
        return Vector2<Type>(x * obj.x, y * obj.y);
    }

    // Overloaded '/' operator
    Vector2 operator/ (const Vector2 &obj)
    {
        return Vector2<Type>(x / obj.x, y / obj.y);
    }

    // Overloaded '+=' operator
    Vector2& operator+= (const Vector2 &obj)
    {
        *this = *this + obj;
        return *this;
    }

    // Overloaded '-=' operator
    Vector2& operator-= (const Vector2 &obj)
    {
        *this = *this - obj;
        return *this;
    }

    // Overloaded '*=' operator
    Vector2& operator*= (const Vector2 &obj)
    {
        *this = *this * obj;
        return *this;
    }

    // Overloaded '/=' operator
    Vector2& operator/= (const Vector2 &obj)
    {
        *this = *this / obj;
        return *this;
    }
    
    // Overloaded '+=' operator
    Vector2& operator+= (const Type &obj)
    {
        this->x += obj;
        this->y += obj;
        return *this;
    }
    
    // Overloaded '-=' operator
    Vector2& operator-= (const Type &obj)
    {
        this->x -= obj;
        this->y -= obj;
        return *this;
    }
    
    // Overloaded '*=' operator
    Vector2& operator*= (const Type &obj)
    {
        this->x *= obj;
        this->y *= obj;
        return *this;
    }
    
    // Overloaded '/=' operator
    Vector2& operator/= (const Type &obj)
    {
        this->x /= obj;
        this->y /= obj;
        return *this;
    }
    
    // overloaded '==' operator
    bool operator== (const Vector2 &obj)
    {
        if(obj.x == x && obj.y == y)
            return true;
        else
            return false;
    }

    // Returns the length of this vector
    double GetLength()
    {
        double X = (double)x;
        double Y = (double)y;
        return sqrt(X*X + Y*Y);
    }
    
    // Normalizes this vector
    void Normalize()
    {
        Type length = GetLength();
        x /= length;
        y /= length;
    }

    // Public types
    Type x, y;

};

// Cross product function
template <typename Type> Type Vector2Cross(Vector2<Type> VectorU, Vector2<Type> VectorV)
{
    // Output scaler
    return VectorU.x * VectorV.y - VectorU.y * VectorV.x;
}

// Dot product function
template <typename Type> double Vector2Dot(Vector2<Type> VectorU, Vector2<Type> VectorV)
{
    // Output vector
    double Total = 0.0f;
    
    // Set the vector
    Total += double(VectorU.x) * double(VectorV.x);
    Total += double(VectorU.y) * double(VectorV.y);
    
    // Return vector
    return Total;
}

#endif

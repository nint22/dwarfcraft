/***************************************************************
 
 Magi3 Lite - Multiplatform OpenGL Interface
 Copyright 2010 Jeremy Bridon - See License.txt for info
 
 This source file is developed and maintained by:
 + Jeremy Bridon jbridon@cores2.com
 
 File: Vector3.h
 Desc: Template class for a three-dimensional tupel.
 
***************************************************************/

// Inclusion guard
#ifndef __VECTOR3_H_
#define __VECTOR3_H_

#include <math.h>

// Vector3 class; A three-dimensional vector class
template <typename Type> class Vector3
{
public:
    
    // Constructor
    Vector3()
    {
        x = (Type)0;
        y = (Type)0;
        z = (Type)0;
    }
    
    // Constructor
    Vector3(Type x, Type y, Type z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    
    // Copy constructor
    Vector3(const Vector3 &obj)
    {
        x = obj.x;
        y = obj.y;
        z = obj.z;
    }
    
    // Deconstructor
    ~Vector3()
    {
        // Nothing to do...
    }
    
    // Overloaded '=' operator
    Vector3 operator= (const Vector3 &obj)
    {
        x = obj.x;
        y = obj.y;
        z = obj.z;
        return *this;
    }
    
    /*** Scalars ***/
    
    // Overloaded '+' operator
    Vector3 operator+ (const Type &obj)
    {
        return Vector3<Type>(x + obj, y + obj, z + obj);
    }
    
    // Overloaded '-' operator
    Vector3 operator- (const Type &obj)
    {
        return Vector3<Type>(x - obj, y - obj, z - obj);
    }
    
    // Overloaded '*' operator
    Vector3 operator* (const Type &obj)
    {
        return Vector3<Type>(x * obj, y * obj, z * obj);
    }
    
    // Overloaded '/' operator
    Vector3 operator/ (const Type &obj)
    {
        return Vector3<Type>(x / obj, y / obj, z / obj);
    }
    
    // Overloaded '+=' operator
    Vector3& operator+= (const Type &obj)
    {
        *this = *this + Vector3<Type>(obj, obj, obj);
        return *this;
    }
    
    // Overloaded '-=' operator
    Vector3& operator-= (const Type &obj)
    {
        *this = *this - Vector3<Type>(obj, obj, obj);
        return *this;
    }
    
    // Overloaded '*=' operator
    Vector3& operator*= (const Type &obj)
    {
        *this = *this * Vector3<Type>(obj, obj, obj);
        return *this;
    }
    
    // Overloaded '/=' operator
    Vector3& operator/= (const Type &obj)
    {
        *this = *this / Vector3<Type>(obj, obj, obj);
        return *this;
    }    
    
    /*** Non-scalars ***/
    
    // Overloaded '+' operator
    Vector3 operator+ (const Vector3 &obj)
    {
        return Vector3<Type>(x + obj.x, y + obj.y, z + obj.z);
    }
    
    // Overloaded '-' operator
    Vector3 operator- (const Vector3 &obj)
    {
        return Vector3<Type>(x - obj.x, y - obj.y, z - obj.z);
    }

    // Overloaded '-' operator for unary purposes:
    friend Vector3 operator- (const Vector3 &obj)
    {
        return Vector3<Type>(-obj.x, -obj.y, -obj.z);
    }
    
    // Overloaded '*' operator
    Vector3 operator* (const Vector3 &obj)
    {
        return Vector3<Type>(x * obj.x, y * obj.y, z * obj.z);
    }
    
    // Overloaded '/' operator
    Vector3 operator/ (const Vector3 &obj)
    {
        return Vector3<Type>(x / obj.x, y / obj.y, z / obj.z);
    }

    // Overloaded '+=' operator
    Vector3& operator+= (const Vector3 &obj)
    {
        *this = *this + obj;
        return *this;
    }
    
    // Overloaded '-=' operator
    Vector3& operator-= (const Vector3 &obj)
    {
        *this = *this - obj;
        return *this;
    }
    
    // Overloaded '*=' operator
    Vector3& operator*= (const Vector3 &obj)
    {
        *this = *this * obj;
        return *this;
    }
    
    // Overloaded '/=' operator
    Vector3& operator/= (const Vector3 &obj)
    {
        *this = *this / obj;
        return *this;
    }
    
    // overloaded '==' operator
    bool operator== (const Vector3 &obj)
    {
        if(obj.x == x && obj.y == y && obj.z == z)
            return true;
        else
            return false;
    }
    
    // overloaded '!=' operator
    bool operator!= (const Vector3 &obj)
    {
        if(obj.x != x || obj.y != y || obj.z != z)
            return true;
        else
            return false;
    }
    
    // Returns the length of this vector
    double GetLength()
    {
        double X = (double)x;
        double Y = (double)y;
        double Z = (double)z;
        return sqrt(X*X + Y*Y + Z*Z);
    }
    
    // Normalizes this vector
    void Normalize()
    {
        Type length = GetLength();
        x /= length;
        y /= length;
        z /= length;
    }
    
    // Public types
    Type x, y, z;
};

// Cross product function
template <typename Type> Vector3<Type> Vector3Cross(Vector3<Type> VectorU, Vector3<Type> VectorV)
{
    // Output vector
    Vector3<Type> OutVector;
    
    // Set the vector
    OutVector.x = (VectorU.y * VectorV.z - VectorU.z * VectorV.y);
    OutVector.y = (VectorU.z * VectorV.x - VectorU.x * VectorV.z);
    OutVector.z = (VectorU.x * VectorV.y - VectorU.y * VectorV.x);
    
    // Return vector
    return OutVector;
}

// Dot product function
template <typename Type> double Vector3Dot(Vector3<Type> VectorU, Vector3<Type> VectorV)
{
    // Output vector
    double Total = 0.0f;
    
    // Set the vector
    Total += double(VectorU.x) * double(VectorV.x);
    Total += double(VectorU.y) * double(VectorV.y);
    Total += double(VectorU.z) * double(VectorV.z);
    
    // Return vector
    return Total;
}

// Convert a vector3 from float to integer (rounding down)
inline Vector3<int> Vector3ftoi(Vector3<float> fV)
{
    return Vector3<int>(fV.x, fV.y, fV.z);
}

// Convert a vector3 from integer to float
inline Vector3<float> Vector3itof(Vector3<int> iV)
{
    return Vector3<float>(iV.x, iV.y, iV.z);
}

#endif

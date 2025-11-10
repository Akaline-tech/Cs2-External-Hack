#pragma once
#include <directxmath.h>
#include <Windows.h>
#include <iostream>

#define PI 3.141592653f
#define Deg2Rad(Degree) ((Degree) * (PI / 180.f))
#define Rad2Deg(Radius) ((Radius) * (180.f / PI))

struct Vector2 {
    float x, y;

    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float x, float y) : x(x), y(y) {}

    Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }

    Vector2 operator-(const Vector2& other) const {
        return Vector2(x - other.x, y - other.y);
    }

    Vector2 operator*(float scalar) const {
        return Vector2(x * scalar, y * scalar);
    }

    float Dot(const Vector2& other) const {
        return x * other.x + y * other.y;
    }

    float Length() const {
        return std::sqrt(x * x + y * y);
    }

    Vector2 Normalized() const {
        float len = Length();
        if (len > 0.0f) {
            return Vector2(x / len, y / len);
        }
        return *this;
    }
};

struct IntVector2 {
    int x, y;
};

struct Vector3
{
    float x, y, z;

    // 默认构造函数
    Vector3() : x(0), y(0), z(0) {}

    // 带参数的构造函数
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3 operator+(Vector3 d);
    Vector3 operator-(Vector3 d);
    Vector3 operator*(Vector3 d);
    Vector3 operator*(float d);
    Vector3& operator-=(Vector3 d);
    Vector3& operator+=(Vector3 d);

    // Lerp 方法声明
    Vector3 Lerp(const Vector3& target, float t) const;

    Vector3 Normalized() const;
    float Dot(const Vector3& other) const;
    float Length() const;
    float Length2DSqr() const;
    float Length2D() const;
    bool IsVectorEmpty() const;

    Vector3 AnglesToVectors(Vector3* pForward, Vector3* pRight = nullptr, Vector3* pUp = nullptr) const;

};

struct Vector4
{
    float x, y, w, h;

    Vector4 operator+(Vector4 d);
    Vector4 operator-(Vector4 d);
    Vector4 operator*(Vector4 d);
    Vector4 operator*(float d);
};


#pragma once
#include <math.h>
#include <stdint.h>
#include <sstream>

#ifndef PI
#define PI 3.14159265f
#endif
constexpr inline float Sqrt(const float& x) noexcept {
    typedef union u { float f; uint32_t i; } u;
    u number = { x };
    number.i = 0x5f3759df - (number.i >> 1);
    number.f *= 1.5f - (x * 0.5f * number.f * number.f);
    number.f *= 1.5f - (x * 0.5f * number.f * number.f);
    return 1 / number.f;
}
template<class T>
struct Vector2D {
    T x, y;
    Vector2D() = default;
    Vector2D(T xAxis, T yAxis) { x = xAxis; y = yAxis; }
    void operator=(const Vector2D<T>& vec) { x = vec.x; y = vec.y;}
    friend void operator-=(Vector2D<T>& vec1, const Vector2D<T>& vec2) { vec1.x -= vec2.x; vec1.y -= vec2.y;}
    friend void operator+=(Vector2D<T>& vec1, const Vector2D<T>& vec2) { vec1.x += vec2.x; vec1.y += vec2.y; }
    friend void operator*=(Vector2D<T>& vec, const T& k) {vec.x *= k; vec.y *= k; }
    friend bool operator==(const Vector2D<T>& vec1, const Vector2D<T>& vec2) {return vec1.x == vec2.x && vec1.y == vec2.y;}
    friend bool operator!=(const Vector2D<T>& vec1, const Vector2D<T>& vec2) {return vec1.x != vec2.y || vec1.y != vec2.y; }
    friend Vector2D<T> operator/(const Vector2D<T>& vec1, const Vector2D<T>& vec2) {return Vector2D<T>(vec1.x/vec2.x, vec1.y/vec2.y);}
    Vector2D<T> operator*(T k) { return { x * k, y * k };}
    Vector2D<T> operator+(Vector2D<T> vec) {return { x + vec.x, y + vec.y };}
    Vector2D<T> operator-(Vector2D<T> vec) {return {x - vec.x, y - vec.y}; }
    T GetLength() { return Sqrt(x * x + y * y); }
    T GetLengthSq() { return x * x + y * y;}
    T GetDistance(Vector2D<T> point) { return Sqrt((x - point.x) * (x - point.x) + (y - point.y) * (y - point.y));}
    T GetDot(Vector2D<T> vec) {return x * vec.x + y * vec.y;}
    Vector2D<T> Normalize() {return { x /= GetLength(), y /= GetLength() };}
    Vector2D<T> lerp(const Vector2D<T>& vec, T f) {return { Lerp<T>(x, vec.x, f),  Lerp<T>(y, vec.y, f) };}
    std::string toString() {return std::to_string(x) + " " + std::to_string(y);}
};
typedef Vector2D<float> Vec2f;
typedef Vector2D<int32_t> Vec2i;
typedef Vector2D<double> Vec2d;
template <class T>
struct Vector3D {
    T x, y, z;
    Vector3D() = default;
    Vector3D(T xAxis, T yAxis, T zAxis) { x = xAxis; y = yAxis; z = zAxis; }
    Vector3D(Vector2D<T> vec, T zAxis) { x = vec.x; y = vec.y; z = zAxis; }
    Vector2D<T> xy() { return Vector2D<T>(x, y); }
    void operator=(const Vector3D<T>& vec) { x = vec.x; y = vec.y; z = vec.z; }
    friend void operator-=(Vector3D<T>& vec1, const Vector3D<T>& vec2) { vec1.x -= vec2.x; vec1.y -= vec2.y; vec1.z -= vec2.z; }
    friend void operator+=(Vector3D<T>& vec1, const Vector3D<T>& vec2) { vec1.x += vec2.x; vec1.y += vec2.y; vec1.z += vec2.z; }
    friend void operator*=(Vector3D<T>& vec, const T& k) { vec.x *= k; vec.y *= k; vec.z *= k; }
    inline Vector3D<T> operator*(T k) { return { x * k, y * k, z * k }; }
    inline T GetLength() {return Sqrt(x * x + y * y + z * z);}
    inline T GetLengthSq() {return x * x + y * y + z * z; }
    inline T GetDistance(Vector3D<T> point) {return Sqrt((x - point.x) * (x - point.x) + (y - point.y) * (y - point.y) + (z - point.z) * (z - point.z));}
    inline T GetDot(Vector3D<T> vec) { return x * vec.x + y * vec.y + z * vec.z; }
    Vector3D<T> operator-(Vector3D<T> vec) {return { x - vec.x, y - vec.y, z - vec.z };}
    void Normalize() {x /= GetLength(); y /= GetLength(); z /= GetLength();}
};
typedef Vector3D<float> Vec3f;
typedef Vector3D<int32_t> Vec3i;
typedef Vector3D<double> Vec3d;
static Vec2f F3ToF2(Vec3f vec3) {
    return { vec3.x, vec3.y };
}
template<class T>
inline constexpr T Lerp(T a, T b, T rate) {
    return a + (b - a) * rate;
}
static Vec2f toVector(float angle) {
    Vec2f ret;
    ret.x = cos(angle);
    ret.y = sin(angle);
    return ret;
}
inline float GetAngle(Vec2f vec1, Vec2f vec2) {
    return atan2(vec1.y - vec2.y, vec1.x - vec2.x);
}
template <class T>
const inline T Min(const T x, const T y) noexcept {
    return (x < y) ? x : y;
}
template <class T>
const inline T Max(const T x, const T y) noexcept {
    return (x > y) ? x : y;
}
template<class T>
const inline T Clamp(const T& x, const T& nMin, const T& nMax) noexcept {
    return Min(Max(x, nMin), nMax);
}
template <class T>
const inline T Smoothstep(const T& nRight, const T& nLeft, T x) noexcept {
    x = Clamp((x - nRight) / (nLeft - nRight), 0.0f, 1.0f);
    return (x * x * (3.0f - 2.0f * x));
}
template <class T>
constexpr inline int32_t Floor(const T x) noexcept {
    return (int32_t)x;
}
template <class T>
constexpr inline int32_t Round(const T x) noexcept {
    T temp = x - Floor(x);
    return Floor(x) + ((temp < abs(temp - 1)) ? 0 : 1);
}
template <class T>
const inline T Abs(const T x) noexcept {
    return (x < 0) ? (x * -1) : x;
}
template <class T, class U>
constexpr inline auto grandTotal(const T a, const U x)
{
    auto result = 1.0;
    for (int i = 1; i <= a; i++) {
        result *= x / i;
    }
    return result;
}
template <class T>
constexpr inline T Cos(const T x)
{
    T result = 1;
    for (int i = 2; i < 50; i++) {
        if (i % 4 == 0) {
            result += grandTotal(i, x);
        }
        if (i % 4 == 2) {
            result -= grandTotal(i, x);
        }
    }
    return result;
}
template <class T>
constexpr inline T Sin(const T x)
{
    return cos(3.14159265359 / 2 - x);
}
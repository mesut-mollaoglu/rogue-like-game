#pragma once
#include <math.h>
#include <stdint.h>
#include <sstream>
#if defined min
#undef min
#endif
#if defined max
#undef max
#endif 
#if defined sqrt
#undef sqrt
#endif 
#ifndef PI
#define PI 3.14159265f
#endif

class Math {
public:
    template<class T>
    struct Vector2D {
        T x;
        T y;
        Vector2D() = default;
        Vector2D(T xAxis, T yAxis) {
            x = xAxis;
            y = yAxis;
        }
        void operator=(Vector2D<T> vec) {
            x = vec.x;
            y = vec.y;
        }
        void operator-=(Vector2D<T> vec) {
            x -= vec.x;
            y -= vec.y;
        }
        void operator+=(Vector2D<T> vec) {
            x += vec.x;
            y += vec.y;
        }
        void operator*=(T k) {
            x *= k;
            y *= k;
        }
        Vector2D<T> operator*(T k) {
            Vector2D<T> ret;
            ret.x = x * k;
            ret.y = y * k;
            return ret;
        }
        Vector2D<T> operator+(Vector2D<T> vec) {
            Vector2D<T> ret;
            ret.x = x + vec.x;
            ret.y = y + vec.y;
            return ret;
        }
        Vector2D<T> operator-(Vector2D<T> vec) {
            Vector2D<T> ret;
            ret.x = x - vec.x;
            ret.y = y - vec.y;
            return ret;
        }
        T GetLength() {
            return Math::sqrt(x * x + y * y);
        }
        T GetLengthSq() {
            return x * x + y * y;
        }
        T GetDistance(Vector2D<T> point) {
            return Math::sqrt((x - point.x) * (x - point.x) + (y - point.y) * (y - point.y));
        }
        T GetDot(Vector2D<T> vec) {
            return x * vec.x + y * vec.y;
        }
        void Normalize() {
            x /= GetLength();
            y /= GetLength();
        }
        Vector2D<T> lerp(const Vector2D<T>& vec, T f)
        {
            Vector2D<T> ret;
            ret.x = Math::Lerp<T>(ret.x, vec.x, f);
            ret.y = Math::Lerp<T>(ret.y, vec.y, f);
            return ret;
        }
        std::string toString() {
            return std::to_string(x) + " " + std::to_string(y);
        }
        static Vector2D<T> toVector(const std::string& data) {
            Math::Vector2D<T> ret;
            std::stringstream ss(data);
            std::string m_x, m_y;
            ss >> m_x;
            ss >> m_y;
            ret.x = static_cast<T>(atoi(m_x.c_str()));
            ret.y = static_cast<T>(atoi(m_y.c_str()));
            return ret;
        }
    };
    typedef Vector2D<float> float2;
    typedef Vector2D<int32_t> int2;
    typedef Vector2D<double> double2;
    template <class T>
    struct Vector3D {
        T x;
        T y;
        T z;
        Vector3D() = default;
        Vector3D(T xAxis, T yAxis, T zAxis) {
            x = xAxis;
            y = yAxis;
            z = zAxis;
        }
        Vector3D(Vector2D<T> vec, T zAxis) {
            x = vec.x;
            y = vec.y;
            z = zAxis;
        }
        Vector2D<T> xy() {
            return Vector2D<T>(x, y);
        }
        void operator=(Vector3D<T> vec) {
            x = vec.x;
            y = vec.y;
            z = vec.z;
        }
        void operator-=(Vector3D<T> vec) {
            x -= vec.x;
            y -= vec.y;
            z -= vec.z;
        }
        void operator+=(Vector3D<T> vec) {
            x += vec.x;
            y += vec.y;
            z += vec.z;
        }
        void operator*=(T k) {
            x *= k;
            y *= k;
            z *= k;
        }
        Vector3D<T> operator*(T k) {
            Vector3D<T> ret;
            ret.x = x * k;
            ret.y = y * k;
            ret.z = z * k;
            return ret;
        }
        T GetLength() {
            return Math::sqrt(x * x + y * y + z * z);
        }
        T GetLengthSq() {
            return x * x + y * y + z * z;
        }
        T GetDistance(Vector3D<T> point) {
            return Math::sqrt((x - point.x) * (x - point.x) + (y - point.y) * (y - point.y) + (z - point.z) * (z - point.z));
        }
        T GetDot(Vector3D<T> vec) {
            return x * vec.x + y * vec.y + z * vec.z;
        }
        Vector3D<T> operator-(Vector3D<T> vec) {
            Vector3D<T> ret;
            ret.x = x - vec.x;
            ret.y = y - vec.y;
            ret.z = z - vec.z;
            return ret;
        }
        void Normalize() {
            x /= GetLength();
            y /= GetLength();
            z /= GetLength();
        }
    };
    typedef Vector3D<float> float3;
    typedef Vector3D<int32_t> int3;
    typedef Vector3D<double> double3;
    static Math::float2 F3ToF2(Math::float3 vec3) {
        return {vec3.x, vec3.y};
    }
    template<class T>
    static constexpr T Lerp(T a, T b, T rate) {
        return a + (b - a) * rate;
    }
    static Math::float2 toVector(float angle) {
        Math::float2 ret;
        ret.x = cos(angle);
        ret.y = sin(angle);
        return ret;
    }
    static float GetAngle(Math::float2 vec1, Math::float2 vec2) {
        return atan2(vec1.y - vec2.y, vec1.x - vec2.x);
    }
    template<class R, class L>
    const static auto& smoothstep(const R& firstEdge, const L& secondEdge, decltype(firstEdge + secondEdge) x) noexcept{
        x = clamp((x - firstEdge) / (secondEdge - firstEdge), 0.0f, 1.0f);
        return static_cast<decltype(x)>(x * x * (3.0f - 2.0f * x));
    }
    template<class T>
    const static T& clamp(const T& x, const T& minValue, const T& maxValue) noexcept{
        return static_cast<T>(min(max(x, minValue), maxValue));
    }
    template <class T, class U>
    const static auto& lerp(const T& x, const U& y, const decltype(x+y)& rate) noexcept {
        return std::forward<decltype(rate)>(smoothstep(x, y, rate));
    }
    template <class T, class U>
    const static auto& min(const T& x, const U& y) noexcept {
        typedef decltype(x + y) type;
        return (static_cast<type>(x) < static_cast<type>(y)) ? static_cast<T>(x) : static_cast<U>(y);
    }
    template <class T, class U>
    const static auto& max(const T& x, const U& y) noexcept {
        typedef decltype(x + y) type;
        return (static_cast<type>(x) > static_cast<type>(y)) ? static_cast<T>(x) : static_cast<U>(y);
    }
    template <class T>
    constexpr static unsigned floor(const T& x) noexcept {
        return (int)x;
    }
    template <class T>
    constexpr static unsigned round(const T& x) noexcept {
        T temp = std::forward<T>(x - floor(x));
        return floor(x) + ((temp < abs(temp - 1)) ? 0 : 1);
    }
    template <class T>
    const static T& abs(const T& x) noexcept {
        return (x < 0) ? (x * -1) : x;
    }
    constexpr static float sqrt(const float& x) noexcept {
        typedef union u{ float f; uint32_t i; } u;
        u number = { x };
        number.i = 0x5f3759df - (number.i >> 1);
        number.f *= 1.5f - (x * 0.5f * number.f * number.f);
        number.f *= 1.5f - (x * 0.5f * number.f * number.f);
        return 1 / number.f;
    }
};
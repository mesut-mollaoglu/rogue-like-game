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
    typedef struct float2 {
        float x;
        float y;
        float2() = default;
        float2(float xAxis, float yAxis) {
            x = xAxis;
            y = yAxis;
        }
        void operator=(float2 vec) {
            x = vec.x;
            y = vec.y;
        }
        void operator-=(float2 vec) {
            x -= vec.x;
            y -= vec.y;
        }
        void operator+=(float2 vec) {
            x += vec.x;
            y += vec.y;
        }
        void operator*=(float k) {
            x *= k;
            y *= k;
        }
        float2 operator*(float k) {
            float2 ret;
            ret.x = x * k;
            ret.y = y * k;
            return ret;
        }
        float2 operator+(float2 vec) {
            float2 ret;
            ret.x = x + vec.x;
            ret.y = y + vec.y;
            return ret;
        }
        float2 operator-(float2 vec) {
            float2 ret;
            ret.x = x - vec.x;
            ret.y = y - vec.y;
            return ret;
        }
        float GetLength() {
            return Math::sqrt(x * x + y * y);
        }
        float GetLengthSq() {
            return x * x + y * y;
        }
        float GetDistance(float2 point) {
            return Math::sqrt((x - point.x) * (x - point.x) + (y - point.y) * (y - point.y));
        }
        float GetDot(float2 vec) {
            return x * vec.x + y * vec.y;
        }
        void Normalize() {
            x /= GetLength();
            y /= GetLength();
        }
        float2 lerp(const float2& vec, float f)
        {
            float2 ret;
            ret.x = Math::Lerp<float>(ret.x, vec.x, f);
            ret.y = Math::Lerp<float>(ret.y, vec.y, f);
            return ret;
        }
        std::string toString() {
            return std::to_string(x) + " " + std::to_string(y);
        }
        static float2 toFloat(const std::string& data) {
            Math::float2 ret;
            std::stringstream ss(data);
            std::string m_x, m_y;
            ss >> m_x;
            ss >> m_y;
            ret.x = atoi(m_x.c_str());
            ret.y = atoi(m_y.c_str());
            return ret;
        }
        static const float2 up;
        static const float2 down;
        static const float2 right;
        static const float2 left;
    }float2;
    typedef struct float3 {
        float x;
        float y;
        float z;
        float3() = default;
        float3(float xAxis, float yAxis, float zAxis) {
            x = xAxis;
            y = yAxis;
            z = zAxis;
        }
        float3(float2 vec, float zAxis) {
            x = vec.x;
            y = vec.y;
            z = zAxis;
        }
        float2 xy() {
            return float2(x, y);
        }
        void operator=(float3 vec) {
            x = vec.x;
            y = vec.y;
            z = vec.z;
        }
        void operator-=(float3 vec) {
            x -= vec.x;
            y -= vec.y;
            z -= vec.z;
        }
        void operator+=(float3 vec) {
            x += vec.x;
            y += vec.y;
            z += vec.z;
        }
        void operator*=(float k) {
            x *= k;
            y *= k;
            z *= k;
        }
        float3 operator*(float k) {
            float3 ret;
            ret.x = x * k;
            ret.y = y * k;
            ret.z = z * k;
            return ret;
        }
        float GetLength() {
            return Math::sqrt(x * x + y * y + z * z);
        }
        float GetLengthSq() {
            return x * x + y * y + z * z;
        }
        float GetDistance(float3 point) {
            return Math::sqrt((x - point.x) * (x - point.x) + (y - point.y) * (y - point.y) + (z - point.z) * (z - point.z));
        }
        float GetDot(float3 vec) {
            return x * vec.x + y * vec.y + z * vec.z;
        }
        float3 operator-(float3 vec) {
            float3 ret;
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
    }float3;
    const Math::float2 up = Math::float2(0, 1);
    const Math::float2 down = Math::float2(0, -1);
    const Math::float2 right = Math::float2(1, 0);
    const Math::float2 left = Math::float2(-1, 0);
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
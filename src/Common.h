//
// Copyright (c) 2017-2020 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef COMMON_H_
#define COMMON_H_

#include "VmaUsage.h"

#ifdef _WIN32

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <array>
#include <type_traits>
#include <utility>
#include <chrono>
#include <string>
#include <exception>

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>

typedef std::chrono::high_resolution_clock::time_point time_point;
typedef std::chrono::high_resolution_clock::duration duration;

#ifdef _DEBUG
    #define TEST(expr) do { \
            if(!(expr)) { \
                assert(0 && #expr); \
            } \
        } while(0)
#else
    #define TEST(expr) do { \
            if(!(expr)) { \
                throw std::runtime_error("TEST FAILED: " #expr); \
            } \
        } while(0)
#endif

#define ERR_GUARD_VULKAN(expr) TEST((expr) >= 0)

extern VkInstance g_hVulkanInstance;
extern VkPhysicalDevice g_hPhysicalDevice;
extern VkDevice g_hDevice;
extern VkInstance g_hVulkanInstance;
extern VmaAllocator g_hAllocator;
extern bool VK_AMD_device_coherent_memory_enabled;

void SetAllocatorCreateInfo(VmaAllocatorCreateInfo& outInfo);

inline float ToFloatSeconds(duration d)
{
    return std::chrono::duration_cast<std::chrono::duration<float>>(d).count();
}

template <typename T>
inline T ceil_div(T x, T y)
{
    return (x+y-1) / y;
}
template <typename T>
inline T round_div(T x, T y)
{
    return (x+y/(T)2) / y;
}

template <typename T>
static inline T align_up(T val, T align)
{
    return (val + align - 1) / align * align;
}

static const float PI = 3.14159265358979323846264338327950288419716939937510582f;

template<typename MainT, typename NewT>
inline void PnextChainPushFront(MainT* mainStruct, NewT* newStruct)
{
    newStruct->pNext = mainStruct->pNext;
    mainStruct->pNext = newStruct;
}
template<typename MainT, typename NewT>
inline void PnextChainPushBack(MainT* mainStruct, NewT* newStruct)
{
    struct VkAnyStruct
    {
        VkStructureType sType;
        void* pNext;
    };
    VkAnyStruct* lastStruct = (VkAnyStruct*)mainStruct;
    while(lastStruct->pNext != nullptr)
    {
        lastStruct = (VkAnyStruct*)lastStruct->pNext;
    }
    newStruct->pNext = nullptr;
    lastStruct->pNext = newStruct;
}

struct vec3
{
    float x, y, z;

    vec3() { }
    vec3(float x, float y, float z) : x(x), y(y), z(z) { }

    float& operator[](uint32_t index) { return *(&x + index); }
    const float& operator[](uint32_t index) const { return *(&x + index); }

    vec3 operator+(const vec3& rhs) const { return vec3(x + rhs.x, y + rhs.y, z + rhs.z); }
    vec3 operator-(const vec3& rhs) const { return vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
    vec3 operator*(float s) const { return vec3(x * s, y * s, z * s); }

    vec3 Normalized() const
    {
        return (*this) * (1.f / sqrt(x * x + y * y + z * z));
    }
};

inline float Dot(const vec3& lhs, const vec3& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}
inline vec3 Cross(const vec3& lhs, const vec3& rhs)
{
    return vec3(
        lhs.y * rhs.z - lhs.z * rhs.y,
	    lhs.z * rhs.x - lhs.x * rhs.z,
	    lhs.x * rhs.y - lhs.y * rhs.x);
}

struct vec4
{
    float x, y, z, w;

    vec4() { }
    vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) { }
    vec4(const vec3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) { }

    float& operator[](uint32_t index) { return *(&x + index); }
    const float& operator[](uint32_t index) const { return *(&x + index); }
};

struct mat4
{
    union
    {
        struct
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        } s;
        float m[4][4]; // [row][column]
    };

    mat4() { }

    mat4(
        float _11, float _12, float _13, float _14,
        float _21, float _22, float _23, float _24,
        float _31, float _32, float _33, float _34,
        float _41, float _42, float _43, float _44)
        : s {
            _11, _12, _13, _14,
            _21, _22, _23, _24,
            _31, _32, _33, _34,
            _41, _42, _43, _44
        }
    {
    }

    mat4(
        const vec4& row1,
        const vec4& row2,
        const vec4& row3,
        const vec4& row4)
        : s {
            row1.x, row1.y, row1.z, row1.w,
            row2.x, row2.y, row2.z, row2.w,
            row3.x, row3.y, row3.z, row3.w,
            row4.x, row4.y, row4.z, row4.w
        }
    {
    }

    mat4 operator*(const mat4 &rhs) const
    {
        return mat4(
            s._11 * rhs.s._11 + s._12 * rhs.s._21 + s._13 * rhs.s._31 + s._14 * rhs.s._41,
            s._11 * rhs.s._12 + s._12 * rhs.s._22 + s._13 * rhs.s._32 + s._14 * rhs.s._42,
            s._11 * rhs.s._13 + s._12 * rhs.s._23 + s._13 * rhs.s._33 + s._14 * rhs.s._43,
            s._11 * rhs.s._14 + s._12 * rhs.s._24 + s._13 * rhs.s._34 + s._14 * rhs.s._44,

            s._21 * rhs.s._11 + s._22 * rhs.s._21 + s._23 * rhs.s._31 + s._24 * rhs.s._41,
            s._21 * rhs.s._12 + s._22 * rhs.s._22 + s._23 * rhs.s._32 + s._24 * rhs.s._42,
            s._21 * rhs.s._13 + s._22 * rhs.s._23 + s._23 * rhs.s._33 + s._24 * rhs.s._43,
            s._21 * rhs.s._14 + s._22 * rhs.s._24 + s._23 * rhs.s._34 + s._24 * rhs.s._44,

            s._31 * rhs.s._11 + s._32 * rhs.s._21 + s._33 * rhs.s._31 + s._34 * rhs.s._41,
            s._31 * rhs.s._12 + s._32 * rhs.s._22 + s._33 * rhs.s._32 + s._34 * rhs.s._42,
            s._31 * rhs.s._13 + s._32 * rhs.s._23 + s._33 * rhs.s._33 + s._34 * rhs.s._43,
            s._31 * rhs.s._14 + s._32 * rhs.s._24 + s._33 * rhs.s._34 + s._34 * rhs.s._44,

            s._41 * rhs.s._11 + s._42 * rhs.s._21 + s._43 * rhs.s._31 + s._44 * rhs.s._41,
            s._41 * rhs.s._12 + s._42 * rhs.s._22 + s._43 * rhs.s._32 + s._44 * rhs.s._42,
            s._41 * rhs.s._13 + s._42 * rhs.s._23 + s._43 * rhs.s._33 + s._44 * rhs.s._43,
            s._41 * rhs.s._14 + s._42 * rhs.s._24 + s._43 * rhs.s._34 + s._44 * rhs.s._44);
    }

    static mat4 RotationY(float angle)
    {
        const float s = sin(angle), c = cos(angle);
        return mat4(
            c,   0.f, -s,  0.f,
            0.f, 1.f, 0.f, 0.f,
            s,   0.f, c,   0.f,
            0.f, 0.f, 0.f, 1.f);
    }

    static mat4 Perspective(float fovY, float aspectRatio, float zNear, float zFar)
    {
        float yScale = 1.0f / tan(fovY * 0.5f);
        float xScale = yScale / aspectRatio;
        return mat4(
            xScale, 0.0f, 0.0f, 0.0f,
            0.0f, yScale, 0.0f, 0.0f,
            0.0f, 0.0f, zFar / (zFar - zNear), 1.0f,
            0.0f, 0.0f, -zNear * zFar / (zFar - zNear), 0.0f);
    }

    static mat4 LookAt(vec3 at, vec3 eye, vec3 up)
    {
        vec3 zAxis = (at - eye).Normalized();
        vec3 xAxis = Cross(up, zAxis).Normalized();
        vec3 yAxis = Cross(zAxis, xAxis);
        return mat4(
            xAxis.x, yAxis.x, zAxis.x, 0.0f,
            xAxis.y, yAxis.y, zAxis.y, 0.0f,
            xAxis.z, yAxis.z, zAxis.z, 0.0f,
            -Dot(xAxis, eye), -Dot(yAxis, eye), -Dot(zAxis, eye), 1.0f);
    }
};

class RandomNumberGenerator
{
public:
    RandomNumberGenerator() : m_Value{GetTickCount()} {}
    RandomNumberGenerator(uint32_t seed) : m_Value{seed} { }
    void Seed(uint32_t seed) { m_Value = seed; }
    uint32_t Generate() { return GenerateFast() ^ (GenerateFast() >> 7); }

private:
    uint32_t m_Value;
    uint32_t GenerateFast() { return m_Value = (m_Value * 196314165 + 907633515); }
};

// Wrapper for RandomNumberGenerator compatible with STL "UniformRandomNumberGenerator" idea.
struct MyUniformRandomNumberGenerator
{
    typedef uint32_t result_type;
    MyUniformRandomNumberGenerator(RandomNumberGenerator& gen) : m_Gen(gen) { }
    static uint32_t min() { return 0; }
    static uint32_t max() { return UINT32_MAX; }
    uint32_t operator()() { return m_Gen.Generate(); }

private:
    RandomNumberGenerator& m_Gen;
};

void ReadFile(std::vector<char>& out, const char* fileName);

enum class CONSOLE_COLOR
{
    INFO,
    NORMAL,
    WARNING,
    ERROR_,
    COUNT
};

void SetConsoleColor(CONSOLE_COLOR color);

void PrintMessage(CONSOLE_COLOR color, const char* msg);
void PrintMessage(CONSOLE_COLOR color, const wchar_t* msg);

inline void Print(const char* msg) { PrintMessage(CONSOLE_COLOR::NORMAL, msg); }
inline void Print(const wchar_t* msg) { PrintMessage(CONSOLE_COLOR::NORMAL, msg); }
inline void PrintWarning(const char* msg) { PrintMessage(CONSOLE_COLOR::WARNING, msg); }
inline void PrintWarning(const wchar_t* msg) { PrintMessage(CONSOLE_COLOR::WARNING, msg); }
inline void PrintError(const char* msg) { PrintMessage(CONSOLE_COLOR::ERROR_, msg); }
inline void PrintError(const wchar_t* msg) { PrintMessage(CONSOLE_COLOR::ERROR_, msg); }

void PrintMessageV(CONSOLE_COLOR color, const char* format, va_list argList);
void PrintMessageV(CONSOLE_COLOR color, const wchar_t* format, va_list argList);
void PrintMessageF(CONSOLE_COLOR color, const char* format, ...);
void PrintMessageF(CONSOLE_COLOR color, const wchar_t* format, ...);
void PrintWarningF(const char* format, ...);
void PrintWarningF(const wchar_t* format, ...);
void PrintErrorF(const char* format, ...);
void PrintErrorF(const wchar_t* format, ...);

void SaveFile(const wchar_t* filePath, const void* data, size_t dataSize);

#endif // #ifdef _WIN32

#endif

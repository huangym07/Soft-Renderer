#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <limits>

// Vector
template <int N, typename T> struct Vec {
    std::array<T, N> data;
    const T &operator[](int index) const { return data[index]; }
    T &operator[](int index) { return data[index]; }
};

template <int N, typename T>
std::ostream &operator<<(std::ostream &os, const Vec<N, T> &vec) {
    for (int i = 0; i < N; ++i)
        os << vec[i] << " ";
    return os;
}

template <int N, typename T>
Vec<N, T> operator-(const Vec<N, T> &lhs, const Vec<N, T> &rhs) {
    Vec<N, T> result;
    for (int i = 0; i < N; ++i) {
        result[i] = lhs[i] - rhs[i];
    }
    return result;
}

// 点积
template <int N, typename T>
T operator*(const Vec<N, T> &lhs, const Vec<N, T> &rhs) {
    T result = 0;
    for (int i = 0; i < N; ++i) {
        result += lhs[i] * rhs[i];
    }
    return result;
}

template <typename T> struct Vec<3, T> {
    T x, y, z;
    const T &operator[](int index) const {
        return index ? (1 == index ? y : z) : x;
    }
    T &operator[](int index) { return index ? (1 == index ? y : z) : x; }
    // 叉积
    Vec<3, T> cross(const Vec<3, T> &rhs) const {
        return {y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z,
                x * rhs.y - y * rhs.x};
    }
};

using Vec3f = Vec<3, float>;
using Vec3i = Vec<3, int>;
using Vec2f = Vec<2, float>;

// 点积法求重心坐标
// P = A + beta * (B - A) + gamma * (C - A)
// 把上式 A 挪到左侧，可得
// P - A = beta * (B - A) + gamma * (C - A)
// 设向量 v0 = P - A, v1 = B - A, v2 = C - A
// 对上式左右两侧同点乘 (B - A), (C - A)，可得
// v0 * v1 = beta * v1 * v1 + gamma * v1 * v2
// v0 * v2 = beta * v1 * v2 + gamma * v2 * v2
// 解上面两式，可得
// 设 denominator = 1 / ((v1 * v1) * (v2 * v2) - (v1 * v2) * (v1 * v2))
// beta = ((v2 * v2) * (v0 * v1) - (v1 * v2) * (v0 * v2)) * denominator
// gamma = ((v1 * v1) * (v0 * v2) - (v1 * v2) * (v0 * v1) * denominator
// alpha = 1 - beta - gamma
// P = alpha * A + beta * B + gamma * C;
template <int N>
Vec<3, float>
barycentric_coordinates(const Vec<N, float> &P, const Vec<N, float> &A,
                        const Vec<N, float> &B, const Vec<N, float> &C) {
    auto v0 = P - A;
    auto v1 = B - A;
    auto v2 = C - A;

    auto dot11 = v1 * v1;
    auto dot22 = v2 * v2;
    auto dot12 = v1 * v2;
    auto dot01 = v0 * v1;
    auto dot02 = v0 * v2;

    auto denominator = dot11 * dot22 - dot12 * dot12;

    // 检查退化三角形
    constexpr float epsilon = 1e-6f;
    if (std::fabs(denominator) < epsilon) {
        // 返回 NAN 表示退化三角形，求重心坐标失败
        return {std::numeric_limits<float>::quiet_NaN(),
                std::numeric_limits<float>::quiet_NaN(),
                std::numeric_limits<float>::quiet_NaN()};
    }

    denominator = 1 / denominator;
    float beta = (dot22 * dot01 - dot12 * dot02) * denominator;
    float gamma = (dot11 * dot02 - dot12 * dot01) * denominator;

    return {1 - beta - gamma, beta, gamma};
}
#pragma once

#include <iostream>

template <int N, typename T> struct Vec {
    T data[N] = {};
    const T &operator[](int index) const { return data[index]; }
    T &operator[](int index) { return data[index]; }
};

template <int N, typename T>
std::ostream &operator<<(std::ostream &os, const Vec<N, T> &vec) {
    for (int i = 0; i < N; ++i)
        os << vec[i] << " ";
    return os;
}

template <typename T> struct Vec<3, T> {
    T x, y, z;
    const T &operator[](int index) const {
        return index ? (1 == index ? y : z) : x;
    }
    T &operator[](int index) { return index ? (1 == index ? y : z) : x; }
};
using Vec3f = Vec<3, float>;
#include "model.h"
#include "tga_image.h"
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <tuple>

constexpr int width = 800;
constexpr int height = 800;

constexpr TgaColor white = {255, 255, 255, 255};
constexpr TgaColor blue = {255, 0, 0, 255};
constexpr TgaColor green = {0, 255, 0, 255};
constexpr TgaColor red = {0, 0, 255, 255};

// 原始方案
// t = (x - ax) / (bx - ax)
// y = ay + t * (by - ay) = ay + (x - ax) * (by - ay) / (bx - ax)
// 递增 x, 每次 y 递增 (by - ay) / (bx - ax) 即可
// 优化 1
// 为了图像平滑, 对 y 进行四舍五入而不是直接截断来获得坐标
// 为了性能, 使用整数计算代替浮点数计算, 使用乘上分支值的乘法代替条件分支
// 使用浮点数 error 存储 y 小数部分来四舍五入, 使用整数 y 作为坐标
// error += std::abs(by - ay) / static_cast<float>(bx - ax)
// y += (by > ay ? : 1 : -1) * (error > 0.5)
// error -= 1
// 优化 2
// 为了性能, 进一步使用整数计算代替浮点数计算
// 设置整数 ierror = 2 * error * (bx - ax)
// ierror += 2 * std::abs(by - ay)
// y += (by > ay ? 1 : -1) * (ierror > bx - ax)
// ierror -= 2 * (bx - ax) * (ierror > bx - ax)
// 性能测试:
// 测试目的 对比 Bresenham 算法与浮点算法的性能
// 处理器	13th Gen Intel(R) Core(TM) i9-13900HX，2200 Mhz，24 个内核，32
// 个逻辑处理器 操作系统 Windows 11 编译环境 (MSYS2) GNU 15.1.0, -std=c++17 -O3
// 测试数据 1 << 24 条线段, 坐标范围在 [0, 64)
// 计时方式:
// auto start = std::chrono::high_resolution_clock::now();
// // 测试代码
// auto end = std::chrono::high_resolution_clock::now();
// auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end -
// start).count(); 测试结果 Bresenham's Line Draw Algorithm
// 比浮点数增量的四舍五入要快 100000 微秒
void line_draw(int ax, int ay, int bx, int by, TgaImage &frame_buffer,
               const TgaColor &color) {
    // std::cerr << __PRETTY_FUNCTION__ << ": " << ax << " " << ay << " " << bx
    // << " " << by << '\n';

    bool steep =
        std::abs(ax - bx) < std::abs(ay - by); // x and y are changed when true
    if (steep) {
        std::swap(ax, ay);
        std::swap(bx, by);
    }

    if (ax > bx) { // make it left to right
        std::swap(ax, bx);
        std::swap(ay, by);
    }

    int y = ay;
    int ierror = 0; // 2 * error * (bx - ax)
    for (int x = ax; x <= bx; ++x) {
        if (steep) {
            frame_buffer.set_pixel(y, x, color);
        } else {
            frame_buffer.set_pixel(x, y, color);
        }
        ierror += 2 * std::abs(by - ay);
        y += (by > ay ? 1 : -1) * (ierror > bx - ax);
        ierror -= 2 * (bx - ax) * (ierror > bx - ax);
    }
}

float linear_interpolate(float value, float old_min_value, float old_max_value,
                         float new_min_value, float new_max_value) {
    return new_min_value + (value - old_min_value) *
                               (new_max_value - new_min_value) /
                               (old_max_value - old_min_value);
}

std::tuple<int, int> viewport_trans(const Vec3f &point, const int width,
                                    const int height) {
    return {(point.x + 1.f) * width / 2, (point.y + 1.f) * height / 2};
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " Path/to/filename.obj\n";
        return 1;
    }

    TgaImage frame_buffer(width, height, TgaImage::RGB);

    Model model(argv[1]);

    for (int i = 0; i < model.num_faces(); ++i) {
        auto [ax, ay] = viewport_trans(model.vertex(i, 0), width, height);
        auto [bx, by] = viewport_trans(model.vertex(i, 1), width, height);
        auto [cx, cy] = viewport_trans(model.vertex(i, 2), width, height);

        line_draw(ax, ay, bx, by, frame_buffer, red);
        line_draw(bx, by, cx, cy, frame_buffer, red);
        line_draw(cx, cy, ax, ay, frame_buffer, red);

        frame_buffer.set_pixel(ax, ay, white);
        frame_buffer.set_pixel(bx, by, white);
        frame_buffer.set_pixel(cx, cy, white);
    }

    frame_buffer.write_tga_file("frame_buffer.tga");

    return 0;
}
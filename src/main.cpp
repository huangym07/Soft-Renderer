#include "model.h"
#include "tga_image.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <ctime>

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

// 视口变换
// NDC -> [0, width]x[0, height]x[0, 255]
// NDC z 的变换用于可视化深度
Vec3f viewport_trans(const Vec3f &point, const int width,
                                    const int height) {
    return {(point.x + 1.f) * (width - 1) / 2, (point.y + 1.f) * (height - 1) / 2, (point.z + 1.f) * 255.f / 2};
}

void triangle_rasterize(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3, TgaImage &frame_buffer, TgaImage &z_buffer, const TgaColor &color) {
    // 屏幕空间坐标
    auto [ax, ay, az] = viewport_trans(p1, width, height);
    auto [bx, by, bz] = viewport_trans(p2, width, height);
    auto [cx, cy, cz] = viewport_trans(p3, width, height);

#ifndef NDEBUG
    std::cerr << __PRETTY_FUNCTION__ << '\n';
    Vec3f v0{ax, ay, az};
    Vec3f v1{bx, by, bz};
    Vec3f v2{cx, cy, cz};
    std::cerr << v0 << '\n';
    std::cerr << v1 << '\n';
    std::cerr << v2 << '\n';
#endif

    // 背面剔除
    // 背面剔除应该使用世界坐标来做
    // 但是目前渲染条件为: 右手坐标系, 使用的模型局部坐标均在 [-1, 1]^3, 直接拿来当作 NDC 坐标
    // 右手坐标系, 如果使用的模型的局部坐标在 [-1, 1]^3, 那么将其直接拿来当作 NDC 坐标，这相当于自动进行了下面操作
    // 1. 不进行模型变换，局部坐标就是世界坐标
    // 2. 接着进行了相机在 z 轴某个能看清出模型全貌(就是和模型不重合)的位置, 
    // x, y, z 轴与世界坐标的 x, y, z 轴相同方向的视图变换
    // 3. 然后进行了选取合适的长方体进行正交投影变换得到 NDC, 
    // 并且这个合适的长方体使得模型各点的 NDC 坐标与模型局部坐标相同的正交投影变换。
    // 所以在目前相机看向 z 轴负方向且使用正交投影的特定条件下，
    // 视口变换后的屏幕空间背面剔除是可行的，结果与世界坐标剔除等价。
    // 这是因为正交投影保留了三维空间中 z 轴方向的朝向关系，
    // 屏幕空间的顶点顺序和法向量分量可直接用于背面判定。
    // 但需注意，当投影方式或观察方向改变时，仍需在三维坐标空间中执行标准背面剔除。

    // 当前使用的模型按照右手坐标系, 逆时针绕序为正面
    if (Vec3f{bx - ax, by - ay, 0}.cross({cx - ax, cy - ay, 0}).z < 0) return;

    // 包围盒
    int x_min = std::min(std::min(ax, bx), cx);
    int x_max = std::max(std::max(ax, bx), cx);
    x_max = std::min(x_max, frame_buffer.get_width() - 1); // x 坐标为 width 的点位于第 width - 1 列像素的右侧边界上
    int y_min = std::min(std::min(ay, by), cy);
    int y_max = std::max(std::max(ay, by), cy);
    y_max = std::min(y_max, frame_buffer.get_height() - 1); // 同上

    // 遍历包围盒内像素
    for (int x = x_min; x <= x_max; ++x) {
        for (int y = y_min; y <= y_max; ++y) {
            // 计算重心坐标
            auto [alpha, beta, gamma] = barycentric_coordinates(Vec2f{x + 0.5f, y + 0.5f}, Vec2f{ax, ay}, Vec2f{bx, by}, Vec2f{cx, cy});
            // 退化三角形，停止光栅化
            if (std::isnan(alpha)) return;
            if (beta >= 0 && gamma >= 0 && beta + gamma <= 1) {
                // 正交投影 可以使用屏幕空间的重心坐标插值 z
                std::uint8_t z = static_cast<std::uint8_t>(alpha * az + beta * bz + gamma * cz);
                if (z > z_buffer.get_pixel(x, y)[0]) {
                    z_buffer.set_pixel(x, y, {z});
                    frame_buffer.set_pixel(x, y, color);
                }
            }
        }
    }
}

void rasterize(const Model &model, TgaImage &frame_buffer, TgaImage &z_buffer) {
    std::srand(std::time({}));
    for (int i = 0; i < model.num_faces(); ++i) {
        TgaColor color;
        for (int j = 0; j < 3; ++j) color[j] = rand() % 255;
        triangle_rasterize(model.vertex(i, 0), model.vertex(i, 1), model.vertex(i, 2), frame_buffer, z_buffer, color);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " Path/to/filename.obj\n";
        return 1;
    }

    TgaImage frame_buffer(width, height, TgaImage::RGB);
    TgaImage z_buffer(width, height, TgaImage::GRAYSCALE);

    Model model(argv[1]);
    
    rasterize(model, frame_buffer, z_buffer);

    frame_buffer.write_tga_file("frame_buffer.tga");
    z_buffer.write_tga_file("z_buffer.tga");

    return 0;
}
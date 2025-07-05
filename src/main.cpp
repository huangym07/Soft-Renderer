#include <cstdlib>
#include <util/tga_image.h>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <limits>

constexpr TgaColor white = {255, 255, 255, 255};
constexpr TgaColor blue = {255, 0, 0, 255};
constexpr TgaColor green = {0,255, 0, 255};
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
// 处理器	13th Gen Intel(R) Core(TM) i9-13900HX，2200 Mhz，24 个内核，32 个逻辑处理器
// 操作系统 Windows 11
// 编译环境 (MSYS2) GNU 15.1.0, -std=c++17 -O3
// 测试数据 1 << 24 条线段, 坐标范围在 [0, 64)
// 计时方式: 
// auto start = std::chrono::high_resolution_clock::now();
// // 测试代码
// auto end = std::chrono::high_resolution_clock::now();
// auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
// 测试结果 Bresenham's Line Draw Algorithm 比浮点数增量的四舍五入要快 100000 微秒
void line_draw(int ax, int ay, int bx, int by, TgaImage &frame_buffer, const TgaColor &color) {
    // std::cerr << __PRETTY_FUNCTION__ << ": " << ax << " " << ay << " " << bx << " " << by << '\n';

    bool steep = std::abs(ax - bx) < std::abs(ay - by); // x and y are changed when true
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

template<typename T>
struct Point {
    T x = 0, y = 0;
};

float linear_interpolate(float x, float old_min_x, float old_max_x, float new_min_x, float new_max_x) {
    float ret = new_min_x + (x - old_min_x) * (new_max_x - new_min_x) / (old_max_x - old_min_x);

    // static int count = 1;

    // if (count <= 10) {
    //     std::string cord = count % 2 ? "x" : "y";
    //     std::cerr << __PRETTY_FUNCTION__ << " " << cord << ": " << x << " " << old_min_x << " " << old_max_x << " " << new_min_x << " " << new_max_x << " " << ret << '\n';
    //     count++;
    // }

    return ret;
}

// 读取 obj 模型文件
// 生成线框图
void obj_draw(const std::string &filename, TgaImage &frame_buffer, const TgaColor &color) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Failed to open " << filename << '\n';
        return;
    }

    std::string line;
    std::vector<Point<float>> pfvec;
    float max_x = std::numeric_limits<float>::min(), min_x = std::numeric_limits<float>::max();
    float max_y = max_x, min_y = min_x;
    while (std::getline(in, line)) {
        std::stringstream ss(line);

        std::string word;
        ss >> word;
        if (word == "v") {
            float x, y;
            ss >> x >> y;
            pfvec.push_back({x, y});

            // 计算坐标极差作为模型空间的宽高, 以便缩放成图像大小
            // 注意, 不要对 x, y 四舍五入取整后再计算极差
            // 因为模型空间和图像中坐标差距占极差的比例是相同的
            // 坐标差距可能很小, 比如是 0.001 级别的, 极差可能 0.01 级别
            // 坐标差距占极差的 1/10, 导致图像中坐标差距占图像的 1/10
            // 如果四舍五入取整后计算极差, 会放大极差, 比如极差放大成 2
            // 坐标差距占极差的 1/2000, 导致图像中坐标差距占图像的 1/2000
            // 这会使图像中所有点非常接近, 看不出模型的样子
            max_x = std::max(max_x, x);
            max_y = std::max(max_y, y);
            min_x = std::min(min_x, x);
            min_y = std::min(min_y, y);
        } else if(word == "f") {
            std::string segment;
            std::vector<int> indexs;
            while (ss >> segment) {
                std::stringstream seg_ss(segment);
                int index;
                seg_ss >> index;
                indexs.push_back(index);
            }
            for (int i = 0; i < 3; ++i) {
                int id1 = indexs[i] - 1, id2 = indexs[(i + 1) % 3] - 1;
                int ax = std::round(linear_interpolate(pfvec[id1].x, min_x, max_x, 0, frame_buffer.get_width() - 1));
                int ay = std::round(linear_interpolate(pfvec[id1].y, min_y, max_y, 0, frame_buffer.get_height() - 1));
                int bx = std::round(linear_interpolate(pfvec[id2].x, min_x, max_x, 0, frame_buffer.get_width() - 1));
                int by = std::round(linear_interpolate(pfvec[id2].y, min_y, max_y, 0, frame_buffer.get_height() - 1));
                line_draw(ax, ay, bx, by, frame_buffer, color);
            }
        }
    }

    std::cerr << "obj x: " << min_x << " " << max_x << " and y: " << min_y << " " << max_y << '\n';
}

int main(int argc, char* argv[]) {
    constexpr int width = 800;
    constexpr int height = 1000;

    TgaImage frame_buffer(width, height, TgaImage::RGB);

    obj_draw(argv[1], frame_buffer, red);

    frame_buffer.write_tga_file("frame_buffer.tga");

    return 0;
}
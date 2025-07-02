#include <util/tga_image.h>
#include <iostream>

constexpr TgaColor white = {255, 255, 255, 255};
constexpr TgaColor blue = {255, 0, 0, 255};
constexpr TgaColor green = {0,255, 0, 255};
constexpr TgaColor red = {0, 0, 255, 255};

int main() {
    constexpr int width = 64;
    constexpr int height = 64;

    TgaImage frame_buffer(width, height, TgaImage::RGB);

    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;
    int dx = 32, dy = 1;

    frame_buffer.set_pixel(ax, ay, white);
    frame_buffer.set_pixel(bx, by, white);
    frame_buffer.set_pixel(cx, cy, white);
    frame_buffer.set_pixel(dx, dy, red);

    frame_buffer.write_tga_file("frame_buffer.tga");

    return 0;
}
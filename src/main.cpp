#include <iostream>
#include <util/tga_image.h>

const TgaColor white{255, 255, 255, 255};
const TgaColor red{0, 0, 255, 255};

int main() {
    TgaImage tga_image(100, 100, TgaImage::RGB);

    tga_image.set_pixel(50, 40, red);
    tga_image.flip_vertically(); // 使原点在图像的左下角
    tga_image.write_tga_file("output.tga");

    return 0;
}
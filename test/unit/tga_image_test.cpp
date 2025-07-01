#include <util/tga_image.h>
#include <debug_tools.h>

void Test_TgaHeader_size() {
    TgaHeader tga_header;
    MY_ASSERT(sizeof(tga_header) == 18, "");
}

void Test_TgaImage_get_and_set() {
    int width = 7980, height = 4320;
    TgaImage tga_image{width, height, TgaImage::RGBA};
    MY_ASSERT(tga_image.get_width() == width, "");
    MY_ASSERT(tga_image.get_height() == height, "");

    int x = 5, y = 10;
    TgaColor tga_color = {255, 255, 255, 255, TgaImage::RGBA};
    tga_image.set_pixel(x, y, tga_color);

    MY_ASSERT(tga_image.get_pixel(x, y) == tga_color, "");

    // test failure
    x = width + 1, y = height + 1;
    MY_ASSERT(tga_image.set_pixel(x, y, tga_color) == false, "");
    MY_ASSERT(tga_image.get_pixel(x, y) == TgaColor(), "");
}

void Test_TgaImage_read_and_write() {
    std::string filename{"./tga_image_test_TgaImage_read_and_write.tga"};

    int width = 100, height = 100;
    TgaImage tga_image{width, height, TgaImage::RGB};
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            std::uint8_t tmp = static_cast<std::uint8_t>((i * width + j) % 256);
            tga_image.set_pixel(i, j, {tmp, tmp, tmp, tmp, TgaImage::RGB});
        }
    }
    MY_ASSERT(true == tga_image.write_tga_file(filename), "");

    TgaImage tga_image2;
    tga_image2.read_tga_file(filename);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            std::uint8_t tmp = static_cast<std::uint8_t>((i * width + j) % 256);
            TgaColor tmp_color = {tmp, tmp, tmp, tmp, TgaImage::RGB};
            MY_ASSERT(tga_image.get_pixel(i, j) == tmp_color, "");
        }
    }
}
int main() {
    Test_TgaHeader_size();
    Test_TgaImage_get_and_set();
    Test_TgaImage_read_and_write();
    return 0;
}
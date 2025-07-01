#include <util/tga_image.h>
#include <debug_tools.h>

void Test_TgaHeader_size() {
    TgaHeader tga_header;
    MY_ASSERT(sizeof(tga_header) == 18, "");
}

void Test_TgaImage_get_and_set() {
    int width = 7980, height = 4320, bytespp = 4;
    TgaImage tga_image{width, height, bytespp};
    MY_ASSERT(tga_image.get_width() == width, "");
    MY_ASSERT(tga_image.get_height() == height, "");
    MY_ASSERT(tga_image.get_bytes_per_pixel() == bytespp, "");

    int x = 5, y = 10;
    TgaColor tga_color{255, 255, 255, 255};
    tga_image.set_pixel(x, y, tga_color);

    MY_ASSERT(tga_image.get_pixel(x, y) == tga_color, "");

    uint8_t *data = tga_image.get_buffer();
    MY_ASSERT(0 == std::memcmp(data + (y * width + x) * bytespp, tga_color.raw, tga_color.bytespp), "");

    // test failure
    x = width + 1, y = height + 1;
    MY_ASSERT(tga_image.set_pixel(x, y, tga_color) == false, "");
    MY_ASSERT(tga_image.get_pixel(x, y) == TgaColor(), "");
}

bool operator==(const TgaImage &lhs, const TgaImage &rhs) {
    if (lhs.get_width() != rhs.get_width()) return false;
    if (lhs.get_height() != rhs.get_height()) return false;
    if (lhs.get_bytes_per_pixel() != rhs.get_bytes_per_pixel()) return false;
    return 0 == std::memcmp(lhs.get_buffer(), rhs.get_buffer(), lhs.get_width() * lhs.get_height() * lhs.get_bytes_per_pixel());
}
bool operator!=(const TgaImage &lhs, const TgaImage &rhs) {
    return !(lhs == rhs);
}

void Test_TgaImage_read_and_write() {
    std::string filename{"./tga_image_test_TgaImage_read_and_write.tga"};

    int width = 100, height = 100, bytespp = 3;
    TgaImage tga_image{width, height, bytespp};
    uint8_t *data = tga_image.get_buffer();
    for (int i = 0; i < width * height * bytespp; ++i) {
        data[i] = static_cast<uint8_t>(i % 256);
    }
    MY_ASSERT(true == tga_image.write_tga_file(filename), "");

    TgaImage tga_image2;
    tga_image2.read_tga_file(filename);
    MY_ASSERT(tga_image == tga_image2, "");
}
int main() {
    Test_TgaHeader_size();
    Test_TgaImage_get_and_set();
    Test_TgaImage_read_and_write();
    return 0;
}
#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

// tga 格式文件的头部
// 详细信息参考 https://en.wikipedia.org/wiki/Truevision_TGA#Header
// 为了和外部二进制交互，禁用内存对齐
#pragma pack(1)
struct TgaHeader {
    std::uint8_t id_length = 0;
    std::uint8_t color_map_type = 0;
    std::uint8_t image_type = 0;

    // color map specification
    std::uint16_t first_entry_index = 0;
    std::uint16_t color_map_length = 0;
    std::uint8_t color_map_entry_size = 0;

    // image specification
    std::uint16_t x_origin = 0;
    std::uint16_t y_origin = 0;
    std::uint16_t image_width = 0;
    std::uint16_t image_height = 0;
    std::uint8_t pixel_depth = 0; // the bits one pixel has
    std::uint8_t image_descriptor = 0;
};
#pragma pack()

static_assert(sizeof(TgaHeader) == 18, "TgaHeader layout mismatch!");

struct TgaColor {
    std::uint8_t bgra[4] = {0, 0, 0, 0};
    std::uint8_t bytespp = 4; // bytes per pixel

    std::uint8_t &operator[](const int index) { return bgra[index]; }
    const std::uint8_t &operator[](const int index) const {
        return bgra[index];
    }
};

inline bool operator==(const TgaColor &lhs, const TgaColor &rhs) {
    if (lhs.bytespp != rhs.bytespp)
        return false;
    for (int i = 0; i < lhs.bytespp; ++i) {
        if (lhs[i] != rhs[i])
            return false;
    }
    return true;
}
inline bool operator!=(const TgaColor &lhs, const TgaColor &rhs) {
    return !(lhs == rhs);
}

class TgaImage {
  private:
    int width_ = 0, height_ = 0;
    std::uint8_t bytespp_ = 0;
    std::vector<std::uint8_t> data_ = {};

    bool load_rle_data(std::ifstream &in);
    bool save_rle_data(std::ofstream &out) const;

  public:
    enum Format { GRAYSCALE = 1, RGB = 3, RGBA = 4 };

    TgaImage() = default;
    TgaImage(const int width, const int height, const int bytespp);

    int get_width() const { return width_; }
    int get_height() const { return height_; }

    TgaColor get_pixel(const int x, const int y) const;
    bool set_pixel(int x, int y, const TgaColor &tga_color);

    bool read_tga_file(const std::string &filename);
    bool write_tga_file(const std::string &filename,
                        const bool is_v_flip = true,
                        const bool is_rle = true) const;

    // flip horizontally image data
    bool flip_horizontally();
    // flip vertically image data
    bool flip_vertically();
};
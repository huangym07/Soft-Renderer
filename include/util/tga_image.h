#ifndef __TGA_IMAGE_H__
#define __TGA_IMAGE_H__

#include <cstdint>
#include <cstring>
#include <iostream>

// tga 格式文件的头部
// 详细信息参考 https://en.wikipedia.org/wiki/Truevision_TGA#Header
// 为了和外部二进制交互，禁用内存对齐
#pragma pack(push, 1)
struct TgaHeader {
    uint8_t id_length;
    uint8_t color_map_type;
    uint8_t image_type;

    // color map specification
    uint16_t first_entry_index;
    uint16_t color_map_length;
    uint8_t color_map_entry_size;

    // image specification
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t image_width;
    uint16_t image_height;
    uint8_t pixel_depth; // the bits one pixel has
    uint8_t image_descriptor;
};
#pragma pack(pop)

static_assert(sizeof(TgaHeader) == 18, "TgaHeader layout mismatch!");

struct TgaColor {
    union {
        struct {
            uint8_t b, g, r, a;
        };
        uint32_t val; // value
        uint8_t raw[4];
    };
    int bytespp; // bytes per pixel

    TgaColor() : val(0), bytespp(1) {}
    TgaColor(uint8_t B, uint8_t G, uint8_t R, uint8_t A)
        : b(B), g(G), r(R), a(A), bytespp(4) {}
    TgaColor(uint32_t value, int bytes_per_pixel)
        : val(value), bytespp(bytes_per_pixel) {}
    TgaColor(const uint8_t *p, int bytes_per_pixel)
        : val(0), bytespp(std::min(4, bytes_per_pixel)) {
        for (int i = 0; i < bytespp; ++i) {
            raw[i] = p[i];
        }
    }

    TgaColor(const TgaColor &tga_color)
        : val(tga_color.val), bytespp(tga_color.bytespp) {}
    TgaColor &operator=(const TgaColor &tga_color) {
        if (this != &tga_color) {
            val = tga_color.val;
            bytespp = tga_color.bytespp;
        }
        return *this;
    }
};

inline bool operator==(const TgaColor &lhs, const TgaColor &rhs) {
    if (lhs.bytespp != rhs.bytespp) return false;
    for (int i = 0; i < lhs.bytespp; ++i) {
        if (lhs.raw[i] != rhs.raw[i]) return false;
    }
    return true;
}
inline bool operator!=(const TgaColor &lhs, const TgaColor &rhs) {
    return !(lhs == rhs);
}

class TgaImage {
  private:
    int width_, height_;
    int bytespp_;
    uint8_t *data_;

    bool load_rle_data(std::ifstream &in);
    bool save_rle_data(std::ofstream &out) const;

  public:
    enum Format { GRAYSCALE = 1, RGB = 3, RGBA = 4 };

    TgaImage();
    TgaImage(int width, int height, int bytespp);

    friend void swap(TgaImage &lhs, TgaImage &rhs) noexcept;
    // deep copy
    TgaImage(const TgaImage &tga_image);
    TgaImage(TgaImage &&tga_image) noexcept;
    TgaImage &operator=(TgaImage tga_image);

    ~TgaImage() {
        if (data_)
            delete[] data_;
    }

    uint8_t *get_buffer() const { return data_; }
    int get_width() const { return width_; }
    int get_height() const { return height_; }
    int get_bytes_per_pixel() const { return bytespp_; }

    TgaColor get_pixel(int x, int y) const;
    bool set_pixel(int x, int y, const TgaColor &tga_color);

    bool read_tga_file(const std::string &filename);
    bool write_tga_file(const std::string &filename, bool is_rle = true) const;

    // flip horizontally image data
    bool flip_horizontally();
    // flip vertically image data
    bool flip_vertically();

    // scale the image to new_width x new_height
    bool scale(int new_width, int new_height);

    void clear();
};

#endif //__TGA_IMAGE_H__
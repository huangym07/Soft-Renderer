#include "tga_image.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>

TgaImage::TgaImage(const int width, const int height, const int bytespp)
    : width_(width), height_(height), bytespp_(bytespp),
      data_(width * height * bytespp, 0) {}

TgaColor TgaImage::get_pixel(int x, int y) const {
    if (!data_.size() || x < 0 || y < 0 || x >= width_ || y >= height_)
        return {};

    TgaColor ret_color = {0, 0, 0, 0, bytespp_};

    const std::uint8_t *pdata = data_.data() + (y * width_ + x) * bytespp_;
    for (int i = 0; i < bytespp_; ++i)
        ret_color[i] = pdata[i];

    return ret_color;
}

bool TgaImage::set_pixel(int x, int y, const TgaColor &tga_color) {
    if (!data_.size() || x < 0 || y < 0 || x >= width_ || y >= height_)
        return false;

    std::memcpy(data_.data() + (y * width_ + x) * bytespp_, tga_color.bgra,
                bytespp_);
    return true;
}

bool TgaImage::read_tga_file(const std::string &filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Failed to open " << filename << ".\n";
        return false;
    }

    TgaHeader header;
    in.read(reinterpret_cast<char *>(&header), sizeof(header));
    if (!in.good()) {
        std::cerr << "An error occured while reading the header.\n";
        return false;
    }

    width_ = header.image_width;
    height_ = header.image_height;
    bytespp_ = header.pixel_depth >> 3;

    if (width_ <= 0 || height_ <= 0 ||
        (bytespp_ != GRAYSCALE && bytespp_ != RGB && bytespp_ != RGBA)) {
        std::cerr << "Width or height or bytes per pixel value is wrong.\n";
        return false;
    }

    in.ignore(header.id_length);
    if (!in.good()) {
        std::cerr << "An error occured while skipping the image id.\n";
        return false;
    }

    uint32_t nbytes = width_ * height_ * bytespp_;
    data_ = std::vector<std::uint8_t>(nbytes, 0);
    if (header.image_type == 2 || header.image_type == 3) {
        in.read(reinterpret_cast<char *>(data_.data()), nbytes);
        if (!in.good()) {
            std::cerr << "An error occured while reading the image data.\n";
            return false;
        }
    } else if (header.image_type == 10 || header.image_type == 11) {
        if (!load_rle_data(in)) {
            std::cerr << "An error occured while reading the run-length "
                         "encoded image data.\n";
            return false;
        }
    } else {
        std::cerr << "Unknown file type " << (int)header.image_type << ".\n";
        return false;
    }

    if (header.image_descriptor & 0x10) {
        flip_horizontally();
    }
    if (!(header.image_descriptor & 0x20)) {
        flip_vertically();
    }

    // WxH/Bits
    std::cerr << width_ << "x" << height_ << "/" << bytespp_ * 8 << '\n';

    return true;
}

bool TgaImage::load_rle_data(std::ifstream &in) {
    int npixels = width_ * height_;
    int pixel_count = 0;
    int byte_count = 0;

    TgaColor color_buffer;
    unsigned char packet_header = 0;
    int nbytes = 0;
    while (pixel_count < npixels) {
        packet_header = in.get();
        if (!in.good())
            return false;

        if (packet_header <= 127) {
            packet_header++;
            if (packet_header + pixel_count > npixels) {
                std::cerr << "Read too many pixels.\n";
                return false;
            }

            nbytes = packet_header * bytespp_;
            in.read(reinterpret_cast<char *>(data_.data() + byte_count),
                    nbytes);
            if (!in.good())
                return false;

            byte_count += nbytes;
        } else {
            packet_header -= 127;
            if (packet_header + pixel_count > npixels) {
                std::cerr << "Read too many pixels.\n";
                return false;
            }

            in.read(reinterpret_cast<char *>(color_buffer.bgra), bytespp_);
            if (!in.good())
                return false;
            for (int i = 0; i < packet_header; ++i, byte_count += bytespp_) {
                std::memcpy(data_.data() + byte_count, color_buffer.bgra,
                            bytespp_);
            }
        }

        pixel_count += packet_header;
    }

    return true;
}

bool TgaImage::flip_horizontally() {
    for (int i = 0; i < height_; ++i) {
        for (int j = 0; j < width_ / 2; ++j) {
            for (int k = 0; k < bytespp_; ++k) {
                std::swap(data_[(i * width_ + j) * bytespp_ + k],
                          data_[(i * width_ + width_ - 1 - j) * bytespp_ + k]);
            }
        }
    }

    return true;
}

bool TgaImage::flip_vertically() {
    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_ / 2; ++j) {
            for (int k = 0; k < bytespp_; ++k) {
                std::swap(
                    data_[(j * width_ + i) * bytespp_ + k],
                    data_[((height_ - 1 - j) * width_ + i) * bytespp_ + k]);
            }
        }
    }
    return true;
}

bool TgaImage::write_tga_file(const std::string &filename, const bool is_v_flip,
                              const bool is_rle) const {
    std::ofstream out(filename, std::ios::binary);

    if (!out.is_open()) {
        std::cerr << "Failed to open file " << filename << ".\n";
        return false;
    }

    TgaHeader tga_header = {0};
    tga_header.image_type =
        bytespp_ == GRAYSCALE ? (is_rle ? 11 : 3) : (is_rle ? 10 : 2);
    tga_header.image_width = width_;
    tga_header.image_height = height_;
    tga_header.pixel_depth = bytespp_ << 3;
    tga_header.image_descriptor =
        is_v_flip ? 0x00 : 0x20; // bottom-left origin or top-left origin
    out.write(reinterpret_cast<char *>(&tga_header), sizeof(tga_header));
    if (!out.good()) {
        std::cerr << "An error occured while writing the tga header.\n";
        return false;
    }

    if (is_rle) {
        if (!save_rle_data(out)) {
            std::cerr << "An error occured while saving the run-length encoded "
                         "image data.\n";
            return false;
        }
    } else {
        out.write(reinterpret_cast<const char *>(data_.data()),
                  width_ * height_ * bytespp_);
        if (!out.good()) {
            std::cerr << "An error occured while writing the image data.\n";
            return false;
        }
    }

    return true;
}

bool TgaImage::save_rle_data(std::ofstream &out) const {
    int max_packet_length = 128;
    int npixels = width_ * height_;
    int cur_pixel = 0;

    int packet_shart = 0;

    while (cur_pixel < npixels) {
        uint8_t run_length = 1;
        uint32_t cur_byte = cur_pixel * bytespp_;
        bool is_raw = true;
        while (cur_pixel + run_length < npixels &&
               run_length < max_packet_length) {
            bool is_same = true;
            for (int i = 0; is_same && i < bytespp_; ++i) {
                is_same = data_[cur_byte + i] == data_[cur_byte + bytespp_ + i];
            }
            if (run_length == 1)
                is_raw = !is_same;

            if (is_same && is_raw) {
                run_length--;
                break;
            }

            if (!is_same && !is_raw) {
                break;
            }

            cur_byte += bytespp_;
            run_length++;
        }

        out.put(is_raw ? run_length - 1 : run_length + 127);
        if (!out.good())
            return false;

        out.write(
            reinterpret_cast<const char *>(data_.data() + cur_pixel * bytespp_),
            is_raw ? run_length * bytespp_ : bytespp_);
        if (!out.good())
            return false;

        cur_pixel += run_length;
    }

    return true;
}
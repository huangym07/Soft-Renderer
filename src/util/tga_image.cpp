#include <cstdint>
#include <cstring>
#include <fstream>
#include <util/tga_image.h>

TgaImage::TgaImage() : width_(0), height_(0), bytespp_(0), data_(nullptr) {}
TgaImage::TgaImage(int width, int height, int bytespp)
    : width_(width), height_(height), bytespp_(bytespp), data_(nullptr) {
    uint32_t nbytes = width_ * height_ * bytespp_;
    data_ = new uint8_t[nbytes];
    std::memset(data_, 0, nbytes);
}

void swap(TgaImage &lhs, TgaImage &rhs) noexcept {
    using std::swap;
    swap(lhs.data_, rhs.data_);
    swap(lhs.width_, rhs.width_);
    swap(lhs.height_, rhs.height_);
    swap(lhs.bytespp_, rhs.bytespp_);
}

TgaImage::TgaImage(const TgaImage &tga_image)
    : width_(tga_image.width_), height_(tga_image.height_),
      bytespp_(tga_image.bytespp_), data_(nullptr) {
    uint32_t nbytes = width_ * height_ * bytespp_;
    data_ = new uint8_t[nbytes];
    std::memcpy(data_, tga_image.data_, nbytes);
}

TgaImage::TgaImage(TgaImage &&tga_image) noexcept
    : width_(tga_image.width_), height_(tga_image.height_),
      bytespp_(tga_image.bytespp_), data_(tga_image.data_) {
    tga_image.data_ = nullptr;
}

TgaImage &TgaImage::operator=(TgaImage tga_image) {
    swap(*this, tga_image);
    return *this;
}

TgaColor TgaImage::get_pixel(int x, int y) const {
    if (!data_ || x < 0 || y < 0 || x >= width_ || y >= height_)
        return TgaColor();

    return {data_ + (y * width_ + x) * bytespp_, bytespp_};
}

bool TgaImage::set_pixel(int x, int y, const TgaColor &tga_color) {
    if (!data_ || x < 0 || y < 0 || x >= width_ || y >= height_)
        return false;

    std::memcpy(data_ + (y * width_ + x) * bytespp_, tga_color.raw, bytespp_);
    return true;
}

bool TgaImage::read_tga_file(const std::string &filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Failed to open " << filename << ".\n";
        return false;
    }

    TgaHeader header;
    in.read((char *)&header, sizeof(header));
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
    if (data_)
        delete[] data_;
    data_ = new uint8_t[nbytes];
    if (header.image_type == 2 || header.image_type == 3) {
        in.read((char *)data_, nbytes);
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
            in.read((char *)(data_ + byte_count), nbytes);
            if (!in.good())
                return false;

            byte_count += nbytes;
        } else {
            packet_header -= 127;
            if (packet_header + pixel_count > npixels) {
                std::cerr << "Read too many pixels.\n";
                return false;
            }

            in.read((char *)color_buffer.raw, bytespp_);
            if (!in.good())
                return false;
            for (int i = 0; i < packet_header; ++i, byte_count += bytespp_) {
                std::memcpy(data_ + byte_count, color_buffer.raw, bytespp_);
            }
        }

        pixel_count += packet_header;
    }

    return true;
}

bool TgaImage::flip_horizontally() {
    if (!data_)
        return false;

    TgaColor color_buffer;
    for (int i = 0; i < height_; ++i) {
        uint8_t *row = data_ + i * width_ * bytespp_;
        int mid = (width_ - 1) / 2;
        for (int j = 0; j <= mid; ++j) {
            int lhs_offset = j * bytespp_;
            int rhs_offset = (width_ - 1 - j) * bytespp_;
            std::memcpy(color_buffer.raw, row + lhs_offset, bytespp_);
            std::memcpy(row + lhs_offset, row + rhs_offset, bytespp_);
            std::memcpy(row + rhs_offset, color_buffer.raw, bytespp_);
        }
    }

    return true;
}

bool TgaImage::flip_vertically() {
    if (!data_)
        return false;

    int row_bytes = width_ * bytespp_;
    uint8_t *tmp_row = new uint8_t[row_bytes];

    int mid = (height_ - 1) / 2;
    for (int i = 0; i <= mid; ++i) {
        int lhs_offset = i * row_bytes;
        int rhs_offset = (height_ - 1 - i) * row_bytes;
        std::memcpy(tmp_row, data_ + lhs_offset, row_bytes);
        std::memcpy(data_ + lhs_offset, data_ + rhs_offset, row_bytes);
        std::memcpy(data_ + rhs_offset, tmp_row, row_bytes);
    }

    return true;
}

bool TgaImage::write_tga_file(const std::string &filename, bool is_rle) const {
    if (!data_)
        return false;

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
    tga_header.image_descriptor = 0x20; // top-left origin
    out.write((char *)&tga_header, sizeof(tga_header));
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
        out.write((char *)data_, width_ * height_ * bytespp_);
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

        out.write((char *)&data_[cur_pixel * bytespp_],
                  is_raw ? run_length * bytespp_ : bytespp_);
        if (!out.good())
            return false;

        cur_pixel += run_length;
    }

    return true;
}

// scale width_ x height_ -> new_width x new_height
// 最邻近插值
// 逆向映射(目标图像遍历)
// 整数误差累积法替代浮点数计算, 提高性能
bool TgaImage::scale(int new_width, int new_height) {
    if (new_width <= 0 || new_height <= 0 || !data_)
        return false;

    uint8_t *new_data = new uint8_t[new_width * new_height * bytespp_];

    int erry = -height_;
    int y = 0, new_y = 0;
    int errx = -width_;
    int x = 0, new_x = 0;

    int new_width_bytes = new_width * bytespp_;
    int new_y_offset_bytes = 0;
    int new_x_offset_bytes = 0;

    for (erry = -height_, y = 0, new_y = 0, new_y_offset_bytes = 0;
         new_y < new_height; ++new_y, new_y_offset_bytes += new_width_bytes) {
        erry += height_;
        // 行复制优化
        // 当目标图像的本行与上一行映射到源图像的不同行时, 逐像素复制本行
        // 否则整行复制目标图像的上一行到本行
        if (y != height_ - 1 && erry >= new_height) {
            y += erry / new_height;
            y = std::min(y, height_ - 1);
            erry %= new_height;

            for (errx = -width_, x = 0, new_x = 0, new_x_offset_bytes = 0;
                 new_x < new_width; ++new_x, new_x_offset_bytes += bytespp_) {
                errx += width_;
                x += errx / new_width;
                x = std::min(x, width_ - 1);
                errx %= new_width;
                memcpy(new_data + new_y_offset_bytes + new_x_offset_bytes,
                       data_ + (y * width_ + x) * bytespp_, bytespp_);
            }
        } else {
            memcpy(new_data + new_y_offset_bytes - new_width_bytes,
                   new_data + new_y_offset_bytes, new_width_bytes);
        }
    }

    delete[] data_;
    data_ = new_data;
    width_ = new_width;
    height_ = new_height;

    return true;
}

void TgaImage::clear() {
    if (data_)
        std::memset(data_, 0, width_ * height_ * bytespp_);
}
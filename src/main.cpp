#include <iostream>
#include <util/tga_image.h>

int main() {
    TgaHeader tga_header;

    std::cout << "TgaHeader size is " << sizeof(tga_header) << " bytes" << std::endl;

    return 0;
}
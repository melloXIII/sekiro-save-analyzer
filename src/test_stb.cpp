#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
int main() {
    int w, h, c;
    unsigned char* data = stbi_load("bg.jpg", &w, &h, &c, 4);
    if (!data) std::cout << "FAILED: " << stbi_failure_reason() << std::endl;
    else std::cout << "SUCCESS: " << w << "x" << h << std::endl;
    return 0;
}

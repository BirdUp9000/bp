#include "../bmp_reader.hpp"

#include <cstdint>
#include <vector>
#include <cstdio>





int main() {

  FILE* file = fopen("/home/rmn/Programming/BMPRDR/test/pictures/pacan.bmp", "rb");
  if (!file) {
    std::cerr << "Failed to open the BMP file!" << std::endl;
    return EXIT_FAILURE;
  }

  BITMAPFILEHEADER bmpFileHeader;
  fread(&bmpFileHeader, sizeof(BITMAPFILEHEADER), 1, file);

  std::cout << "File type: " << bmpFileHeader.file_type << "\n"
            << "File size: " << bmpFileHeader.file_size << "\n"
            << "Offset: " << bmpFileHeader.offset << "\n"
            << "Reserved 1: " << bmpFileHeader.reserved1 << "\n"
            << "Reserved 2: " << bmpFileHeader.reserved2 << std::endl;




  return EXIT_SUCCESS;
}
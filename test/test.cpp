#include "../bmp_reader.hpp"

#include <cstdint>
#include <vector>
#include <cstdio>





int main() {

  BMPINFO bmp_file(std::filesystem::path("/home/rmn/Programming/BMPRDR/test/pictures/pal8os2.bmp"));
  std::ostringstream oss;
  oss << bmp_file;
  std::cout << oss.str();
  bmp_file.getFileHeader()->getFileSize();


  return EXIT_SUCCESS;
}
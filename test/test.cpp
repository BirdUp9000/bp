#include "../bmp_reader.hpp"

#include <cstdint>
#include <vector>
#include <cstdio>





int main() {

  BMPINFO bmp_file(std::filesystem::path("/home/rmn/Programming/BMPRDR/test/pictures/pacan.bmp"));
  //std::ostringstream oss;
  //oss << bmp_file;
  //std::cout << oss.str();





  return EXIT_SUCCESS;
}
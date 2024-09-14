#ifndef BMP_READER_HPP
#define BMP_READER_HPP

#include <fstream>
#include <iostream>
#include <string>

struct FileHeader {
  
};

struct BmpInfo {
  FileHeader file_header;

};

class BSCII {
  public:
    BSCII(const std::string & filename);
    BSCII(const BSCII &) = delete;
    BSCII(BSCII &&) = delete;
    BSCII & operator=(const BSCII &) = delete;
    BSCII & operator=(BSCII &&) = delete;
    ~BSCII() = default;





  private:







};


// SOME OF THE OLD LOGICS
/*
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>

using namespace std;

#define NORMAL_MODE 1


struct Info {
  string filepath;
  string format;
  size_t length;
  size_t bitmap_location;
  size_t width;
  size_t height;
  size_t data_offset;
  size_t bit_per_pix;
  size_t raw_bitmap;
};



size_t takeData(int start_byte, int end_byte, size_t size, unsigned char *bytes);
void printInfo(const Info& PictureInfo);

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "Not enough args" << endl;
    return EXIT_FAILURE;
  }

  if (argc > 2) {
    cout << "Too many args" << endl;
    return EXIT_FAILURE;
  }

  string filename = argv[1];
  ifstream picture(filename, ios::binary);

  if (!picture) {
    cout << "Cannot read file: " << filename << endl;
    return EXIT_FAILURE;
  }

  Info PictureInfo;
  PictureInfo.filepath = argv[1];

  // Перемещаем указатель в конец файла
  picture.seekg(0, ios::end);
  // Получаем позицию указателя, которая равна размеру файла
  streampos length = picture.tellg();
  // Возвращаем указатель в начало файла, если собираемся с ним работать
  picture.seekg(0, ios::beg);

  PictureInfo.length = length;
  unsigned char* bytes = new unsigned char[length];

  picture.read(reinterpret_cast<char*>(bytes), length);
  picture.close();

  if (bytes[0] == 0x42 && bytes[1] == 0x4D) {
    PictureInfo.format = "BMP";
  }

  PictureInfo.bitmap_location = takeData(10, 14, 4, bytes);
  PictureInfo.width = takeData(18, 22, 4, bytes);
  PictureInfo.height = takeData(22, 26, 4, bytes);
  PictureInfo.bit_per_pix = takeData(28, 30, 2, bytes);
  PictureInfo.raw_bitmap = takeData(34, 38, 4, bytes);

  printInfo(PictureInfo);

  //for (size_t i = PictureInfo.bitmap_location; i < PictureInfo.bitmap_location + PictureInfo.raw_bitmap; i++){
  //  cout << static_cast<int>(bytes[i]) << ' ';
  //}
  cout << endl;

  size_t counter = 0;
  size_t lum_len = PictureInfo.raw_bitmap / 3;
  int* luminocity = new int[lum_len];
  memset(luminocity, 0, lum_len * sizeof(int));

  if ((PictureInfo.width * 3) % 4 == 0) {
    //   Y = 0.299R + 0.587G + 0.114B
    for (size_t i = PictureInfo.bitmap_location + PictureInfo.raw_bitmap; i > PictureInfo.bitmap_location;){
      float B = (0.114 * static_cast<int>(bytes[i--]));
      float G = (0.587 * static_cast<int>(bytes[i--]));
      float R = (0.299 * static_cast<int>(bytes[i--]));

      luminocity[counter++] = static_cast<int>(B + G + R);
    }
  } else {
    for (size_t i = PictureInfo.bitmap_location + PictureInfo.raw_bitmap; i > PictureInfo.bitmap_location;){
      if (counter % PictureInfo.width == 0) i -= 2;
      float B = (0.114 * static_cast<int>(bytes[i--]));
      float G = (0.587 * static_cast<int>(bytes[i--]));
      float R = (0.299 * static_cast<int>(bytes[i--]));

      luminocity[counter++] = static_cast<int>(B + G + R);
    }
  }
  delete[] bytes;

  //for (size_t i = 0; i < lum_len; i++){
  //  cout << luminocity[i] << ' ';
  //  if ((i + 1) % PictureInfo.width == 0) cout << endl;
  //}

  const char* ASCII_DENSITY = " .:-+*#%@";
  char* ascii = new char[lum_len];

  for (size_t i = 0; i < lum_len; i++){
    # if NORMAL_MODE
    if (luminocity[i] <= 25) ascii[i] = ASCII_DENSITY[0];
    if (luminocity[i] > 25 && luminocity[i] <= 50) ascii[i] = ASCII_DENSITY[1];
    if (luminocity[i] > 50 && luminocity[i] <= 75) ascii[i] = ASCII_DENSITY[2];
    if (luminocity[i] > 75 && luminocity[i] <= 100) ascii[i] = ASCII_DENSITY[3];
    if (luminocity[i] > 100 && luminocity[i] <= 125) ascii[i] = ASCII_DENSITY[4];
    if (luminocity[i] > 125 && luminocity[i] <= 150) ascii[i] = ASCII_DENSITY[5];
    if (luminocity[i] > 175 && luminocity[i] <= 200) ascii[i] = ASCII_DENSITY[6];
    if (luminocity[i] > 200 && luminocity[i] <= 225) ascii[i] = ASCII_DENSITY[7];
    if (luminocity[i] > 225) ascii[i] = ASCII_DENSITY[8];
    # else
    if (luminocity[i] <= 25) ascii[i] = ASCII_DENSITY[8];
    if (luminocity[i] > 25 && luminocity[i] <= 50) ascii[i] = ASCII_DENSITY[7];
    if (luminocity[i] > 50 && luminocity[i] <= 75) ascii[i] = ASCII_DENSITY[6];
    if (luminocity[i] > 75 && luminocity[i] <= 100) ascii[i] = ASCII_DENSITY[5];
    if (luminocity[i] > 100 && luminocity[i] <= 125) ascii[i] = ASCII_DENSITY[4];
    if (luminocity[i] > 125 && luminocity[i] <= 150) ascii[i] = ASCII_DENSITY[3];
    if (luminocity[i] > 175 && luminocity[i] <= 200) ascii[i] = ASCII_DENSITY[2];
    if (luminocity[i] > 200 && luminocity[i] <= 225) ascii[i] = ASCII_DENSITY[1];
    if (luminocity[i] > 225) ascii[i] = ASCII_DENSITY[0];
    # endif
  }
  delete[] luminocity;

  char* mirrored_picture = new char[lum_len]; 
  int row = PictureInfo.width;
  int index = 0;

  for (size_t i = 0; i < lum_len; i += row) {
    int start = i;
    int end = i + row - 1;

    for (int j = end; j >= start; j--) {
      mirrored_picture[index++] = ascii[j];
    }
  }
  delete[] ascii;

  for (size_t i = 0, height = 1; i < lum_len && height < PictureInfo.height; i++){
    cout << mirrored_picture[i] << ' ';
    if ((i + 1) % PictureInfo.width == 0) {
      cout << endl;
      height++;
    }
  }
  cout << endl;
  delete[] mirrored_picture;

  return EXIT_SUCCESS;
}

size_t takeData(int start_byte, int end_byte, size_t size, unsigned char *bytes){
  unsigned char* b = new unsigned char[size];
  for (size_t i = 0; i < size; i++){
    b[i] = 0;
  }

  for (int i = start_byte; i < end_byte; i++) {
    b[i - start_byte] = bytes[i];
  }
  size_t ans;

  if (size == 4){
    ans = b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
  }
  if (size == 2){
    ans = b[0] | (b[1] << 8);
  }
  delete[] b;
  return ans;
}

void printInfo(const Info& PictureInfo){
  cout << "Filepath: " << PictureInfo.filepath << endl;
  cout << "Format: " << PictureInfo.format << endl;
  cout << "Length: " << PictureInfo.length << endl;
  cout << "Width: " << PictureInfo.width << endl;
  cout << "Height: " << PictureInfo.height << endl;
  cout << "Number of bits per pixel: " << PictureInfo.bit_per_pix << endl;
  cout << "Offset where the pixel array (bitmap data) can be found: " << PictureInfo.bitmap_location << endl;
  cout << "Size of the raw bitmap data: " << PictureInfo.raw_bitmap << endl;
}


*/



#endif // BMP_READER_HPP
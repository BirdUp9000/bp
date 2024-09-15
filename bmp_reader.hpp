/**
 * @file bmp_reader.hpp
 * @author Artem Romanov linkedin.com/in/a-rmn/
 * @brief 
 * @version 0.1
 * @date 2024-09-14
 * 
 * @copyright Copyright (c) 2024
 * 
*/

#ifndef BMP_READER_LIB_HPP
#define BMP_READER_LIB_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>



//-------------------------------------------------------------------------------------------------
//  Definition of the file header structure
//-------------------------------------------------------------------------------------------------
/**
 * @brief Store general information about the bitmap image file.
 * @brief Source: https://en.wikipedia.org/wiki/BMP_file_format
 * 
*/
#pragma pack(push, 1)
struct BITMAPFILEHEADER {
  uint16_t file_type;             /* The header field used to identify the BMP */
  uint32_t file_size;             /* The size of the BMP file in bytes */
  uint16_t reserved1;             /* Reserved. Must be set to 0 */
  uint16_t reserved2;             /* Reserved. Must be set to 0 */
  uint32_t offset;                /* Address of the byte where the bitmap data can be found */
};
#pragma pack(pop)


//-------------------------------------------------------------------------------------------------
// Definitions for all of the 7 variants of DIB headers.
//-------------------------------------------------------------------------------------------------
/**
 * @brief DIB header (bitmap information header).
 * @brief BITMAPCOREHEADER/OS21XBITMAPHEADER
 * @brief Source: https://www.fileformat.info/format/os2bmp/egff.htm
 *
*/
#pragma pack(push, 1)
struct BITMAPCOREHEADER {
  uint32_t header_size;           /* Size of this structure in bytes */
  uint32_t bitmap_width;          /* Bitmap width in pixels */
  uint32_t bitmap_height;         /* Bitmap height in pixel */
  uint16_t color_planes;          /* Number of bit planes (color depth) */
  uint16_t bits_per_pixel;        /* Number of bits per pixel per plane */
};
#pragma pack(pop)


/**
 * @brief Adds halftoning. Adds RLE and Huffman 1D compression.
 * @brief May contain only the first 16 bytes.
 * @brief Source: https://www.fileformat.info/format/os2bmp/egff.htm
 * 
*/
#pragma pack(push, 1)
struct OS22XBITMAPHEADER {
  BITMAPCOREHEADER header;        /* This header contains info from previous header */
	uint32_t compression;           /* Bitmap compression scheme */
	uint32_t bitmap_size;           /* Size of bitmap data in bytes */
	uint32_t h_px_res;              /* X resolution of display device */
	uint32_t v_px_res;              /* Y resolution of display device */
	uint32_t colors;                /* Number of color table indices used */
	uint32_t important_colors;      /* Number of important color indices */
	uint16_t units;                 /* Type of units used to measure resolution */
	uint16_t reserved;              /* Pad structure to 4-byte boundary */
	uint16_t recording;             /* Recording algorithm */
	uint16_t rendering;             /* Halftoning algorithm used */
	uint32_t size1;                 /* Reserved for halftoning algorithm use */
	uint32_t size2;                 /* Reserved for halftoning algorithm use */
	uint32_t color_encoding;         /* Color model used in bitmap */
	uint32_t identifier;            /* Reserved for application use */
};
#pragma pack(pop)


/**
 * @brief This is the identical structure defined in Windows.
 * @brief Extends bitmap width and height to 4 bytes. Adds 16 bpp and 32 bpp formats.
 * @brief Adds RLE compression.
 * @brief Source: 
 * https://help.accusoft.com/AIMTools/ProgrammersReference/GeneralStructures-BitmapInfoHeader.html
 * 
*/
#pragma pack(push, 1)
struct BITMAPINFOHEADER {
  BITMAPCOREHEADER header;        /* This header contains info from previous header */
  uint32_t  compression;          /* This value indicates the format of the image */
  uint32_t  bitmap_size;          /* This value is the size in bytes of the image data */
  int32_t   x_px_per_meter;       /* Specifies the horizontal print resolution */
  int32_t   y_px_per_meter;       /* Specifies the vertical print resolution */
  uint32_t  colors;               /* Number of RGBQUAD elements */
  uint32_t  important_colors;     /* The first biClrImportant elements of ColorTable */
};
#pragma pack(pop)


/**
 * @brief Adds RGB bit masks.
 * @brief Source: https://formats.kaitai.io/bmp/
*/
#pragma pack(push, 1)
struct BITMAPV2INFOHEADER {
  BITMAPINFOHEADER header;        /* This header contains info from previous header */
  uint32_t red_mask;              /* Color mask that specifies the 'color' component */
  uint32_t green_mask;            /* of each pixel, valid only if the Compression */
  uint32_t blue_mask;             /* member is set to BI_BITFIELDS */
};
#pragma pack(pop)


/**
 * @brief Adds alpha channel bit mask.
 * @brief Source: https://formats.kaitai.io/bmp/
 * 
*/
#pragma pack(push, 1)
struct BITMAPV3INFOHEADER {
  BITMAPV2INFOHEADER header;      /* This header contains info from previous header */
  uint32_t alpha_mask;            /* Alpha channel bit mask that specifies the transparency */
};
#pragma pack(pop)


/**
 * @brief FXPT2DOT30 data type.
 * @brief FXPT2DOT30 is a fixed-point data type used in Windows,
 * @brief specifically for color profiles and certain image formats like BMP.
 * @brief It represents a 32-bit signed fixed-point number with:
 * @brief 2 bits for the integer part,
 * @brief 30 bits for the fractional part.
 * @brief We will save this in int32_t and convert this value by ourselves.
 * @brief Source: https://courses.cs.washington.edu/courses/cse373/00sp/bmp.h
 * 
*/
typedef int32_t FXPT2DOT30;

/**
 * @brief The CIEXYZ structure contains the x,y, and z coordinates 
 * @brief of a specific color in a specified color space.
 * @brief Windows structure implementation.
 * @brief Source: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-ciexyz
 * 
*/
struct CIEXYZ {
  FXPT2DOT30 ciexyzX;
  FXPT2DOT30 ciexyzY;
  FXPT2DOT30 ciexyzZ;
};

/**
 * @brief The CIEXYZTRIPLE structure contains the x,y, and z coordinates
 * @brief of the three colors that correspond to the red, green, and blue
 * @brief endpoints for a specified logical color space.
 * 
*/
struct CIEXYZTRIPLE {
  CIEXYZ ciexyzRed;
  CIEXYZ ciexyzGreen;
  CIEXYZ ciexyzBlue;
};

/**
 * @brief Adds color space type and gamma correction.
 * @brief Source: 
 * @brief https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv4header
 * 
*/
#pragma pack(push, 1)
struct BITMAPV4HEADER {
  BITMAPV3INFOHEADER header;      /* This header contains info from previous header */
  uint32_t type;                  /* Color space type */
  CIEXYZTRIPLE bV4Endpoints;      /* Specifies the coordinates of the three colors */
  uint32_t gamma_red;             /* Gamma red coordinate scale value */
  uint32_t gamma_green;           /* Gamma green coordinate scale value */
  uint32_t gamma_blue;            /* Gamma blue coordinate scale value */
};
#pragma pack(pop)


/**
 * @brief Adds ICC color profiles.
 * @brief Source: 
 * @brief https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv5header
 * 
*/
#pragma pack(push, 1)
struct BITMAPV5HEADER {
  BITMAPV4HEADER header;          /* This header contains info from previous header */
  uint32_t intent;                /* Rendering intent for bitmap */
  uint32_t profile_data;          /* The offset from the start of the header to the profile data.*/
  uint32_t profile_size;          /* Size, in bytes, of embedded profile data */
  uint32_t reserved;              /* This member has been reserved. Value should be set to zero */
};
#pragma pack(pop)


//-------------------------------------------------------------------------------------------------
// Definition of the color table
//-------------------------------------------------------------------------------------------------
/**
 * @brief The RGBQUAD structure describes a color consisting of
 * @brief relative intensities of the colors and transparency.
 * @brief Source: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-rgbquad
 * 
*/
#pragma pack(push, 1)
struct RGBQUAD {
  uint8_t blue;                   /* Blue component */
  uint8_t green;                  /* Green component */
  uint8_t red;                    /* Red component */
  uint8_t reserved;               /* Reserved (often used as Alpha channel) */
};
#pragma pack(pop)

/**
 * @brief The RGBTRIPLE structure describes a color consisting of 
 * @brief relative intensities of red, green, and blue.
 * @brief Source: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-rgbtriple
 * 
*/
#pragma pack(push, 1)
struct RGBTRIPLE {
  uint8_t blue;                   /* Blue component */
  uint8_t green;                  /* Green component */
  uint8_t red;                    /* Red component */
};
#pragma pack(pop)



/*
  TODO: Color tables

enum class ColorTableType {
  NONE,
  RGB_TRIPLE,
  RGB_QUAD
};
*/









/*

class BMPRDR {
  public:
    BMPRDR(const std::string & filename);
    BMPRDR(const BMPRDR &) = delete;
    BMPRDR(BMPRDR &&) = delete;
    BMPRDR & operator=(const BMPRDR &) = delete;
    BMPRDR & operator=(BMPRDR &&) = delete;
    ~BMPRDR() = default;





  private:







};

*/
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
    f (counter % PictureInfo.width == 0) i -= 2;
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
  f (luminocity[i] <= 25) ascii[i] = ASCII_DENSITY[0];
  f (luminocity[i] > 25 && luminocity[i] <= 50) ascii[i] = ASCII_DENSITY[1];
  f (luminocity[i] > 50 && luminocity[i] <= 75) ascii[i] = ASCII_DENSITY[2];
  f (luminocity[i] > 75 && luminocity[i] <= 100) ascii[i] = ASCII_DENSITY[3];
  f (luminocity[i] > 100 && luminocity[i] <= 125) ascii[i] = ASCII_DENSITY[4];
  f (luminocity[i] > 125 && luminocity[i] <= 150) ascii[i] = ASCII_DENSITY[5];
  f (luminocity[i] > 175 && luminocity[i] <= 200) ascii[i] = ASCII_DENSITY[6];
  f (luminocity[i] > 200 && luminocity[i] <= 225) ascii[i] = ASCII_DENSITY[7];
  f (luminocity[i] > 225) ascii[i] = ASCII_DENSITY[8];
    # else
  f (luminocity[i] <= 25) ascii[i] = ASCII_DENSITY[8];
  f (luminocity[i] > 25 && luminocity[i] <= 50) ascii[i] = ASCII_DENSITY[7];
  f (luminocity[i] > 50 && luminocity[i] <= 75) ascii[i] = ASCII_DENSITY[6];
  f (luminocity[i] > 75 && luminocity[i] <= 100) ascii[i] = ASCII_DENSITY[5];
  f (luminocity[i] > 100 && luminocity[i] <= 125) ascii[i] = ASCII_DENSITY[4];
  f (luminocity[i] > 125 && luminocity[i] <= 150) ascii[i] = ASCII_DENSITY[3];
  f (luminocity[i] > 175 && luminocity[i] <= 200) ascii[i] = ASCII_DENSITY[2];
  f (luminocity[i] > 200 && luminocity[i] <= 225) ascii[i] = ASCII_DENSITY[1];
  f (luminocity[i] > 225) ascii[i] = ASCII_DENSITY[0];
    # endif
  }
  delete[] luminocity;

  char* mirrored_picture = new char[lum_len]; 
  int row = PictureInfo.width;
  int index = 0;

  for (size_t i = 0; i < lum_len; i += row) {
  nt start = i;
  nt end = i + row - 1;

    for (int j = end; j >= start; j--) {
      mirrored_picture[index++] = ascii[j];
    }
  }
  delete[] ascii;

  for (size_t i = 0, height = 1; i < lum_len && height < PictureInfo.height; i++){
    cout << mirrored_picture[i] << ' ';
  f ((i + 1) % PictureInfo.width == 0) {
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



#endif // BMP_READER_LIB_HPP
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
#include <vector>
#include <memory>
#include <filesystem>
#include <exception>
#include <set>



//-------------------------------------------------------------------------------------------------
//  Predefined constants.
//-------------------------------------------------------------------------------------------------
/**
 * @brief The header field used to identify the BMP and DIB file is 0x42 0x4D in hexadecimal,
 * @brief same as BM in ASCII. The following entries are possible:
 * @brief BM -> Windows 3.1x, 95, NT, ... etc.
 * @brief BA -> OS/2 struct bitmap array
 * @brief CI -> OS/2 struct color icon
 * @brief CP -> OS/2 const color pointer
 * @brief IC -> OS/2 struct icon
 * @brief PT -> OS/2 pointer
 * 
*/
const std::set<uint16_t> validIdentifier = {
  0x4D42, // BM
  0x4142, // BA
  0x4349, // CI
  0x4350, // CP
  0x4943, // IC
  0x5450  // PT
};


constexpr int BITMAPFILEHEADERLENGTH      = 12;
constexpr int BITMAPCOREHEADERLENGTH      = 12;
constexpr int OS22XBITMAPHEADERLENGTH     = 64;
constexpr int BITMAPINFOHEADERLENGTH      = 40;
constexpr int BITMAPV2INFOHEADERLENGTH    = 52;
constexpr int BITMAPV3INFOHEADERLENGTH    = 56;
constexpr int BITMAPV4HEADERLENGTH       = 108;
constexpr int BITMAPV5INFOHEADERLENGTH   = 124;

//-------------------------------------------------------------------------------------------------
//  Definition of the file header structure.
//-------------------------------------------------------------------------------------------------
/**
 * @brief Store general information about the bitmap image file.
 * @brief Source: https://en.wikipedia.org/wiki/BMP_file_format
 * 
*/
#pragma pack(push, 1)
struct BITMAPFILEHEADER {
  BITMAPFILEHEADER() = default;

  [[nodiscard]] bool isValid() const {
    if (validIdentifier.find(file_type) == validIdentifier.end()) return false;
    return true;
  }

  void print() const {
    std::cout << "BITMAPFILEHEADER: " << "\n";
    std::cout << "File Type: " << std::hex << file_type << std::dec << "\n";
    std::cout << "File Size: " << file_size << "\n";
    std::cout << "Reserved 1: " << reserved1 << "\n";
    std::cout << "Reserved 2: " << reserved2 << "\n";
    std::cout << "Offset: " << offset << "\n";
  }

  uint16_t file_type = 0;         /* The header field used to identify the BMP */
  uint32_t file_size = 0;         /* The size of the BMP file in bytes */
  uint16_t reserved1 = 0;         /* Reserved. Must be set to 0 */
  uint16_t reserved2 = 0;         /* Reserved. Must be set to 0 */
  uint32_t offset    = 0;         /* Address of the byte where the bitmap data can be found */
};
#pragma pack(pop)


//-------------------------------------------------------------------------------------------------
// Definitions for all of the 7 variants of DIB headers.
//-------------------------------------------------------------------------------------------------
/**
 * @brief Interface for the 7 types of the headers.
 * 
*/
class HeaderInterface {
  public:
    virtual ~HeaderInterface() = default;
    uint32_t header_size;         /* Size of this structure in bytes */
    uint32_t bitmap_width;        /* Bitmap width in pixels */
    uint32_t bitmap_height;       /* Bitmap height in pixel */
    uint16_t color_planes;        /* Number of bit planes (color depth) */
    uint16_t bits_per_pixel;      /* Number of bits per pixel per plane */
};

/**
 * @brief DIB header (bitmap information header).
 * @brief BITMAPCOREHEADER/OS21XBITMAPHEADER
 * @brief Source: https://www.fileformat.info/format/os2bmp/egff.htm
 *
*/
#pragma pack(push, 1)
struct BITMAPCOREHEADER : public HeaderInterface {};
#pragma pack(pop)

/**
 * @brief Indexed color images may be compressed with 4-bit or 8-bit RLE or Huffman 1D algorithm.
 * @brief OS/2 BITMAPCOREHEADER2 24bpp images may be compressed with the 24-bit RLE algorithm.
 * @brief The 16bpp and 32bpp images are always stored uncompressed.
 * @brief Note that images in all color depths can be stored without compression if so desired.
 * @brief Source: https://en.wikipedia.org/wiki/BMP_file_format#Compression
 * 
*/
enum Compression {    /*      BitCount      |        Pixel Storage         |   Height Sign   */
  BI_RGB,             /*   Any except zero  |    Two-dimensional array     |        +/-      */
  BI_RLE8,            /*         8          |         RLE encoding         |         +       */
  BI_RLE4,            /*         4          |         RLE encoding         |         +       */
  BI_BITFIELDS,       /*    16 and 32  | Two-dim array with color channel masks |   +/−      */
  BI_JPEG,            /*         0          |   In an embedded JPEG file   |         -       */
  BI_PNG,             /*         0          |   In an embedded PNG file    |         -       */
  BI_ALPHABITFIELDS,  /*   16 and 32   | Two-dim array with color channel masks |   +/−      */

  BI_CMYK = 11,       /*             The image is an uncompressed CMYK format.               */
  BI_CMYKRLE8,        /* A CMYK format that uses RLE compr for bitmaps with 8 bits per pixel */
  BI_CMYKRLE4         /* A CMYK format that uses RLE compr for bitmaps with 4 bits per pixel */
};

/**
 * @brief Adds halftoning. Adds RLE and Huffman 1D compression.
 * @brief May contain only the first 16 bytes.
 * @brief Source: https://www.fileformat.info/format/os2bmp/egff.htm
 * 
*/
#pragma pack(push, 1)
struct OS22XBITMAPHEADER : public BITMAPCOREHEADER {
	Compression compression;        /* Bitmap compression scheme */
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
	uint32_t color_encoding;        /* Color model used in bitmap */
	uint32_t identifier;            /* Reserved for application use */


  // EXAMPLE 
  /*
  void print() const override {
    BITMAPCOREHEADER::print(); // Call the base class print method
    std::cout << "OS22XBITMAPHEADER specific fields:\n";
    std::cout << "  Compression: " << compression << "\n";
    std::cout << "  Bitmap Size: " << bitmap_size << " bytes\n";
    std::cout << "  Horizontal Resolution: " << h_px_res << " pixels/meter\n";
    std::cout << "  Vertical Resolution: " << v_px_res << " pixels/meter\n";
    std::cout << "  Colors: " << colors << "\n";
    std::cout << "  Important Colors: " << important_colors << "\n";
    // Add other fields as needed
  }
  */



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
struct BITMAPINFOHEADER : public BITMAPCOREHEADER {
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
struct BITMAPV2INFOHEADER : public BITMAPINFOHEADER {
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
struct BITMAPV3INFOHEADER : public BITMAPV2INFOHEADER {
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
  FXPT2DOT30 xyz_x;
  FXPT2DOT30 xyz_y;
  FXPT2DOT30 xyz_z;
};

/**
 * @brief The CIEXYZTRIPLE structure contains the x,y, and z coordinates
 * @brief of the three colors that correspond to the red, green, and blue
 * @brief endpoints for a specified logical color space.
 * 
*/
struct CIEXYZTRIPLE {
  CIEXYZ xyz_red;
  CIEXYZ xyz_green;
  CIEXYZ xyz_blue;
};

/**
 * @brief Adds color space type and gamma correction.
 * @brief Source: 
 * @brief https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv4header
 * 
*/
#pragma pack(push, 1)
struct BITMAPV4HEADER : public BITMAPV3INFOHEADER  {
  uint32_t type;                  /* Color space type */
  CIEXYZTRIPLE endpoints;      /* Specifies the coordinates of the three colors */
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
struct BITMAPV5HEADER : public BITMAPV4HEADER {
  uint32_t intent;                /* Rendering intent for bitmap */
  uint32_t profile_data;          /* The offset from the start of the header to the profile data.*/
  uint32_t profile_size;          /* Size, in bytes, of embedded profile data */
  uint32_t reserved;              /* This member has been reserved. Value should be set to zero */
};
#pragma pack(pop)


/**
 * @brief Factory class for creating bitmap header.
 * @brief This block of bytes tells the application detailed information about the image,
 * @brief which will be used to display the image on the screen. The block also matches
 * @brief the header used internally by Windows and OS/2 and has several different variants.
 * @brief All of them contain a dword (32-bit) field, specifying their size, so that an
 * @brief application can easily determine which header is used in the image. The reason that
 * @brief there are different headers is that Microsoft extended the DIB format several times.
 * @brief The new extended headers can be used with some GDI functions instead of the older ones,
 * @brief providing more functionality. Since the GDI supports a function for loading bitmap files,
 * @brief typical Windows applications use that functionality. One consequence of this is that for
 * @brief such applications, the BMP formats that they support match the formats supported by the
 * @brief Windows version being run.
 * 
*/
class BitmapHeaderFactory {
  public:
    /**
     * @brief Create a Color Table object based on the DIB header size.
     * @param bitsPerPixel Number of bits per pixel (typically 24 or 32).
     * @return std::unique_ptr<ColorTable> A unique pointer to the created ColorTable object.
    */

   /*
    static std::unique_ptr<ColorTable> createBitmapHeader(uint16_t bitsPerPixel) {
      auto colorTable = std::make_unique<ColorTable>();

      if (bitsPerPixel == 32) {
        // TODO: create Color Table vector 32 bit.
      } else if (bitsPerPixel == 24) {
        // TODO: create Color Table vector 24 bit.
      }
      // TODO: return empty vector.
    }
    */
};


//-------------------------------------------------------------------------------------------------
// Definition of the color table.
//-------------------------------------------------------------------------------------------------
/**
 * @brief Interface for the 2 types of the colors.
 * 
*/
class ColorInterface {
  public:
    virtual ~ColorInterface() = default;
    uint8_t blue;                   /* Blue component */
    uint8_t green;                  /* Green component */
    uint8_t red;                    /* Red component */
};

/**
 * @brief The RGBQUAD structure describes a color consisting of
 * @brief relative intensities of the colors and transparency.
 * @brief Source: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-rgbquad
 * 
*/
#pragma pack(push, 1)
struct RGBQUAD : public ColorInterface{
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
struct RGBTRIPLE : public ColorInterface {};
#pragma pack(pop)


/**
 * @brief Color Table structure.
 * 
*/
class ColorTable {
  private:
    std::vector<std::unique_ptr<ColorInterface>> colors;
  public:
  /**
   * @brief Add color to the vector.
   * 
   * @param color 
  */
    void addColor(std::unique_ptr<ColorInterface> color) {
      colors.push_back(std::move(color));
    }
};


/**
 * @brief Factory class for creating ColorTable.
 * @brief The color table (palette) occurs in the BMP image file directly after the BMP file
 * @brief header, the DIB header, and after the optional three or four bitmasks if the
 * @brief BITMAPINFOHEADER header with BI_BITFIELDS (12 bytes) or BI_ALPHABITFIELDS (16 bytes)
 * @brief option is used.
 * 
*/
class ColorTableFactory {
  public:
    /**
     * @brief Create a Color Table object based on the bits per pixel.
     * @param bitsPerPixel Number of bits per pixel (typically 24 or 32).
     * @return std::unique_ptr<ColorTable> A unique pointer to the created ColorTable object.
    */
    static std::unique_ptr<ColorTable> createColorTable(uint16_t bitsPerPixel) {
      auto colorTable = std::make_unique<ColorTable>();

      if (bitsPerPixel == 32) {
        // TODO: create Color Table vector 32 bit.
      } else if (bitsPerPixel == 24) {
        // TODO: create Color Table vector 24 bit.
      }
      // TODO: return empty vector.
    }
};


//-------------------------------------------------------------------------------------------------
// Definition of the Image Data Pixel Array structure.
//-------------------------------------------------------------------------------------------------






//-------------------------------------------------------------------------------------------------
// Creating structure of the BMP file.
//-------------------------------------------------------------------------------------------------
/**
 * @brief 
 * 
*/
class BMPRDR {
  public:
    BMPRDR(const std::filesystem::path & path) {
      if (std::filesystem::exists(path)) {
        std::ifstream file(path);
        if (file.is_open()) {
          
          // Creating a BITMAPFILEHEADER structure
          bmp_info.file_header = std::make_unique<BITMAPFILEHEADER>();
          file.read(reinterpret_cast<char*>(bmp_info.file_header.get()), BITMAPFILEHEADERLENGTH);
          if (!file) throw std::runtime_error("Error reading BITMAPFILEHEADER");
          if (!bmp_info.file_header->isValid()) throw std::runtime_error("Not a BMP file");







          file.close();
        } else throw std::runtime_error("Failed to open the file");

      } else throw std::runtime_error("File does not exist");
    }

    private:
      struct BMPINFO {
        std::unique_ptr<BITMAPFILEHEADER> file_header;
        std::unique_ptr<HeaderInterface> bitmap_header;
        std::unique_ptr<ColorInterface> color_table;

    };

    BMPINFO bmp_info;
};










#endif // BMP_READER_LIB_HPP
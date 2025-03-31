/**
 * @file bmp_reader.hpp
 * @author Artem Romanov alexromanov322@gmail.com
 * @brief BP - BMP PHOTOS PROBE LIBRARY
 * @version 0.1
 * @date 2024-09-14
 *
 * @copyright Copyright (c) 2024
 */

#ifndef BMP_READER_LIB_HPP
#define BMP_READER_LIB_HPP

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <set>
#include <vector>

//-------------------------------------------------------------------------------------------------
//  Predefined constants.
//-------------------------------------------------------------------------------------------------
/**
 * @brief The header field used to identify the BMP and DIB file is 0x42 0x4D in hexadecimal,
 * same as BM in ASCII. The following entries are possible: <br>
 * BM -> Windows 3.1x, 95, NT, ... etc. <br>
 * BA -> OS/2 struct bitmap array <br>
 * CI -> OS/2 struct color icon <br>
 * CP -> OS/2 const color pointer <br>
 * IC -> OS/2 struct icon <br>
 * PT -> OS/2 pointer
 */
const std::set<uint16_t> validIdentifier = {
    0x4D42,  // BM
    0x4142,  // BA
    0x4349,  // CI
    0x4350,  // CP
    0x4943,  // IC
    0x5450   // PT
};

/**
 * @brief Length of various HEADER's.
 */
constexpr int BITMAPFILEHEADERLENGTH = 14;
constexpr int BITMAPCOREHEADERLENGTH = 12;
constexpr int OS22XBITMAPHEADERLENGTH = 64;
constexpr int BITMAPINFOHEADERLENGTH = 40;
constexpr int BITMAPV2INFOHEADERLENGTH = 52;
constexpr int BITMAPV3INFOHEADERLENGTH = 56;
constexpr int BITMAPV4HEADERLENGTH = 108;
constexpr int BITMAPV5HEADERLENGTH = 124;

//-------------------------------------------------------------------------------------------------
//  Definition of the file header structure.
//-------------------------------------------------------------------------------------------------
#pragma pack(push, 1)
/**
 * @brief Store general information about the bitmap image file. <br>
 * <a href="https://en.wikipedia.org/wiki/BMP_file_format">Source</a>
 */
class BITMAPFILEHEADER {
public:
  [[nodiscard]] bool isValid() const { return validIdentifier.contains(file_type); }

  std::ostream& operator<<(std::ostream& oss) const {
    oss << "BITMAPFILEHEADER: " << "\n";
    oss << "File Type: " << std::hex << file_type << std::dec << "\n";
    oss << "File Size: " << file_size << "\n";
    oss << "Reserved 1: " << reserved1 << "\n";
    oss << "Reserved 2: " << reserved2 << "\n";
    oss << "Offset: " << offset << "\n\n";
    return oss;
  }

  [[nodiscard]] uint16_t getFileType() const { return file_type; }
  [[nodiscard]] uint32_t getFileSize() const { return file_size; }
  [[nodiscard]] uint16_t getReserved1() const { return reserved1; }
  [[nodiscard]] uint16_t getReserved2() const { return reserved2; }
  [[nodiscard]] uint32_t getOffset() const { return offset; }

private:
  uint16_t file_type {0}; /* The header field used to identify the BMP */
  uint32_t file_size {0}; /* The size of the BMP file in bytes */
  uint16_t reserved1 {0}; /* Reserved. Must be set to 0 */
  uint16_t reserved2 {0}; /* Reserved. Must be set to 0 */
  uint32_t offset    {0}; /* Address of the byte where the bitmap data can be found */
};
#pragma pack(pop)

//-------------------------------------------------------------------------------------------------
// Definitions for all the 7 variants of DIB headers and wrappers for them.
//-------------------------------------------------------------------------------------------------
/**
 * @brief Parent class for HEADER's wrappers.
 */
class HeaderInterface {
public:
  virtual ~HeaderInterface() = default;
  virtual std::ostream& operator<<(std::ostream& oss) const = 0;
};

/**
 * @brief We need to use wrapper's classes bc using of inheritance
 * for example (BITMAPCOREHEADER --> HeaderInterface), adds a V-Tables
 * who can spoil our 'packed' structure
 */
template <typename HeaderType>
struct HeaderWrapper final : HeaderInterface {
  explicit HeaderWrapper(const HeaderType& _header) : header(_header) {}
  std::ostream& operator<<(std::ostream& oss) const override {
    header.operator<<(oss);
    return oss;
  }
private:
  const HeaderType header;
};

#pragma pack(push, 1)
/**
 * @brief DIB header (bitmap information header). <br>
 * BITMAPCOREHEADER/OS21XBITMAPHEADER <br>
 * <a href="https://www.fileformat.info/format/os2bmp/egff.htm">Source</a>
 */
struct BITMAPCOREHEADER {
  std::ostream& operator<<(std::ostream& oss) const {
    oss << "DIB HEADER: " << "\n";
    oss << "Header size: " << header_size << "\n";
    oss << "Bitmap width: " << bitmap_width << "\n";
    oss << "Bitmap height: " << bitmap_height << "\n";
    oss << "Color planes: " << color_planes << "\n";
    oss << "Bits per pixel: " << bits_per_pixel << "\n";
    return oss;
  }

  uint32_t header_size{0};    /* Size of this structure in bytes */
  uint32_t bitmap_width{0};   /* Bitmap width in pixels */
  uint32_t bitmap_height{0};  /* Bitmap height in pixel */
  uint16_t color_planes{0};   /* Number of bit planes (color depth) */
  uint16_t bits_per_pixel{0}; /* Number of bits per pixel per plane */
};
#pragma pack(pop)

/**
 * @brief Indexed color images may be compressed with 4-bit or 8-bit RLE or Huffman 1D algorithm. <br>
 * OS/2 BITMAPCOREHEADER2 24bpp images may be compressed with the 24-bit RLE algorithm. <br>
 * The 16bpp and 32bpp images are always stored uncompressed. <br>
 * Note that images in all color depths can be stored without compression if so desired. <br>
 * <a href="https://en.wikipedia.org/wiki/BMP_file_format#Compression">Source</a>
 */
enum Compression : uint32_t {
                          /*      BitCount      |        Pixel Storage         |   Height Sign   */
  BI_RGB = 0,             /*   Any except zero  |    Two-dimensional array     |        +/-      */
  BI_RLE8 = 1,            /*         8          |         RLE encoding         |         +       */
  BI_RLE4 = 2,            /*         4          |         RLE encoding         |         +       */
  BI_BITFIELDS = 3,       /*    16 and 32  | Two-dim array with color channel masks |   +/−      */
  BI_JPEG = 4,            /*         0          |   In an embedded JPEG file   |         -       */
  BI_PNG = 5,             /*         0          |   In an embedded PNG file    |         -       */
  BI_ALPHABITFIELDS = 6,  /*   16 and 32   | Two-dim array with color channel masks |   +/−      */

  BI_CMYK = 11,           /*             The image is an uncompressed CMYK format.               */
  BI_CMYKRLE8 = 12,       /* A CMYK format that uses RLE compr for bitmaps with 8 bits per pixel */
  BI_CMYKRLE4 = 13        /* A CMYK format that uses RLE compr for bitmaps with 4 bits per pixel */
};

#pragma pack(push, 1)
/**
 * @brief Adds halftoning. Adds RLE and Huffman 1D compression. <br>
 * May contain only the first 16 bytes. <br>
 * <a href="https://www.fileformat.info/format/os2bmp/egff.htm">Source</a>
 */
struct OS22XBITMAPHEADER {
  std::ostream& operator<<(std::ostream& oss) const {
    prev_header.operator<<(oss);
    oss << "Compression: " << compression << "\n";
    oss << "Bitmap size: " << bitmap_size << " bytes\n";
    oss << "Horizontal res: " << h_px_res << " pixels/meter\n";
    oss << "Vertical res: " << v_px_res << " pixels/meter\n";
    oss << "Colors used: " << colors << "\n";
    oss << "Important colors: " << important_colors << "\n";
    oss << "Units: " << units << "\n";
    oss << "Recording: " << recording << "\n";
    oss << "Rendering: " << rendering << "\n";
    oss << "Size1: " << size1 << "\n";
    oss << "Size2: " << size2 << "\n";
    oss << "Color encoding: " << color_encoding << "\n";
    oss << "Identifier: " << identifier << "\n";
    return oss;
  }

  BITMAPCOREHEADER prev_header;
  Compression compression;      /* Bitmap compression scheme */
  uint32_t bitmap_size{0};      /* Size of bitmap data in bytes */
  uint32_t h_px_res{0};         /* X resolution of display device */
  uint32_t v_px_res{0};         /* Y resolution of display device */
  uint32_t colors{0};           /* Number of color table indices used */
  uint32_t important_colors{0}; /* Number of important color indices */
  uint16_t units{0};            /* Type of units used to measure resolution */
  uint16_t reserved{0};         /* Pad structure to 4-byte boundary */
  uint16_t recording{0};        /* Recording algorithm */
  uint16_t rendering{0};        /* Halftoning algorithm used */
  uint32_t size1{0};            /* Reserved for halftoning algorithm use */
  uint32_t size2{0};            /* Reserved for halftoning algorithm use */
  uint32_t color_encoding{0};   /* Color model used in bitmap */
  uint32_t identifier{0};       /* Reserved for application use */
};
#pragma pack(pop)

#pragma pack(push, 1)
/**
 * @brief This is the identical structure defined in Windows. <br>
 * Extends bitmap width and height to 4 bytes. Adds 16 bpp and 32 bpp formats. <br>
 * Adds RLE compression. <br>
 * <a href="https://help.accusoft.com/AIMTools/ProgrammersReference/GeneralStructures-BitmapInfoHeader.html">Source</a>
 */
struct BITMAPINFOHEADER {
  std::ostream& operator<<(std::ostream& oss) const {
    prev_header.operator<<(oss);
    oss << "Compression: " << compression << "\n";
    oss << "Bitmap size: " << bitmap_size << " bytes\n";
    oss << "Horizontal res: " << x_px_per_meter << " pixels/meter\n";
    oss << "Vertical res: " << y_px_per_meter << " pixels/meter\n";
    oss << "Colors used: " << colors << "\n";
    oss << "Important colors: " << important_colors << "\n";
    return oss;
  }

  BITMAPCOREHEADER prev_header;
  Compression compression;      /* This value indicates the format of the image */
  uint32_t bitmap_size{0};      /* This value is the size in bytes of the image data */
  int32_t x_px_per_meter{0};    /* Specifies the horizontal print resolution */
  int32_t y_px_per_meter{0};    /* Specifies the vertical print resolution */
  uint32_t colors{0};           /* Number of RGBQUAD elements */
  uint32_t important_colors{0}; /* The first biClrImportant elements of ColorTable */
};
#pragma pack(pop)

#pragma pack(push, 1)
/**
 * @brief Adds RGB bit masks. <br>
 * <a href="https://formats.kaitai.io/bmp/">Source</a>
 */
struct BITMAPV2INFOHEADER {
  std::ostream& operator<<(std::ostream& oss) const {
    prev_header.operator<<(oss);
    oss << "Red mask: " << red_mask << "\n";
    oss << "Green mask: " << green_mask << "\n";
    oss << "Blue mask: " << blue_mask << "\n";
    return oss;
  }

  BITMAPINFOHEADER prev_header;
  uint32_t red_mask{0};   /* Color mask that specifies the 'color' component */
  uint32_t green_mask{0}; /* of each pixel, valid only if the Compression */
  uint32_t blue_mask{0};  /* member is set to BI_BITFIELDS */
};
#pragma pack(pop)

#pragma pack(push, 1)
/**
 * @brief Adds alpha channel bit mask. <br>
 * <a href="https://formats.kaitai.io/bmp/">Source</a>
 */
struct BITMAPV3INFOHEADER {
  std::ostream& operator<<(std::ostream& oss) const {
    prev_header.operator<<(oss);
    oss << "Alpha mask: " << alpha_mask << "\n";
    return oss;
  }

  BITMAPV2INFOHEADER prev_header;
  uint32_t alpha_mask{0}; /* Alpha channel bit mask that specifies the transparency */
};
#pragma pack(pop)

/**
 * @brief FXPT2DOT30 data type. <br>
 * FXPT2DOT30 is a fixed-point data type used in Windows,
 * specifically for color profiles and certain image formats like BMP. <br>
 * It represents a 32-bit signed fixed-point number with: <br>
 * 2 bits for the integer part, <br>
 * 30 bits for the fractional part. <br>
 * We will save this in int32_t and convert this value by ourselves. <br>
 * <a href="https://courses.cs.washington.edu/courses/cse373/00sp/bmp.h">Source</a>
 */
using FXPT2DOT30 = int32_t;

#pragma pack(push, 1)
/**
 * @brief The CIEXYZ structure contains the x,y, and z coordinates
 * of a specific color in a specified color space. <br>
 * Windows structure implementation. <br>
 * <a href="https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-ciexyz">Source</a>
 */
struct CIEXYZ {
  friend std::ostream& operator<<(std::ostream& oss, const CIEXYZ& obj) {
    oss << "X coordinate: " << static_cast<float>(obj.xyz_x) / (1 << 30) << "\n";
    oss << "Y coordinate: " << static_cast<float>(obj.xyz_y) / (1 << 30) << "\n";
    oss << "Z coordinate: " << static_cast<float>(obj.xyz_z) / (1 << 30) << "\n";
    return oss;
  }

  FXPT2DOT30 xyz_x{0};
  FXPT2DOT30 xyz_y{0};
  FXPT2DOT30 xyz_z{0};
};
#pragma pack(pop)

#pragma pack(push, 1)
/**
 * @brief The CIEXYZTRIPLE structure contains the x,y, and z coordinates
 * of the three colors that correspond to the red, green, and blue
 * endpoints for a specified logical color space.
 */
struct CIEXYZTRIPLE {
  friend std::ostream& operator<<(std::ostream& oss, const CIEXYZTRIPLE& obj) {
    oss << "Red color coordinates: " << "\n" << obj.xyz_red;
    oss << "Green color coordinates: " << "\n" << obj.xyz_green;
    oss << "Blue color coordinates: " << "\n" << obj.xyz_blue;
    return oss;
  }

  CIEXYZ xyz_red{0};
  CIEXYZ xyz_green{0};
  CIEXYZ xyz_blue{0};
};
#pragma pack(pop)

#pragma pack(push, 1)
/**
 * @brief Adds color space type and gamma correction. <br>
 * <a href="https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv4header">Source</a>
 */
struct BITMAPV4HEADER {
  std::ostream& operator<<(std::ostream& oss) const {
    prev_header.operator<<(oss);
    oss << "Type: " << type << "\n";
    oss << endpoints;
    oss << "Gamma red: " << gamma_red << "\n";
    oss << "Gamma green: " << gamma_green << "\n";
    oss << "Gamma blue: " << gamma_blue << "\n";
    return oss;
  }

  BITMAPV3INFOHEADER prev_header;
  uint32_t type{0};        /* Color space type */
  CIEXYZTRIPLE endpoints;  /* Specifies the coordinates of the three colors */
  uint32_t gamma_red{0};   /* Gamma red coordinate scale value */
  uint32_t gamma_green{0}; /* Gamma green coordinate scale value */
  uint32_t gamma_blue{0};  /* Gamma blue coordinate scale value */
};
#pragma pack(pop)

#pragma pack(push, 1)
/**
 * @brief Adds ICC color profiles. <br>
 * <a href="https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv5header">Source</a>
 */
struct BITMAPV5HEADER {
  std::ostream& operator<<(std::ostream& oss) const {
    prev_header.operator<<(oss);
    oss << "Intent: " << intent << "\n";
    oss << "Profile data: " << profile_data << "\n";
    oss << "Profile size: " << profile_size << "\n";
    oss << "Reserved: " << reserved << "\n";
    return oss;
  }

  BITMAPV4HEADER prev_header;
  uint32_t intent{0};       /* Rendering intent for bitmap */
  uint32_t profile_data{0}; /* The offset from the start of the header to the profile data.*/
  uint32_t profile_size{0}; /* Size, in bytes, of embedded profile data */
  uint32_t reserved{0};     /* This member has been reserved. Value should be set to zero */
};
#pragma pack(pop)

/**
 * @brief Factory for creating HEADER --> HEADER_WRAPPER --> HeaderInterface structure.
 */
class HeaderFactory {
public:
  static std::unique_ptr<HeaderInterface> createBitmapHeader(std::ifstream& file) {
    // Read the header length
    uint32_t header_size;
    file.read(reinterpret_cast<char*>(&header_size), 4);

    // 4 bytes back
    file.seekg(-4, std::ios::cur);

    std::unique_ptr<HeaderInterface> header = nullptr;

    switch (header_size) {
      case BITMAPCOREHEADERLENGTH: {
        BITMAPCOREHEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPCOREHEADERLENGTH);
        header = std::make_unique<HeaderWrapper<BITMAPCOREHEADER> >(tmp);
        break;
      }

      case OS22XBITMAPHEADERLENGTH: {
        OS22XBITMAPHEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), OS22XBITMAPHEADERLENGTH);
        header = std::make_unique<HeaderWrapper<OS22XBITMAPHEADER> >(tmp);
        break;
      }

      case BITMAPINFOHEADERLENGTH: {
        BITMAPINFOHEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPINFOHEADERLENGTH);
        header = std::make_unique<HeaderWrapper<BITMAPINFOHEADER> >(tmp);
        break;
      }

      case BITMAPV2INFOHEADERLENGTH: {
        BITMAPV2INFOHEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPV2INFOHEADERLENGTH);
        header = std::make_unique<HeaderWrapper<BITMAPV2INFOHEADER> >(tmp);
        break;
      }

      case BITMAPV3INFOHEADERLENGTH: {
        BITMAPV3INFOHEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPV3INFOHEADERLENGTH);
        header = std::make_unique<HeaderWrapper<BITMAPV3INFOHEADER> >(tmp);
        ;
        break;
      }

      case BITMAPV4HEADERLENGTH: {
        BITMAPV4HEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPV4HEADERLENGTH);
        header = std::make_unique<HeaderWrapper<BITMAPV4HEADER> >(tmp);
        break;
      }

      case BITMAPV5HEADERLENGTH: {
        BITMAPV5HEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPV5HEADERLENGTH);
        header = std::make_unique<HeaderWrapper<BITMAPV5HEADER> >(tmp);
        break;
      }

      default:
        throw std::runtime_error("Invalid header size");
    }

    return header;
  }
};

//-------------------------------------------------------------------------------------------------
// Definition of the color table.
//-------------------------------------------------------------------------------------------------
/**
 * @brief Interface for the 2 types of the colors.
 */
class ColorInterface {
public:
  virtual ~ColorInterface() = default;

  uint8_t blue;  /* Blue component */
  uint8_t green; /* Green component */
  uint8_t red;   /* Red component */
};

/**
 * @brief The RGBQUAD structure describes a color consisting of
 * @brief relative intensities of the colors and transparency.
 * @brief Source: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-rgbquad
 *
 */
#pragma pack(push, 1)
struct RGBQUAD : public ColorInterface {
  uint8_t reserved; /* Reserved (often used as Alpha channel) */
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
  std::vector<std::unique_ptr<ColorInterface> > colors;

public:
  /**
   * @brief Add color to the vector.
   * @param color
   */
  void addColor(std::unique_ptr<ColorInterface> color) { colors.push_back(std::move(color)); }
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
  static std::unique_ptr<ColorTable> createColorTable(const uint16_t bitsPerPixel) {
    auto colorTable = std::make_unique<ColorTable>();

    if (bitsPerPixel == 32) {
      // TODO: create Color Table vector 32 bit.
    } else if (bitsPerPixel == 24) {
      // TODO: create Color Table vector 24 bit.
    }
    // TODO: return empty vector.
    return colorTable;
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
class BP {
public:
  explicit BP(const std::filesystem::path& path) {
    if (exists(path)) {
      std::ifstream file(path, std::ios::binary);

      if (!file || !file.is_open() || !file.good()) {
        throw std::runtime_error("Error reading BITMAP FILE HEADER");
      }

      // Creating a BITMAPFILEHEADER structure
      file_header = std::make_unique<BITMAPFILEHEADER>();
      file.read(reinterpret_cast<char*>(file_header.get()), BITMAPFILEHEADERLENGTH);
      if (!file_header->isValid()) {
        throw std::runtime_error("Not a valid BMP file");
      }

      // Creating a DIB HEADER.
      bitmap_header = HeaderFactory::createBitmapHeader(file);

      // Creating a COLOR TABLE

      file.close();
    } else {
      throw std::runtime_error("File does not exist");
    }
  }

  friend std::ostream& operator<<(std::ostream& oss, const BP& src) {
    src.file_header->operator<<(oss);
    src.bitmap_header->operator<<(oss);
    /////// color table
    return oss;
  }

  [[nodiscard]] const std::unique_ptr<BITMAPFILEHEADER>& getFileHeader() const { return file_header; }

  [[nodiscard]] const std::unique_ptr<HeaderInterface>& getBitmapHeader() const { return bitmap_header; }

  [[nodiscard]] const std::unique_ptr<ColorInterface>& getColorTable() const { return color_table; }

private:
  std::unique_ptr<BITMAPFILEHEADER> file_header;
  std::unique_ptr<HeaderInterface> bitmap_header;
  std::unique_ptr<ColorInterface> color_table;
};

#endif  // BMP_READER_LIB_HPP

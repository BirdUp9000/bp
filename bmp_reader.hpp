#ifndef BMP_READER_LIB_HPP
#define BMP_READER_LIB_HPP

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

#include <fstream>
#include <iostream>
#include <cstdint>
#include <vector>
#include <memory>
#include <filesystem>
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

/**
 * @brief Length of various HEADER's.
 * 
*/
constexpr int BITMAPFILEHEADERLENGTH      = 14;
constexpr int BITMAPCOREHEADERLENGTH      = 12;
constexpr int OS22XBITMAPHEADERLENGTH     = 64;
constexpr int BITMAPINFOHEADERLENGTH      = 40;
constexpr int BITMAPV2INFOHEADERLENGTH    = 52;
constexpr int BITMAPV3INFOHEADERLENGTH    = 56;
constexpr int BITMAPV4HEADERLENGTH       = 108;
constexpr int BITMAPV5HEADERLENGTH       = 124;

//-------------------------------------------------------------------------------------------------
//  Definition of the file header structure.
//-------------------------------------------------------------------------------------------------
/**
 * @brief Store general information about the bitmap image file.
 * @brief Source: https://en.wikipedia.org/wiki/BMP_file_format
 * 
*/
#pragma pack(push, 1)
class BITMAPFILEHEADER {
  public:
  [[nodiscard]] bool isValid() const {
    if (!validIdentifier.contains(file_type)) return false;
    return true;
  }

  std::ostream & operator<<(std::ostream & os) const {
    os << "BITMAPFILEHEADER: "  << "\n";
    os << "File Type: "         << std::hex << file_type << std::dec << "\n";
    os << "File Size: "         << file_size << "\n";
    os << "Reserved 1: "        << reserved1 << "\n";
    os << "Reserved 2: "        << reserved2 << "\n";
    os << "Offset: "            << offset << "\n\n";
    return os;
  }

  [[nodiscard]] uint16_t getFileType() const {return file_type;}
  [[nodiscard]] uint32_t getFileSize() const {return file_size;}
  [[nodiscard]] uint16_t getReserved1() const {return reserved1;}
  [[nodiscard]] uint16_t getReserved2() const {return reserved2;}
  [[nodiscard]] uint32_t getOffset() const {return offset;}

  private:
  uint16_t file_type {0};         /* The header field used to identify the BMP */
  uint32_t file_size {0};         /* The size of the BMP file in bytes */
  uint16_t reserved1 {0};         /* Reserved. Must be set to 0 */
  uint16_t reserved2 {0};         /* Reserved. Must be set to 0 */
  uint32_t offset    {0};         /* Address of the byte where the bitmap data can be found */
};
#pragma pack(pop)


//-------------------------------------------------------------------------------------------------
// Definitions for all the 7 variants of DIB headers and wrappers for them.
//-------------------------------------------------------------------------------------------------
/**
 * @brief Parent class for HEADER's wrappers.
 * 
*/
class HeaderInterface {
  public:
  virtual ~HeaderInterface() = default;
  virtual std::ostream & operator<<(std::ostream & os) const = 0;

  //virtual uint32_t getHeaderSize() const = 0;
};

/**
 * @brief DIB header (bitmap information header).
 * @brief BITMAPCOREHEADER/OS21XBITMAPHEADER
 * @brief Source: https://www.fileformat.info/format/os2bmp/egff.htm
 *
*/
#pragma pack(push, 1)
struct BITMAPCOREHEADER {
  std::ostream & operator<<(std::ostream & os) const {
    os << "DIB HEADER: "          << "\n";
    os << "Header size: "         << header_size << "\n";
    os << "Bitmap width: "        << bitmap_width << "\n";
    os << "Bitmap height: "       << bitmap_height << "\n";
    os << "Color planes: "        << color_planes << "\n";
    os << "Bits per pixel: "      << bits_per_pixel << "\n";
    return os;
  }

  uint32_t header_size    {0};    /* Size of this structure in bytes */
  uint32_t bitmap_width   {0};    /* Bitmap width in pixels */
  uint32_t bitmap_height  {0};    /* Bitmap height in pixel */
  uint16_t color_planes   {0};    /* Number of bit planes (color depth) */
  uint16_t bits_per_pixel {0};    /* Number of bits per pixel per plane */
};
#pragma pack(pop)

/**
 * @brief We need to use wrapper's classes bc using of inheritance 
 * @brief for example (BITMAPCOREHEADER --> HeaderInterface), adds a V-Tables
 * @brief who can spoil our 'packed' structure
 * 
*/
template<typename HeaderType>
struct HeaderWrapper final : public HeaderInterface {
  explicit HeaderWrapper(const HeaderType & _header) : header(_header) {}

  std::ostream & operator<<(std::ostream & os) const override {
    header.operator<<(os);
    return os;
  }

private:
  const HeaderType header;
};

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
struct OS22XBITMAPHEADER {
  std::ostream & operator<<(std::ostream & os) const {
    prev_header.operator<<(os);
    os << "Compression: "         << compression << "\n";
    os << "Bitmap size: "         << bitmap_size << " bytes\n";
    os << "Horizontal res: "      << h_px_res << " pixels/meter\n";
    os << "Vertical res: "        << v_px_res << " pixels/meter\n";
    os << "Colors used: "         << colors << "\n";
    os << "Important colors: "    << important_colors << "\n";
    os << "Units: "               << units << "\n";
    os << "Recording: "           << recording << "\n";
    os << "Rendering: "           << rendering << "\n";
    os << "Size1: "               << size1 << "\n";
    os << "Size2: "               << size2 << "\n";
    os << "Color encoding: "      << color_encoding << "\n";
    os << "Identifier: "          << identifier << "\n";
    return os;
  }

  BITMAPCOREHEADER prev_header;
	Compression compression;        /* Bitmap compression scheme */
	uint32_t bitmap_size      {0};  /* Size of bitmap data in bytes */
	uint32_t h_px_res         {0};  /* X resolution of display device */
	uint32_t v_px_res         {0};  /* Y resolution of display device */
	uint32_t colors           {0};  /* Number of color table indices used */
	uint32_t important_colors {0};  /* Number of important color indices */
	uint16_t units            {0};  /* Type of units used to measure resolution */
	uint16_t reserved         {0};  /* Pad structure to 4-byte boundary */
	uint16_t recording        {0};  /* Recording algorithm */
	uint16_t rendering        {0};  /* Halftoning algorithm used */
	uint32_t size1            {0};  /* Reserved for halftoning algorithm use */
	uint32_t size2            {0};  /* Reserved for halftoning algorithm use */
	uint32_t color_encoding   {0};  /* Color model used in bitmap */
	uint32_t identifier       {0};  /* Reserved for application use */
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
  std::ostream & operator<<(std::ostream & os) const {
    prev_header.operator<<(os);
    os << "Compression: "         << compression << "\n";
    os << "Bitmap size: "         << bitmap_size << " bytes\n";
    os << "Horizontal res: "      << x_px_per_meter << " pixels/meter\n";
    os << "Vertical res: "        << y_px_per_meter << " pixels/meter\n";
    os << "Colors used: "         << colors << "\n";
    os << "Important colors: "    << important_colors << "\n";
    return os;
  }

  BITMAPCOREHEADER prev_header;
  Compression compression;        /* This value indicates the format of the image */
  uint32_t bitmap_size      {0};  /* This value is the size in bytes of the image data */
  int32_t  x_px_per_meter   {0};  /* Specifies the horizontal print resolution */
  int32_t  y_px_per_meter   {0};  /* Specifies the vertical print resolution */
  uint32_t colors           {0};  /* Number of RGBQUAD elements */
  uint32_t important_colors {0};  /* The first biClrImportant elements of ColorTable */
};
#pragma pack(pop)

/**
 * @brief Adds RGB bit masks.
 * @brief Source: https://formats.kaitai.io/bmp/
*/
#pragma pack(push, 1)
struct BITMAPV2INFOHEADER {
  std::ostream & operator<<(std::ostream & os) const {
    prev_header.operator<<(os);
    os << "Red mask: "            << red_mask << "\n";
    os << "Green mask: "          << green_mask << "\n";
    os << "Blue mask: "           << blue_mask << "\n";
    return os;
  }

  BITMAPINFOHEADER prev_header;
  uint32_t red_mask         {0};  /* Color mask that specifies the 'color' component */
  uint32_t green_mask       {0};  /* of each pixel, valid only if the Compression */
  uint32_t blue_mask        {0};  /* member is set to BI_BITFIELDS */
};
#pragma pack(pop)

/**
 * @brief Adds alpha channel bit mask.
 * @brief Source: https://formats.kaitai.io/bmp/
 * 
*/
#pragma pack(push, 1)
struct BITMAPV3INFOHEADER {
  std::ostream & operator<<(std::ostream & os) const {
    prev_header.operator<<(os);
    os << "Alpha mask: "            << alpha_mask << "\n";
    return os;
  }

  BITMAPV2INFOHEADER prev_header;
  uint32_t alpha_mask       {0};  /* Alpha channel bit mask that specifies the transparency */
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
#pragma pack(push, 1)
struct CIEXYZ {
  friend std::ostream & operator<<(std::ostream & os, const CIEXYZ & obj) {
    os << "X coordinate: "    << static_cast<float>(obj.xyz_x) / (1 << 30) << "\n";
    os << "Y coordinate: "    << static_cast<float>(obj.xyz_y) / (1 << 30) << "\n";
    os << "Z coordinate: "    << static_cast<float>(obj.xyz_z) / (1 << 30) << "\n";
    return os;
  }

  FXPT2DOT30 xyz_x {0};
  FXPT2DOT30 xyz_y {0};
  FXPT2DOT30 xyz_z {0};
};
#pragma pack(pop)

/**
 * @brief The CIEXYZTRIPLE structure contains the x,y, and z coordinates
 * @brief of the three colors that correspond to the red, green, and blue
 * @brief endpoints for a specified logical color space.
 * 
*/
#pragma pack(push, 1)
struct CIEXYZTRIPLE {
  friend std::ostream & operator<<(std::ostream & os, const CIEXYZTRIPLE & obj) {
    os << "Red color coordinates: "       << "\n"<< obj.xyz_red;
    os << "Green color coordinates: "     << "\n"<< obj.xyz_green;
    os << "Blue color coordinates: "      << "\n"<< obj.xyz_blue;
    return os;
  }

  CIEXYZ xyz_red    {0};
  CIEXYZ xyz_green  {0};
  CIEXYZ xyz_blue   {0};
};
#pragma pack(pop)

/**
 * @brief Adds color space type and gamma correction.
 * @brief Source: 
 * @brief https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv4header
 * 
*/
#pragma pack(push, 1)
struct BITMAPV4HEADER {
  std::ostream & operator<<(std::ostream & os) const {
    prev_header.operator<<(os);
    os << "Type: "             << type << "\n";
    os <<                         endpoints;
    os << "Gamma red: "        << gamma_red << "\n";
    os << "Gamma green: "      << gamma_green << "\n";
    os << "Gamma blue: "       << gamma_blue << "\n";
    return os;
  }

  BITMAPV3INFOHEADER prev_header;
  uint32_t type             {0};  /* Color space type */
  CIEXYZTRIPLE endpoints;         /* Specifies the coordinates of the three colors */
  uint32_t gamma_red        {0};  /* Gamma red coordinate scale value */
  uint32_t gamma_green      {0};  /* Gamma green coordinate scale value */
  uint32_t gamma_blue       {0};  /* Gamma blue coordinate scale value */
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
  std::ostream & operator<<(std::ostream & os) const {
    prev_header.operator<<(os);
    os << "Intent: "            << intent << "\n";
    os << "Profile data: "      << profile_data << "\n";
    os << "Profile size: "      << profile_size << "\n";
    os << "Reserved: "          << reserved << "\n";
    return os;
  }

  BITMAPV4HEADER prev_header;
  uint32_t intent           {0};  /* Rendering intent for bitmap */
  uint32_t profile_data     {0};  /* The offset from the start of the header to the profile data.*/
  uint32_t profile_size     {0};  /* Size, in bytes, of embedded profile data */
  uint32_t reserved         {0};  /* This member has been reserved. Value should be set to zero */
};
#pragma pack(pop)

/**
 * @brief Factroy for creating HEADER --> HEADER_WRAPPER --> HeaderInterface structure.
 * 
*/
class HeaderFactory {
  public:
  static std::unique_ptr<HeaderInterface> createBitmapHeader(std::ifstream & file) {
    // Read the header length
    uint32_t header_size;
    file.read(reinterpret_cast<char*>(&header_size), 4);

    // 4 bytes back
    file.seekg(-4, std::ios::cur);

    std::unique_ptr<HeaderInterface> header = nullptr;

    switch (header_size) {
      case BITMAPCOREHEADERLENGTH : {
        BITMAPCOREHEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPCOREHEADERLENGTH);
        header = std::make_unique<HeaderWrapper<BITMAPCOREHEADER>>(tmp);
        break;
      }

      case OS22XBITMAPHEADERLENGTH : {
        OS22XBITMAPHEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), OS22XBITMAPHEADERLENGTH);
        header = std::make_unique<HeaderWrapper<OS22XBITMAPHEADER>>(tmp);
        break;
      }

      case BITMAPINFOHEADERLENGTH : {
        BITMAPINFOHEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPINFOHEADERLENGTH);
        header = std::make_unique<HeaderWrapper<BITMAPINFOHEADER>>(tmp);
        break;
      }

      case BITMAPV2INFOHEADERLENGTH : {
        BITMAPV2INFOHEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPV2INFOHEADERLENGTH);
        header = std::make_unique<HeaderWrapper<BITMAPV2INFOHEADER>>(tmp);
        break;
      }

      case BITMAPV3INFOHEADERLENGTH : {
        BITMAPV3INFOHEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPV3INFOHEADERLENGTH);
        header = std::make_unique<HeaderWrapper<BITMAPV3INFOHEADER>>(tmp);;
        break;
      }

      case BITMAPV4HEADERLENGTH : {
        BITMAPV4HEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPV4HEADERLENGTH);
        header = std::make_unique<HeaderWrapper<BITMAPV4HEADER>>(tmp);
        break;
      }

      case BITMAPV5HEADERLENGTH : {
        BITMAPV5HEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPV5HEADERLENGTH);
        header = std::make_unique<HeaderWrapper<BITMAPV5HEADER>>(tmp);
        break;
      }

      default : throw std::runtime_error("Invalid header size");
    }

    return header;
  }

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
    static std::unique_ptr<ColorTable> createColorTable(const uint16_t bitsPerPixel) {
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
class BMPINFO {
  public:
  explicit BMPINFO(const std::filesystem::path & path) {
    if (std::filesystem::exists(path)) {
      std::ifstream file(path, std::ios::binary);
      if (file.is_open()) {
          
        // Creating a BITMAPFILEHEADER structure
        if (!file) throw std::runtime_error("Error reading BITMAP FILE HEADER");
        file_header = std::make_unique<BITMAPFILEHEADER>();
        file.read(reinterpret_cast<char*>(file_header.get()), BITMAPFILEHEADERLENGTH);
        if (!file_header->isValid()) throw std::runtime_error("Not a valid BMP file");

        // Creating a DIB HEADER.
        if (!file) throw std::runtime_error("Error reading DIB HEADER");
        bitmap_header = HeaderFactory::createBitmapHeader(file);

        // Creating a COLOR TABLE
        if (!file) throw std::runtime_error("Error reading COLOR TABLE");

        //



        file.close();
      } else throw std::runtime_error("Failed to open the file");

    } else throw std::runtime_error("File does not exist");
  }
  
  friend std::ostream & operator<<(std::ostream & os, const BMPINFO & src) {
    src.file_header->operator<<(os);
    src.bitmap_header->operator<<(os);
    /////// color table
    return os;
  }

  [[nodiscard]] const std::unique_ptr<BITMAPFILEHEADER> & getFileHeader() const {
    return file_header;
  }

  [[nodiscard]] const std::unique_ptr<HeaderInterface> & getBitmapHeader() const {
    return bitmap_header;
  }

  [[nodiscard]] const std::unique_ptr<ColorInterface> & getColorTable() const {
    return color_table;
  }

  private:
  std::unique_ptr<BITMAPFILEHEADER> file_header;
  std::unique_ptr<HeaderInterface> bitmap_header;
  std::unique_ptr<ColorInterface> color_table;
};










#endif // BMP_READER_LIB_HPP

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

/**
 * @brief 
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
  BITMAPFILEHEADER() = default;

  bool isValid() const {
    if (validIdentifier.find(file_type) == validIdentifier.end()) return false;
    return true;
  }

  friend std::ostream & operator<<(std::ostream & os, const BITMAPFILEHEADER & header) {
    os << "BITMAPFILEHEADER: "  << "\n";
    os << "File Type: "         << std::hex << header.file_type << std::dec << "\n";
    os << "File Size: "         << header.file_size << "\n";
    os << "Reserved 1: "        << header.reserved1 << "\n";
    os << "Reserved 2: "        << header.reserved2 << "\n";
    os << "Offset: "            << header.offset << "\n";
    return os;
  }

  uint16_t getFileType() const {return file_type;}
  uint32_t getFileSize() const {return file_size;}
  uint16_t getReserved1() const {return reserved1;}
  uint16_t getReserved2() const {return reserved2;}
  uint32_t getOffset() const {return offset;}

  private:
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
struct HeaderInterface {
  virtual std::ostream & operator<<(std::ostream & os) const = 0;
  virtual ~HeaderInterface() = default;
};

/**
 * @brief DIB header (bitmap information header).
 * @brief BITMAPCOREHEADER/OS21XBITMAPHEADER
 * @brief Source: https://www.fileformat.info/format/os2bmp/egff.htm
 *
*/
#pragma pack(push, 1)
struct BITMAPCOREHEADER {
  uint32_t header_size    = 0;    /* Size of this structure in bytes */
  uint32_t bitmap_width   = 0;    /* Bitmap width in pixels */
  uint32_t bitmap_height  = 0;    /* Bitmap height in pixel */
  uint16_t color_planes   = 0;    /* Number of bit planes (color depth) */
  uint16_t bits_per_pixel = 0;    /* Number of bits per pixel per plane */
};
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
	uint32_t bitmap_size      = 0;  /* Size of bitmap data in bytes */
	uint32_t h_px_res         = 0;  /* X resolution of display device */
	uint32_t v_px_res         = 0;  /* Y resolution of display device */
	uint32_t colors           = 0;  /* Number of color table indices used */
	uint32_t important_colors = 0;  /* Number of important color indices */
	uint16_t units            = 0;  /* Type of units used to measure resolution */
	uint16_t reserved         = 0;  /* Pad structure to 4-byte boundary */
	uint16_t recording        = 0;  /* Recording algorithm */
	uint16_t rendering        = 0;  /* Halftoning algorithm used */
	uint32_t size1            = 0;  /* Reserved for halftoning algorithm use */
	uint32_t size2            = 0;  /* Reserved for halftoning algorithm use */
	uint32_t color_encoding   = 0;  /* Color model used in bitmap */
	uint32_t identifier       = 0;  /* Reserved for application use */
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
  Compression compression;       /* This value indicates the format of the image */
  uint32_t bitmap_size;          /* This value is the size in bytes of the image data */
  int32_t  x_px_per_meter;       /* Specifies the horizontal print resolution */
  int32_t  y_px_per_meter;       /* Specifies the vertical print resolution */
  uint32_t colors;               /* Number of RGBQUAD elements */
  uint32_t important_colors;     /* The first biClrImportant elements of ColorTable */
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
  friend std::ostream & operator<<(std::ostream & os, const CIEXYZ & obj) {
    os << "X coordinate: "    << static_cast<float>(obj.xyz_x) / (1 << 30) << "\n";
    os << "Y coordinate: "    << static_cast<float>(obj.xyz_y) / (1 << 30) << "\n";
    os << "Z coordinate: "    << static_cast<float>(obj.xyz_z) / (1 << 30) << "\n";
    return os;
  }

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
  friend std::ostream & operator<<(std::ostream & os, const CIEXYZTRIPLE & obj) {
    os << "Red color coordinates: "       << obj.xyz_red << "\n";
    os << "Green color coordinates: "     << obj.xyz_green << "\n";
    os << "Blue color coordinates: "      << obj.xyz_blue << "\n";
    return os;
  }

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
struct BITMAPV4HEADER : public BITMAPV3INFOHEADER {
  uint32_t type;                  /* Color space type */
  CIEXYZTRIPLE endpoints;         /* Specifies the coordinates of the three colors */
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


//-------------------------------------------------------------------------------------------------
// Wrappers for HEADER's structures.
//-------------------------------------------------------------------------------------------------
/**
 * @brief We need to use wrappers bc V-Tables in classes destroy alignment of the fields.
 * 
*/
struct BITMAPCOREHEADERWRAPPER : public HeaderInterface {
  explicit BITMAPCOREHEADERWRAPPER(const BITMAPCOREHEADER & _header) : header(_header) {}

  std::ostream& operator<<(std::ostream& os) const override {
    os << "DIB HEADER: "        << "\n";
    os << "Header size: "       << std::hex << header.header_size << std::dec << "\n";
    os << "Bitmap width: "      << header.bitmap_width << "\n";
    os << "Bitmap height: "     << header.bitmap_height << "\n";
    os << "Color planes: "      << header.color_planes << "\n";
    os << "Bits per pixel: "    << header.bits_per_pixel << "\n";
    return os;
  }

private:
  BITMAPCOREHEADER header;
};

/**
 * @brief OS22XBITMAPHEADER wrapper.
 * 
*/
struct OS22XBITMAPHEADERWRAPPER : public BITMAPCOREHEADERWRAPPER {
  explicit OS22XBITMAPHEADERWRAPPER(const OS22XBITMAPHEADER & _header) : header(_header) {}

  std::ostream & operator<<(std::ostream & os) const override {
    BITMAPCOREHEADERWRAPPER::operator<<(os);
    os << "Compression: "               << header.compression << "\n";
    os << "Bitmap size: "               << header.bitmap_size << "\n";
    os << "Horizontal resolution: "     << header.h_px_res << "\n";
    os << "Vertical resolution: "       << header.v_px_res << "\n";
    os << "Colors: "                    << header.colors << "\n";
    os << "Important colors: "          << header.important_colors << "\n";
    os << "Units: "                     << header.units << "\n";
    os << "Reserved: "                  << header.reserved << "\n";
    os << "Recording: "                 << header.recording << "\n";
    os << "Rendering: "                 << header.rendering << "\n";
    os << "Size1: "                     << header.size1 << "\n";
    os << "Size2: "                     << header.size2 << "\n";
    os << "Color encoding: "            << header.color_encoding << "\n";
    os << "Identifier: "                << header.identifier << "\n";
    return os;
  }

private:
  OS22XBITMAPHEADER header;
};

/**
 * @brief BITMAPINFOHEADER wrapper.
 * 
*/
struct BITMAPINFOHEADERWRAPPER : public BITMAPCOREHEADERWRAPPER {
  explicit BITMAPINFOHEADERWRAPPER(const BITMAPINFOHEADER & _header) : header(_header) {}

  std::ostream & operator<<(std::ostream & os) const override {
    BITMAPCOREHEADERWRAPPER::operator<<(os);
    os << "Compression: "                     << header.compression << "\n";
    os << "Bitmap size: "                     << header.bitmap_size << "\n";
    os << "Horizontal pixels per meter: "     << header.x_px_per_meter << "\n";
    os << "Vertical pixels per meter: "       << header.y_px_per_meter << "\n";
    os << "Colors: "                          << header.colors << "\n";
    os << "Important colors: "                << header.important_colors << "\n";
    return os;
  }

private:
  BITMAPINFOHEADER header;
};

/**
 * @brief BITMAPV2INFOHEADER wrapper.
 * 
*/
struct BITMAPV2INFOHEADERWRAPPER : public BITMAPINFOHEADERWRAPPER {
  explicit BITMAPV2INFOHEADERWRAPPER(const BITMAPV2INFOHEADER & _header) : header(_header) {}

  std::ostream & operator<<(std::ostream & os) const override {
    BITMAPINFOHEADERWRAPPER::operator<<(os);
    os << "Red mask: "                        << header.red_mask << "\n";
    os << "Green mask: "                      << header.green_mask << "\n";
    os << "Blue mask: "                       << header.blue_mask << "\n";
    return os;
  }

private:
  BITMAPV2INFOHEADER header;
};

/**
 * @brief BITMAPV3INFOHEADER wrapper.
 * 
*/
struct BITMAPV3INFOHEADERWRAPPER : public BITMAPV2INFOHEADERWRAPPER {
  explicit BITMAPV3INFOHEADERWRAPPER(const BITMAPV3INFOHEADER & _header) : header(_header) {}

  std::ostream & operator<<(std::ostream & os) const override {
    BITMAPV2INFOHEADERWRAPPER::operator<<(os);
    os << "Alpha mask: "          << header.alpha_mask << "\n";
    return os;
  }

private:
  BITMAPV3INFOHEADER header;
};

/**
 * @brief BITMAPV4HEADER wrapper.
 * 
*/
struct BITMAPV4HEADERWRAPPER : public BITMAPV3INFOHEADERWRAPPER {
  explicit BITMAPV4HEADERWRAPPER(const BITMAPV4HEADER & _header) : header(_header) {}

  std::ostream & operator<<(std::ostream & os) const override {
    BITMAPV3INFOHEADERWRAPPER::operator<<(os);
    os << "Type: "              << header.type << "\n";
    os << "Endpoints: "         << header.endpoints << "\n";
    os << "Gamma red: "         << header.gamma_red << "\n";
    os << "Gamma green: "       << header.gamma_green << "\n";
    os << "Gamma blue: "        << header.gamma_blue << "\n";
    return os;
  }

private:
  BITMAPV4HEADER header;
};

/**
 * @brief BITMAPV5HEADER wrapper.
 * 
*/
struct BITMAPV5HEADERWRAPPER : public BITMAPV4HEADERWRAPPER {
  explicit BITMAPV5HEADERWRAPPER(const BITMAPV5HEADER & _header) : header(_header) {}

  std::ostream & operator<<(std::ostream & os) const override {
    BITMAPV4HEADERWRAPPER::operator<<(os);
    os << "Intent: "              << header.intent << "\n";
    os << "Profile data: "        << header.profile_data << "\n";
    os << "Profile size: "        << header.profile_size << "\n";
    os << "Reserved: "            << header.reserved << "\n";
    return os;
  }

private:
  BITMAPV5HEADER header;
};


//-------------------------------------------------------------------------------------------------
// Factory class for creating bitmap header.
//-------------------------------------------------------------------------------------------------
/**
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
   * @brief Create a Bitmap Header object
   * 
   * @param file 
   * @return std::unique_ptr<HeaderInterface> 
  */
  static std::unique_ptr<HeaderInterface> createBitmapHeader(std::ifstream & file) {
    if (!file) throw std::runtime_error("Error reading BITMAP HEADER");

    // Read a Header length
    uint32_t header_size;
    file.read(reinterpret_cast<char*>(&header_size), 4);
    std::cout << "DEBUG: " << header_size << std::endl;

    // 4 bytes back
    file.seekg(-4, std::ios::cur);

    std::unique_ptr<HeaderInterface> header = nullptr;

    switch (header_size) {
      case BITMAPCOREHEADERLENGTH : {
        BITMAPCOREHEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPCOREHEADERLENGTH);
        header = std::make_unique<BITMAPCOREHEADERWRAPPER>(tmp);
        break;
      }

      case OS22XBITMAPHEADERLENGTH : {
        OS22XBITMAPHEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), OS22XBITMAPHEADERLENGTH);
        header = std::make_unique<OS22XBITMAPHEADERWRAPPER>(tmp);
        break;
      }

      case BITMAPINFOHEADERLENGTH : {
        BITMAPINFOHEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPINFOHEADERLENGTH);
        header = std::make_unique<BITMAPINFOHEADERWRAPPER>(tmp);
        break;
      }

      case BITMAPV2INFOHEADERLENGTH : {
        BITMAPV2INFOHEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPV2INFOHEADERLENGTH);
        header = std::make_unique<BITMAPV2INFOHEADERWRAPPER>(tmp);
        break;
      }

      case BITMAPV3INFOHEADERLENGTH : {
        BITMAPV3INFOHEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPV3INFOHEADERLENGTH);
        header = std::make_unique<BITMAPV3INFOHEADERWRAPPER>(tmp);
        break;
      }

      case BITMAPV4HEADERLENGTH : {
        BITMAPV4HEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPV4HEADERLENGTH);
        header = std::make_unique<BITMAPV4HEADERWRAPPER>(tmp);
        break;
      }

      case BITMAPV5HEADERLENGTH : {
        BITMAPV5HEADER tmp;
        file.read(reinterpret_cast<char*>(&tmp), BITMAPV5HEADERLENGTH);
        header = std::make_unique<BITMAPV5HEADERWRAPPER>(tmp);
        break;
      }

      default : throw std::runtime_error ("Invalid header size");  
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
class BMPINFO {
  public:
  BMPINFO(const std::filesystem::path & path) {
    if (std::filesystem::exists(path)) {
      std::ifstream file(path, std::ios::binary);
      if (file.is_open()) {
          
        // Creating a BITMAPFILEHEADER structure
        file_header = std::make_unique<BITMAPFILEHEADER>();
        file.read(reinterpret_cast<char*>(file_header.get()), BITMAPFILEHEADERLENGTH);
        if (!file) throw std::runtime_error("Error reading BITMAPFILEHEADER");
        if (!file_header->isValid()) throw std::runtime_error("Not a BMP file");

        // Creating a DIB HEADER.
        bitmap_header = std::move(BitmapHeaderFactory::createBitmapHeader(file));

        // Creating a COLOR TABLE


        //



        file.close();
      } else throw std::runtime_error("Failed to open the file");

    } else throw std::runtime_error("File does not exist");
  }
  
  std::ostream & operator<<(std::ostream & os) {
    os << file_header.get();
    os << bitmap_header.get();
    /////// color table
    return os;
  }

  const std::unique_ptr<BITMAPFILEHEADER> & getFileHeader() const {
    return file_header;
  }

  const std::unique_ptr<HeaderInterface> & getBitmapHeader() const {
    return bitmap_header;
  }

  const std::unique_ptr<ColorInterface> & getColorTable() const {
    return color_table;
  }

  private:
  std::unique_ptr<BITMAPFILEHEADER> file_header;
  std::unique_ptr<HeaderInterface> bitmap_header;
  std::unique_ptr<ColorInterface> color_table;
};










#endif // BMP_READER_LIB_HPP
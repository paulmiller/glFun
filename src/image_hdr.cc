#include "image_hdr.h"

#include "ohno.h"
#include "util.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <istream>
#include <vector>

/* example to convert HDR to PNG:

  std::ifstream test_hdr_input(
    "probes/uffizi_probe.hdr", std::ifstream::binary);
  Image testHdr;
  try {
    testHdr = read_hdr(test_hdr_input);
  } catch(OhNo ohno) {
    std::cout << ohno << std::endl;
  }
  assert(testHdr.type() == Pixel::RGBE8_T);
  float max = -INFINITY;
  float min = INFINITY;
  Image test_f(testHdr.width(), testHdr.height(), Pixel::RGBf_T);
  for(int r = 0; r < testHdr.height(); r++) {
    for(int c = 0; c < testHdr.width(); c++) {
      Pixel::RGBE8* src = static_cast<Pixel::RGBE8*>(testHdr.getPixelPtr(r, c));
      Pixel::RGBf*  dst = static_cast<Pixel::RGBf* >(test_f.getPixelPtr(r, c));
      dst->R = float(src->R) * pow(2.0f, float(src->E) - 100);
      dst->G = float(src->G) * pow(2.0f, float(src->E) - 100);
      dst->B = float(src->B) * pow(2.0f, float(src->E) - 100);
      if(std::isfinite(dst->R)) {
        max=std::max(max,dst->R); min=std::min(min,dst->R); }
      if(std::isfinite(dst->G)) {
        max=std::max(max,dst->G); min=std::min(min,dst->G); }
      if(std::isfinite(dst->B)) {
        max=std::max(max,dst->B); min=std::min(min,dst->B); }
    }
  }
  std::cout << "min=" << min << " max=" << max << '\n';
  for(int r = 0; r < testHdr.height(); r++) {
    for(int c = 0; c < testHdr.width(); c++) {
      Pixel::RGBf*  dst = static_cast<Pixel::RGBf* >(test_f.getPixelPtr(r, c));
      dst->R /= max;
      dst->G /= max;
      dst->B /= max;
    }
  }
  Image test_png(testHdr.width(), testHdr.height(), Pixel::RGB8_T);
  for(int r = 0; r < testHdr.height(); r++) {
    for(int c = 0; c < testHdr.width(); c++) {
      Pixel::RGBf* src = static_cast<Pixel::RGBf*>(test_f.getPixelPtr(r, c));
      Pixel::RGB8* dst = static_cast<Pixel::RGB8*>(test_png.getPixelPtr(r, c));
      dst->R = uint8_t(src->R * 255.0f);
      dst->G = uint8_t(src->G * 255.0f);
      dst->B = uint8_t(src->B * 255.0f);
    }
  }
  std::ofstream test_out("test.png", std::ifstream::binary);
  writePng(test_out, test_png);
*/

namespace {
  const char HDR_SIG[] = "#?RADIANCE";
  const size_t HDR_SIG_BYTES = sizeof(HDR_SIG) - 1;

  bool isNewRLEBeginCode(Pixel::RGBE8 code) {
    // in the new RLE scheme, a line starts with 2 bytes set to 2, and then the
    // upper and then lower byte of the image width, which must be < 0x8000
    return code.R == 2 && code.G == 2 && code.B < 0x80;
  }

  bool isOldRLERepeatCode(Pixel::RGBE8 code) {
    // in the old RLE scheme, a run was indicated by a pixel with all channels
    // set to 1
    return code.R == 1 && code.G == 1 && code.B == 1;
  }

  bool isNormalized(Pixel::RGBE8 px) {
    // the most-significant bit should be set in at least 1 channel
    return (px.R | px.G | px.B) & 0x80;
  }

  // read a scanline of pixel data according to the new RLE scheme, not
  // counting the begin code
  void scanNewRLE(std::istream& input, std::vector<Pixel::RGBE8>& scanline) {
    int scanline_size = scanline.size();
    for(int channel = 0; channel < 4; channel++) {
      for(int i = 0; i < scanline_size;) {
        uint8_t code;
        input.read(reinterpret_cast<char*>(&code), sizeof(code));
        if(input.fail())
          throw OHNO("HDR read code failed");
        int length = (code > 0x80) ? (code & 0x7f) : code;
        if(i + length > scanline_size)
          throw OHNO("HDR new RLE overrun");
        // The Radiance filefmts.pdf claims a code with the high bit set
        // indicates a run. The Radiance code in color.c shows this is not
        // quite correct; it uses (code > 128) to check for a run, meaning a
        // byte with the high bit AND at least 1 other bit set indicates a
        // run. A code of exactly 128 is a non-run.
        if(code > 0x80) {
          // run
          uint8_t value;
          input.read(reinterpret_cast<char*>(&value), sizeof(value));
          if(input.fail())
            throw OHNO("HDR read fail during run");
          for(int j = 0; j < length; j++)
            scanline[i++][channel] = value;
        } else {
          // non-run
          uint8_t value;
          for(int j = 0; j < length; j++) {
            input.read(reinterpret_cast<char*>(&value), sizeof(value));
            if(input.fail())
              throw OHNO("HDR read fail during non-run");
            scanline[i++][channel] = value;
          }
        }
      }
    }
  }

  // read a scanline of pixel data
  void scanRLE(std::istream& input, std::vector<Pixel::RGBE8>& scanline) {
    Pixel::RGBE8 next;
    int scanline_size = scanline.size();
    for(int i = 0; i < scanline_size; i++) {
      input.read(reinterpret_cast<char*>(&next), sizeof(next));
      if(input.fail())
        throw OHNO("HDR next pixel read fail");

      if(isNewRLEBeginCode(next)) {
        if(i != 0)
          throw OHNO("HDR new RLE indicator not at start of line");
        if(scanline_size != ((next.B << 8) | next.E))
          throw OHNO("HDR new RLE wrong length");
        scanNewRLE(input, scanline);
        return;
      }

      isOldRLERepeatCode(next);
        throw OHNO("HDR old RLE not implemented");

      scanline[i] = next;
    }
  }
}

Image read_hdr(std::istream &input) {
  // check signature
  char sig[HDR_SIG_BYTES];
  input.read(sig, HDR_SIG_BYTES);
  if(!input.good())
    throw OHNO("couldn't read HDR signature");
  if(strncmp(sig, HDR_SIG, HDR_SIG_BYTES))
    throw OHNO("HDR signature doesn't match");

  static const int MAX_LINE = 4096;
  char line[MAX_LINE];

  // header FORMAT values
  char format_str[MAX_LINE];
  enum {
    NONE,
    RGBE,
    XYZE
  } format;
  format = NONE;

  // header EXPOSURE values
  float exposure;
  float exposure_total = 1;

  // header COLORCORR values
  float r_corr, g_corr, b_corr;
  float r_corr_total = 1, g_corr_total = 1, b_corr_total = 1;

  // resolution values
  char sign1[2], sign2[2], axis1[2], axis2[2];
  unsigned size1, size2;

  // read header and resolution
  while(true) {
    // TODO binary: need to handle '\r'?
    input.getline(line, sizeof(line));

    if(input.eof())
      throw OHNO("HDR resolution not found");

    if (input.fail())
      throw OHNO("HDR header read fail");

    if(strlen(line) == 0) {
      // ignore blank lines
    } else if(
      hasPrefix("SOFTWARE", line) ||
      hasPrefix("PIXASPECT", line) ||
      hasPrefix("VIEW", line) ||
      hasPrefix("PRIMARIES", line)
    ) {
      // ignore these header lines
    } else if(1 == sscanf(line, "FORMAT= %s", format_str)) {
      // exactly 1 FORMAT line is required
      if(format != NONE)
        throw OHNO("HDR has multiple FORMAT lines");

      if(!strcmp("32-bit_rle_rgbe", format_str))
        format = RGBE;
      else if(!strcmp("32-bit_rle_xyze", format_str))
        format = XYZE;
      else
        throw OHNO("HDR has unrecognized FORMAT");
    } else if(1 == sscanf(line, "EXPOSURE= %f", &exposure)) {
      // any number of EXPOSURE lines are allowed
      exposure_total *= exposure;
    } else if(3 == sscanf(line, "COLORCORR= %f %f %f",
        &r_corr, &g_corr, &b_corr)) {
      // any number of COLORCORR lines are allowed
      r_corr_total *= r_corr;
      g_corr_total *= g_corr;
      b_corr_total *= b_corr;
    } else if(6 == sscanf(line, "%1[-+]%1[XY] %u %1[-+]%1[XY] %u",
        sign1, axis1, &size1, sign2, axis2, &size2)) {
      if(axis1[0] == axis2[0])
        throw OHNO("malformed HDR resolution");
      // the resolution line marks the end of the header
      break;
    } else {
      std::cout << "unrecognized HDR header line: \""
        << line << '"' << std::endl;
    }
  }
  if(format == NONE)
    throw OHNO("HDR has no FORMAT line");
  if(format == XYZE)
    std::cout << "HDR treating XYZE as RGBE\n";
  if(size1 == 0 || size2 == 0)
    throw OHNO("HDR has 0 dimension");

  // TODO use EXPOSURE & COLORCORR

  int width, height;
  bool row_order, col_order;
  if(axis1[0] == 'Y') {
    // the first axis is Y: the HDR file is in row-major order
    width = size2;
    height = size1;
    row_order = sign1[0] == '-';
    col_order = sign2[0] == '+';
  } else {
    // the first axis is X: the HDR file is in column-major order
    width = size1;
    height = size2;
    row_order = sign2[0] == '-';
    col_order = sign1[0] == '+';
  }
  Image image(width, height, Pixel::RGBE8_T);

  std::cout << "reading HDR format=" << format
      << " exposure_total=" << exposure_total
      << " r_corr_total=" << r_corr_total << " g_corr_total=" << g_corr_total
      << " b_corr_total=" << b_corr_total
      << " resolution=" << sign1 << axis1 << size1
          << sign2 << axis2 << size2
      << " width=" << width << " height=" << height << '\n';

  Fliperator flip(&image, axis1[0] == 'Y', row_order, col_order);

  // buffer a scanline of pixels
  std::vector<Pixel::RGBE8> scanline(size2);
  bool unnormalized = false;
  for(int i1 = 0; i1 < size1; i1++) {
    scanRLE(input, scanline);
    for(int i2 = 0; i2 < size2; i2++) {
      if(!isNormalized(scanline[i2]))
        unnormalized = true;
      *static_cast<Pixel::RGBE8*>(*flip) = scanline[i2];
      ++flip;
    }
  }
  if(unnormalized)
    std::cout << "warning, HDR unnormalized pixel data\n";

  return image;
}

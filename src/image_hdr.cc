#include "image_hdr.h"

#include "ohno.h"
#include "util.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <istream>

#include <cassert>

namespace {
  const char HDR_SIG[] = "#?RADIANCE";
  const size_t HDR_SIG_BYTES = sizeof(HDR_SIG) - 1;
}

Image readHdr(std::istream &input) {
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
  char formatStr[MAX_LINE];
  enum {
    NONE,
    RGBE,
    XYZE
  } format;
  format = NONE;

  // header EXPOSURE values
  float exposure;
  float exposureTotal = 1;

  // header COLORCORR values
  float rCorr, gCorr, bCorr;
  float rCorrTotal = 1, gCorrTotal = 1, bCorrTotal = 1;

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
      throw OHNO("input fail");

    if(strlen(line) == 0) {
      // ignore blank lines
    } else if(
      hasPrefix("SOFTWARE", line) ||
      hasPrefix("PIXASPECT", line) ||
      hasPrefix("VIEW", line) ||
      hasPrefix("PRIMARIES", line)
    ) {
      // ignore these header lines
    } else if(1 == sscanf(line, "FORMAT= %s", formatStr)) {
      // exactly 1 FORMAT line is required
      if(format != NONE)
        throw OHNO("HDR has multiple FORMAT lines");

      if(!strcmp("32-bit_rle_rgbe", formatStr))
        format = RGBE;
      else if(!strcmp("32-bit_rle_xyze", formatStr))
        format = XYZE;
      else
        throw OHNO("HDR has unrecognized FORMAT");
    } else if(1 == sscanf(line, "EXPOSURE= %f", &exposure)) {
      // any number of EXPOSURE lines are allowed
      exposureTotal *= exposure;
    } else if(3 == sscanf(line, "COLORCORR= %f %f %f",
        &rCorr, &gCorr, &bCorr)) {
      // any number of COLORCORR lines are allowed
      rCorrTotal *= rCorr;
      gCorrTotal *= gCorr;
      bCorrTotal *= bCorr;
    } else if(6 == sscanf(line, "%1[-+]%1[XY] %u %1[-+]%1[XY] %u",
        sign1, axis1, &size1, sign2, axis2, &size2)) {
      if(axis1[0] == axis2[0])
        throw OHNO("malformed HDR resolution");
      // the resolution line marks the end of the header
      break;
    } else {
      std::cout << "unrecognized HDR header line: \"" << line << '"' << std::endl;
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
  bool rowOrder, colOrder;
  if(axis1[0] == 'Y') {
    // the first axis is Y: the HDR file is in row-major order
    width = size2;
    height = size1;
    rowOrder = sign1[0] == '-';
    colOrder = sign2[0] == '+';
  } else {
    // the first axis is X: the HDR file is in column-major order
    width = size1;
    height = size2;
    rowOrder = sign2[0] == '-';
    colOrder = sign1[0] == '+';
  }
  Image image(width, height, Pixel::RGBE8_T);

  std::cout << "reading HDR format=" << format
      << " exposureTotal=" << exposureTotal
      << " rCorrTotal=" << rCorrTotal << " gCorrTotal=" << gCorrTotal
      << " bCorrTotal=" << bCorrTotal
      << " resolution=" << sign1 << axis1 << size1
          << sign2 << axis2 << size2
      << " width=" << width << " height=" << height << '\n';

  Fliperator flip(&image, axis1[0] == 'Y', rowOrder, colOrder);

  // read pixel data

  // buffer a scanline of pixels, to be allocated the first time a new RLE
  // scanline is found
  Pixel::RGBE8* scanline = nullptr;
  // first 4 bytes of the scanline (not necessarily the first pixel value)
  Pixel::RGBE8 first;
  for(int i1 = 0; i1 < size1; i1++) {
    input.read(reinterpret_cast<char*>(&first), sizeof(first));
    if(input.fail())
      throw OHNO("read fail at start of scanline");
    if(first.R == 2 && first.G == 2) {
      // new RLE
      if(size2 != ((first.B << 8) | first.E))
        throw OHNO("HDR RLE wrong length");
      if(!scanline)
        scanline = new Pixel::RGBE8[size2];
      for(int channel = 0; channel < 4; channel++) {
        for(int i2 = 0; i2 < size2;) {
          uint8_t code;
          input.read(reinterpret_cast<char*>(&code), sizeof(code));
          if(input.fail())
            throw OHNO("HDR read code failed");
          // The Radiance filefmts.pdf claims a code with the high bit set
          // indicates a run. The Radiance code in color.c shows this is not
          // quite correct; it uses (code > 128) to check for a run, meaning a
          // byte with the high bit AND at least 1 other bit set indicates a
          // run. A code of exactly 128 is a non-run.
          int length = (code > 0x80) ? (code & 0x7f) : code;
          if(i2 + length > size2)
            throw OHNO("HDR overrun");
          uint8_t value;
          if(code > 0x80) {
            // run
            input.read(reinterpret_cast<char*>(&value), sizeof(value));
            if(input.fail())
              throw OHNO("read fail during run");
            for(int j = 0; j < length; j++)
              scanline[i2++][channel] = value;
          } else {
            // non-run
            for(int j = 0; j < length; j++) {
              input.read(reinterpret_cast<char*>(&value), sizeof(value));
              if(input.fail())
                throw OHNO("read fail during non-run");
              scanline[i2++][channel] = value;
            }
          }
        }
      }
      for(int i2 = 0; i2 < size2; i2++) {
        *static_cast<Pixel::RGBE8*>(*flip) = scanline[i2];
        ++flip;
      }
    } else {
      *static_cast<Pixel::RGBE8*>(*flip) = first;
      ++flip;
      Pixel::RGBE8 next;
      for(int i2 = 1; i2 < size2;) {
        input.read(reinterpret_cast<char*>(&next), sizeof(next));
        if(input.fail())
          throw OHNO("read fail for next pixel");
        if(next.R == 1 && next.G == 1 && next.B == 1) {
          // TODO old RLE
          throw OHNO("HDR old RLE not implemented");
        } else {
          // no RLE
          *static_cast<Pixel::RGBE8*>(*flip) = next;
          ++flip;
          i2++;
        }
      }
    }
  }
  delete[] scanline;

  return image;
}

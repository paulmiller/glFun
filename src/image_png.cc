#include "image_png.h"

#include "ohno.h"

#include "png.h"

#include <cassert>
#include <csetjmp>
#include <functional>
#include <iostream>

namespace {
  const int PNG_SIG_SIZE = 8;

  void readPngStream(png_structp pngStruct, png_bytep data, png_size_t length) {
    if(!pngStruct) {
      return;
    }

    std::istream *input = static_cast<std::istream*>(png_get_io_ptr(pngStruct));

    input->read(reinterpret_cast<char*>(data), length);
    if(!input->good()) {
      std::cout << "readPngStream error" << std::endl;
      png_error(pngStruct, "Read Error");
    }
  }

  /*
  void writePngStream(png_structp pngStruct, png_bytep data, png_size_t length) {
    if(!pngStruct) {
      return;
    }

    std::ostream *output = static_cast<std::ostream*>(png_get_io_ptr(pngStruct));

    output->write(reinterpret_cast<char*>(data), length);
    if(!output->good()) {
      std::cout << "writePngStream error" << std::endl;
      png_error(pngStruct, "Write Error");
    }
  }

  void flushPngStream(png_structp pngStruct) {
    if(!pngStruct) {
      return;
    }

    std::ostream *output = static_cast<std::ostream*>(png_get_io_ptr(pngStruct));

    output->flush();
    if(!output->good()) {
      std::cout << "flushPngStream error" << std::endl;
      png_error(pngStruct, "Write Error");
    }
  }
  */

  class PngReadStruct {
  public:
    PngReadStruct(
      png_voidp error_ptr,
      png_error_ptr error_fn,
      png_error_ptr warn_fn
    ) {
      p = png_create_read_struct(PNG_LIBPNG_VER_STRING,
          error_ptr, error_fn, warn_fn);
      if(!p) {
        throw OHNO("png_create_read_struct failed");
      }
    }

    ~PngReadStruct() {
      png_destroy_read_struct(&p, nullptr, nullptr);
    }

    operator png_structp() {
      return p;
    }
  private:
    PngReadStruct(const PngReadStruct&);
    void operator=(const PngReadStruct&);

    png_structp p;
  };
}

Image loadPng(std::istream &input) {
  char signature[PNG_SIG_SIZE];
  input.read(signature, PNG_SIG_SIZE);
  if(!input.good()) {
    throw OHNO("Couldn't read PNG signature");
  }
  if(png_sig_cmp(reinterpret_cast<png_const_bytep>(signature),
      0, PNG_SIG_SIZE)) {
    throw OHNO("PNG signature doesn't match");
  }

  PngReadStruct pngStruct(nullptr, nullptr, nullptr);

  png_infop pngInfo = png_create_info_struct(pngStruct);
  if(!pngInfo) {
    throw OHNO("png_create_info_struct failed");
  }

  if(setjmp(png_jmpbuf(pngStruct))) {
    throw OHNO("got PNG long jump");
  }

  png_set_read_fn(pngStruct, &input, readPngStream);
  png_set_sig_bytes(pngStruct, PNG_SIG_SIZE);
  // TODO gamma, alpha

  png_read_info(pngStruct, pngInfo);
  png_uint_32 width = png_get_image_width(pngStruct, pngInfo);
  png_uint_32 height = png_get_image_height(pngStruct, pngInfo);
  png_byte bitDepth = png_get_bit_depth(pngStruct, pngInfo);
  png_byte colorType = png_get_color_type(pngStruct, pngInfo);

  if(bitDepth < 8) {
    // Expand to 8 bits per channel
    png_set_packing(pngStruct);
    png_set_expand(pngStruct);
  }

  if(bitDepth == 16) {
    // libpng has the design flaw of forcing us to care about endianness. It
    // should supply data in native endianness, rather than forcing us to
    // explicitly choose between big- or little-. Our hack to check if PNG
    // default endianness matches native endianness is to create an int in PNG
    // endianness and then compare it to a native int. If if doesn't match, then
    // we need to call png_set_swap. Thus we avoid having to know what native
    // endianness is.
    int32_t one;
    png_save_int_32(reinterpret_cast<png_bytep>(&one), 1);
    if(one != 1) {
      png_set_swap(pngStruct);
    }
  }

  if(colorType & PNG_COLOR_MASK_PALETTE) {
    png_set_palette_to_rgb(pngStruct);
  }

  /*
  if(colorType & PNG_COLOR_MASK_COLOR) {
    if(colorType & PNG_COLOR_MASK_ALPHA) {
      type = (bitDepth <= 8) ? Pixel::RGBA8 : Pixel::RGBA16;
    } else {
      type = (bitDepth <= 8) ? Pixel::RGB8 : Pixel::RGB16;
    }
  } else {
    if(colorType & PNG_COLOR_MASK_ALPHA) {
      type = (bitDepth <= 8) ? Pixel::VA8 : Pixel::VA16;
    } else {
      type = (bitDepth <= 8) ? Pixel::V8 : Pixel::V16;
    }
  }
  */

  bool hasAlpha = colorType & PNG_COLOR_MASK_ALPHA;
  if(png_get_valid(pngStruct, pngInfo, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(pngStruct);
    hasAlpha = true;
  }

  Pixel::Type type;
  if(hasAlpha) {
    if(colorType & PNG_COLOR_MASK_COLOR) {
      type = (bitDepth <= 8) ? Pixel::RGBA8 : Pixel::RGBA16;
    } else {
      type = (bitDepth <= 8) ? Pixel::VA8 : Pixel::VA16;
    }
  } else {
    if(colorType & PNG_COLOR_MASK_COLOR) {
      type = (bitDepth <= 8) ? Pixel::RGB8 : Pixel::RGB16;
    } else {
      type = (bitDepth <= 8) ? Pixel::V8 : Pixel::V16;
    }
  }

  std::cout << "loaded PNG width=" << width << " height=" << height
      << " bitDepth=" << (int) bitDepth << " hasAlpha=" << hasAlpha
      << " type=" << Pixel::name(type) << std::endl;

  Image img(width, height, type);

  png_bytepp rows = new png_bytep[height];
  for(int i = 0; i < (int)height; i++) {
    rows[i] = reinterpret_cast<png_bytep>(img.getRowPtr(i));
  }
  png_read_image(pngStruct, rows);
  delete[] rows;

  return img;
}

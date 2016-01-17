#include "image_png.h"

#include "ohno.h"
#include "png.h"
#include "util.h"

#include <cassert>
#include <csetjmp>
#include <functional>
#include <iostream>

namespace {
  const int PNG_SIG_SIZE = 8;

  void readPngStream(png_structp s, png_bytep data, png_size_t length) {
    if(!s)
      return;
    std::istream *input = static_cast<std::istream*>(png_get_io_ptr(s));
    input->read(reinterpret_cast<char*>(data), length);
    if(!input->good()) {
      std::cout << "readPngStream error" << std::endl;
      png_error(s, "Read Error");
    }
  }

  void writePngStream(png_structp s, png_bytep data, png_size_t length) {
    if(!s)
      return;
    std::ostream *output = static_cast<std::ostream*>(png_get_io_ptr(s));
    output->write(reinterpret_cast<char*>(data), length);
    if(!output->good()) {
      std::cout << "writePngStream error" << std::endl;
      png_error(s, "Write Error");
    }
  }

  void flushPngStream(png_structp s) {
    if(!s)
      return;
    std::ostream *output = static_cast<std::ostream*>(png_get_io_ptr(s));
    output->flush();
    if(!output->good()) {
      std::cout << "flushPngStream error" << std::endl;
      png_error(s, "Write Error");
    }
  }

  // libpng has the design flaw of forcing us to care about endianness. It
  // should supply data in native endianness, rather than forcing us to
  // explicitly choose between big- or little-. Our hack to check if PNG default
  // endianness matches native endianness is to create an int in PNG endianness
  // and then compare it to a native int. If if doesn't match, then we need to
  // call png_set_swap. Thus we avoid having to know what native endianness is.
  bool shouldSwapEndian() {
    int32_t one;
    png_save_int_32(reinterpret_cast<png_bytep>(&one), 1);
    return one == 1;
  }

  class PngReader {
  public:
    PngReader(
      png_voidp error_ptr,
      png_error_ptr error_fn,
      png_error_ptr warn_fn
    ) {
      s = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                 error_ptr, error_fn, warn_fn);
      if(!s)
        throw OHNO("png_create_read_struct failed");
      i = png_create_info_struct(s);
      if(!i) {
        png_destroy_read_struct(&s, nullptr, nullptr);
        throw OHNO("png_create_info_struct failed");
      }
    }

    ~PngReader() {
      png_destroy_read_struct(&s, &i, nullptr);
    }

    png_structp s;
    png_infop i;
  };

  class PngWriter {
  public:
    PngWriter(
      png_voidp error_ptr,
      png_error_ptr error_fn,
      png_error_ptr warn_fn
    ) {
      s = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                  error_ptr, error_fn, warn_fn);
      if(!s)
        throw OHNO("png_create_write_struct failed");
      i = png_create_info_struct(s);
      if(!i) {
        png_destroy_write_struct(&s, nullptr);
        throw OHNO("png_create_info_struct failed");
      }
    }

    ~PngWriter() {
      png_destroy_write_struct(&s, &i);
    }

    png_structp s;
    png_infop i;
  };
}

Image readPng(std::istream &input) {
  char signature[PNG_SIG_SIZE];
  input.read(signature, PNG_SIG_SIZE);
  if(!input.good())
    throw OHNO("couldn't read PNG signature");
  if(png_sig_cmp(reinterpret_cast<png_const_bytep>(signature),
      0, PNG_SIG_SIZE))
    throw OHNO("PNG signature doesn't match");

  PngReader reader(nullptr, nullptr, nullptr);

  if(setjmp(png_jmpbuf(reader.s)))
    throw OHNO("got PNG long jump");

  png_set_read_fn(reader.s, &input, readPngStream);
  png_set_sig_bytes(reader.s, PNG_SIG_SIZE);
  // TODO gamma, alpha

  png_read_info(reader.s, reader.i);
  png_uint_32 width = png_get_image_width(reader.s, reader.i);
  png_uint_32 height = png_get_image_height(reader.s, reader.i);
  png_byte bitDepth = png_get_bit_depth(reader.s, reader.i);
  png_byte colorType = png_get_color_type(reader.s, reader.i);

  if(bitDepth < 8) {
    // Expand to 8 bits per channel
    png_set_packing(reader.s);
    png_set_expand(reader.s);
  }

  if(bitDepth == 16 && shouldSwapEndian()) {
    png_set_swap(reader.s);
  }

  if(colorType & PNG_COLOR_MASK_PALETTE) {
    png_set_palette_to_rgb(reader.s);
  }

  bool hasAlpha = colorType & PNG_COLOR_MASK_ALPHA;
  if(png_get_valid(reader.s, reader.i, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(reader.s);
    hasAlpha = true;
  }

  Pixel::Type type;
  if(hasAlpha) {
    if(colorType & PNG_COLOR_MASK_COLOR) {
      type = (bitDepth <= 8) ? Pixel::RGBA8_T : Pixel::RGBA16_T;
    } else {
      type = (bitDepth <= 8) ? Pixel::VA8_T : Pixel::VA16_T;
    }
  } else {
    if(colorType & PNG_COLOR_MASK_COLOR) {
      type = (bitDepth <= 8) ? Pixel::RGB8_T : Pixel::RGB16_T;
    } else {
      type = (bitDepth <= 8) ? Pixel::V8_T : Pixel::V16_T;
    }
  }

  std::cout << "reading PNG width=" << width << " height=" << height
      << " bitDepth=" << (int) bitDepth
      << " type=" << Pixel::name(type) << std::endl;

  Image img(width, height, type);

  png_bytepp rows = new png_bytep[height];
  for(int i = 0; i < (int)height; i++) {
    rows[i] = reinterpret_cast<png_bytep>(img.getRowPtr(i));
  }
  png_read_image(reader.s, rows);
  delete[] rows;

  png_read_end(reader.s, nullptr);

  return img;
}

void writePng(std::ostream &output, Image &img) {
  PngWriter writer(nullptr, nullptr, nullptr);

  if(setjmp(png_jmpbuf(writer.s)))
    throw OHNO("got PNG long jump");

  png_set_write_fn(writer.s, &output, writePngStream, flushPngStream);

  png_byte bitDepth;
  switch(img.type()) {
    case Pixel::V8_T:
    case Pixel::VA8_T:
    case Pixel::RGB8_T:
    case Pixel::RGBA8_T:
      bitDepth = 8;
      break;
    case Pixel::V16_T:
    case Pixel::VA16_T:
    case Pixel::RGB16_T:
    case Pixel::RGBA16_T:
      bitDepth = 16;
      break;
    case Pixel::RGBf_T:
    case Pixel::RGBE8_T:
      throw OHNO("PNG unsupported pixel type");

    default:
      assert(0);
      return;
  }

  png_byte colorType;
  switch(img.type()) {
    case Pixel::V8_T:
    case Pixel::V16_T:
      colorType = PNG_COLOR_TYPE_GRAY;
      break;
    case Pixel::VA8_T:
    case Pixel::VA16_T:
      colorType = PNG_COLOR_TYPE_GRAY_ALPHA;
      break;
    case Pixel::RGB8_T:
    case Pixel::RGB16_T:
      colorType = PNG_COLOR_TYPE_RGB;
      break;
    case Pixel::RGBA8_T:
    case Pixel::RGBA16_T:
      colorType = PNG_COLOR_TYPE_RGB_ALPHA;
      break;

    default:
      assert(0);
      return;
  }

  int width = img.width();
  int height = img.height();
  Pixel::Type type = img.type();

  std::cout << "writing PNG width=" << width << " height=" << height
      << " bitDepth=" << (int) bitDepth
      << " type=" << Pixel::name(type) << std::endl;

  png_set_IHDR(
      writer.s, writer.i,
      width, height,
      bitDepth, colorType,
      PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  int transform = PNG_TRANSFORM_IDENTITY;
  if(bitDepth == 16 && shouldSwapEndian())
    transform = PNG_TRANSFORM_SWAP_ENDIAN;

  png_bytepp rows = new png_bytep[height];
  for(int i = 0; i < (int)height; i++) {
    rows[i] = reinterpret_cast<png_bytep>(img.getRowPtr(i));
  }
  png_set_rows(writer.s, writer.i, rows);
  png_write_png(writer.s, writer.i, transform, nullptr);
  delete[] rows;
}

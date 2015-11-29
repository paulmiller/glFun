#include "image.h"

#include <cassert>
#include <cstring> // memset
#include <cstdio> // snprintf

std::size_t Pixel::size(Type t) {
  // Return our own, un-padded sizes to avoid alignment uncertainty.
  switch(t) {
    case NONE:   return 0;
    case V8:     return 1;
    case V16:    return 2;
    case VA8:    return 2;
    case VA16:   return 4;
    case RGB8:   return 3;
    case RGB16:  return 6;
    case RGBA8:  return 4;
    case RGBA16: return 8;

    default:
      assert(0);
      return 0;
  }
}

const char * Pixel::name(Type t) {
  switch(t) {
    case NONE:   return "NONE";
    case V8:     return "V8";
    case V16:    return "V16";
    case VA8:    return "VA8";
    case VA16:   return "VA16";
    case RGB8:   return "RGB8";
    case RGB16:  return "RGB16";
    case RGBA8:  return "RGBA8";
    case RGBA16: return "RGBA16";

    default:
      assert(0);
      return 0;
  }
}

std::ostream& operator<<(std::ostream & o, const PixelV8 & p) {
  char buf[sizeof("00")];
  snprintf(buf, sizeof(buf), "%02x", p.V);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const PixelV16 & p) {
  char buf[sizeof("0000")];
  snprintf(buf, sizeof(buf), "%04x", p.V);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const PixelVA8 & p) {
  char buf[sizeof("00-00")];
  snprintf(buf, sizeof(buf), "%02x-%02x", p.V, p.A);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const PixelVA16 & p) {
  char buf[sizeof("0000-0000")];
  snprintf(buf, sizeof(buf), "%04x-%04x", p.V, p.A);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const PixelRGB8 & p) {
  char buf[sizeof("00-00-00")];
  snprintf(buf, sizeof(buf), "%02x-%02x-%02x", p.R, p.G, p.B);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const PixelRGB16 & p) {
  char buf[sizeof("0000-0000-0000")];
  snprintf(buf, sizeof(buf), "%04x-%04x-%04x", p.R, p.G, p.B);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const PixelRGBA8 & p) {
  char buf[sizeof("00-00-00-00")];
  snprintf(buf, sizeof(buf), "%02x-%02x-%02x-%02x", p.R, p.G, p.B, p.A);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const PixelRGBA16 & p) {
  char buf[sizeof("0000-0000-0000-0000")];
  snprintf(buf, sizeof(buf), "%04x-%04x-%04x-%04x", p.R, p.G, p.B, p.A);
  o << buf;
  return o;
}

Image::Image() :
  mData(nullptr), mType(Pixel::NONE), mWidth(0), mHeight(0) {}

Image::Image(int width, int height, Pixel::Type type) : mData(nullptr) {
  init(width, height, type);
}

Image::Image(Image&& src) :
  mData(src.mData), mType(src.mType), mWidth(src.mWidth), mHeight(src.mHeight)
{
  src.mData = nullptr;
}

Image::~Image() {
  delete[] mData;
}

void Image::init(int width, int height, Pixel::Type type) {
  assert(mData == nullptr);
  mWidth = width;
  mHeight = height;
  mType = type;
  std::size_t size = width * height * Pixel::size(type);
  mData = new char[size];
  memset(mData, 0, size);
}

void Image::clear() {
  delete[] mData;
  mData = nullptr;
  mType = Pixel::NONE;
  mWidth = 0;
  mHeight = 0;
}

int Image::width() const {
  return mWidth;
}

int Image::height() const {
  return mHeight;
}

Pixel::Type Image::type() const {
  return mType;
}

void *Image::data() {
  return mData;
}

void *Image::getRowPtr(int row) {
  return mData + (mWidth * row) * Pixel::size(mType);
}

void *Image::getPixelPtr(int row, int col) {
  return mData + (mWidth * row + col) * Pixel::size(mType);
}

const void *Image::getPixelPtr(int row, int col) const {
  return mData + (mWidth * row + col) * Pixel::size(mType);
}

std::ostream& operator<<(std::ostream& o, const Image& image) {
  Pixel::Type type = image.type();
  if(type == Pixel::NONE) {
    o << "none" << std::endl;
  } else {
    int height = image.height();
    int width = image.width();
    for(int r = 0; r < height; r++) {
      for(int c = 0; c < width; c++) {
        if(c) {
          o << ' ';
        }
        const void *p = image.getPixelPtr(r, c);
        switch(type) {
          case Pixel::V8:     o << *static_cast<const PixelV8 *>(p); break;
          case Pixel::V16:    o << *static_cast<const PixelV16 *>(p); break;
          case Pixel::VA8:    o << *static_cast<const PixelVA8 *>(p); break;
          case Pixel::VA16:   o << *static_cast<const PixelVA16 *>(p); break;
          case Pixel::RGB8:   o << *static_cast<const PixelRGB8 *>(p); break;
          case Pixel::RGB16:  o << *static_cast<const PixelRGB16 *>(p); break;
          case Pixel::RGBA8:  o << *static_cast<const PixelRGBA8 *>(p); break;
          case Pixel::RGBA16: o << *static_cast<const PixelRGBA16 *>(p); break;
          default:
            assert(0);
            return o;
        }
      }
      o << std::endl;
    }
  }
  return o;
}

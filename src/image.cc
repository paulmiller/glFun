#include "image.h"

#include <cassert>
#include <cstring> // memset
#include <cstdio> // snprintf

std::size_t Pixel::size(Type t) {
  switch(t) {
    case NONE_T:   return 0;
    case V8_T:     return sizeof(Pixel::V8);
    case V16_T:    return sizeof(Pixel::V16);
    case VA8_T:    return sizeof(Pixel::VA8);
    case VA16_T:   return sizeof(Pixel::VA16);
    case RGB8_T:   return sizeof(Pixel::RGB8);
    case RGB16_T:  return sizeof(Pixel::RGB16);
    case RGBA8_T:  return sizeof(Pixel::RGBA8);
    case RGBA16_T: return sizeof(Pixel::RGBA16);
    case RGBE8_T:  return sizeof(Pixel::RGBE8);
    case RGBf_T:   return sizeof(Pixel::RGBf);

    default:
      assert(0);
      return 0;
  }
}

const char * Pixel::name(Type t) {
  switch(t) {
    case NONE_T:   return "NONE";
    case V8_T:     return "V8";
    case V16_T:    return "V16";
    case VA8_T:    return "VA8";
    case VA16_T:   return "VA16";
    case RGB8_T:   return "RGB8";
    case RGB16_T:  return "RGB16";
    case RGBA8_T:  return "RGBA8";
    case RGBA16_T: return "RGBA16";
    case RGBE8_T:  return "RGBE8";
    case RGBf_T:   return "RGBf";

    default:
      assert(0);
      return 0;
  }
}

uint8_t& Pixel::VA8::operator[](int i) {
  assert(0 <= i && i < 2);
  return (&V)[i];
}

uint16_t& Pixel::VA16::operator[](int i) {
  assert(0 <= i && i < 2);
  return (&V)[i];
}

uint8_t& Pixel::RGB8::operator[](int i) {
  assert(0 <= i && i < 3);
  return (&R)[i];
}

uint16_t& Pixel::RGB16::operator[](int i) {
  assert(0 <= i && i < 3);
  return (&R)[i];
}

uint8_t& Pixel::RGBA8::operator[](int i) {
  assert(0 <= i && i < 4);
  return (&R)[i];
}

uint16_t& Pixel::RGBA16::operator[](int i) {
  assert(0 <= i && i < 4);
  return (&R)[i];
}

uint8_t& Pixel::RGBE8::operator[](int i) {
  assert(0 <= i && i < 4);
  return (&R)[i];
}

float& Pixel::RGBf::operator[](int i) {
  assert(0 <= i && i < 4);
  return (&R)[i];
}

std::ostream& operator<<(std::ostream & o, const Pixel::V8 & p) {
  char buf[sizeof("00")];
  snprintf(buf, sizeof(buf), "%02x", p.V);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const Pixel::V16 & p) {
  char buf[sizeof("0000")];
  snprintf(buf, sizeof(buf), "%04x", p.V);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const Pixel::VA8 & p) {
  char buf[sizeof("00-00")];
  snprintf(buf, sizeof(buf), "%02x-%02x", p.V, p.A);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const Pixel::VA16 & p) {
  char buf[sizeof("0000-0000")];
  snprintf(buf, sizeof(buf), "%04x-%04x", p.V, p.A);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const Pixel::RGB8 & p) {
  char buf[sizeof("00-00-00")];
  snprintf(buf, sizeof(buf), "%02x-%02x-%02x", p.R, p.G, p.B);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const Pixel::RGB16 & p) {
  char buf[sizeof("0000-0000-0000")];
  snprintf(buf, sizeof(buf), "%04x-%04x-%04x", p.R, p.G, p.B);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const Pixel::RGBA8 & p) {
  char buf[sizeof("00-00-00-00")];
  snprintf(buf, sizeof(buf), "%02x-%02x-%02x-%02x", p.R, p.G, p.B, p.A);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const Pixel::RGBA16 & p) {
  char buf[sizeof("0000-0000-0000-0000")];
  snprintf(buf, sizeof(buf), "%04x-%04x-%04x-%04x", p.R, p.G, p.B, p.A);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const Pixel::RGBE8 & p) {
  char buf[sizeof("00-00-00-00")];
  snprintf(buf, sizeof(buf), "%02x-%02x-%02x-%02x", p.R, p.G, p.B, p.E);
  o << buf;
  return o;
}

std::ostream& operator<<(std::ostream & o, const Pixel::RGBf & p) {
  char buf[sizeof("-0.000e+00/-0.000e+00/-0.000e+00")];
  snprintf(buf, sizeof(buf), "%.3e/%.3e/%.3e", p.R, p.G, p.B);
  o << buf;
  return o;
}

Image::Image() :
  mData(nullptr), mType(Pixel::NONE_T), mWidth(0), mHeight(0) {}

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
  mType = Pixel::NONE_T;
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

Fliperator::Fliperator(
  Image* image, bool rowMajor, bool rowOrder, bool colOrder
) :
  mImage(image), mRowMajor(rowMajor), mRowOrder(rowOrder), mColOrder(colOrder),
  mDone(false)
{
  if(rowOrder)
    mRow = 0;
  else
    mRow = mImage->height() - 1;

  if(colOrder)
    mCol = 0;
  else
    mCol = mImage->width() - 1;
}

void* Fliperator::operator*() {
  return mImage->getPixelPtr(mRow, mCol);
}

// return true if we were on the last row
bool Fliperator::advanceRow() {
  if(mRowOrder) {
    if(mRow + 1 < mImage->height()) {
      mRow++;
      return false;
    } else {
      mRow = 0;
      return true;
    }
  } else {
    if(mRow > 0) {
      mRow--;
      return false;
    } else {
      mRow = mImage->height() - 1;
      return true;
    }
  }
}

// return true if we were on the last column
bool Fliperator::advanceCol() {
  if(mColOrder) {
    if(mCol + 1 < mImage->width()) {
      mCol++;
      return false;
    } else {
      mCol = 0;
      return true;
    }
  } else {
    if(mCol > 0) {
      mCol--;
      return false;
    } else {
      mCol = mImage->width() - 1;
      return true;
    }
  }
}

Fliperator& Fliperator::operator++() {
  if(mRowMajor) {
    if(advanceCol()) {
      if(advanceRow())
        mDone = true;
    }
  } else {
    if(advanceRow()) {
      if(advanceCol())
        mDone = true;
    }
  }
  return *this;
}

bool Fliperator::isDone() {
  return mDone;
}

std::ostream& operator<<(std::ostream& o, const Image& image) {
  Pixel::Type type = image.type();
  if(type == Pixel::NONE_T) {
    o << "none\n";
    return o;
  }

  int height = image.height();
  int width = image.width();
  for(int r = 0; r < height; r++) {
    for(int c = 0; c < width; c++) {
      if(c)
        o << ' ';

      const void *p = image.getPixelPtr(r, c);
      switch(type) {
        case Pixel::V8_T:     o << *static_cast<const Pixel::V8 *>(p); break;
        case Pixel::V16_T:    o << *static_cast<const Pixel::V16 *>(p); break;
        case Pixel::VA8_T:    o << *static_cast<const Pixel::VA8 *>(p); break;
        case Pixel::VA16_T:   o << *static_cast<const Pixel::VA16 *>(p); break;
        case Pixel::RGB8_T:   o << *static_cast<const Pixel::RGB8 *>(p); break;
        case Pixel::RGB16_T:  o << *static_cast<const Pixel::RGB16 *>(p); break;
        case Pixel::RGBA8_T:  o << *static_cast<const Pixel::RGBA8 *>(p); break;
        case Pixel::RGBA16_T: o << *static_cast<const Pixel::RGBA16 *>(p); break;

        default:
          assert(0);
          return o;
      }
    }
    o << '\n';
  }
  return o;
}

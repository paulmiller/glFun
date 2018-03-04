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
    case RGBf_T:   return sizeof(Pixel::RGBf);
    case RGBA8_T:  return sizeof(Pixel::RGBA8);
    case RGBA16_T: return sizeof(Pixel::RGBA16);
    case RGBE8_T:  return sizeof(Pixel::RGBE8);

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
    case RGBf_T:   return "RGBf";
    case RGBA8_T:  return "RGBA8";
    case RGBA16_T: return "RGBA16";
    case RGBE8_T:  return "RGBE8";

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

float& Pixel::RGBf::operator[](int i) {
  assert(0 <= i && i < 4);
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

std::ostream& operator<<(std::ostream & o, const Pixel::RGBf & p) {
  char buf[sizeof("-0.000e+00/-0.000e+00/-0.000e+00")];
  snprintf(buf, sizeof(buf), "%.3e/%.3e/%.3e", p.R, p.G, p.B);
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

Image::Image() :
  data_(nullptr), type_(Pixel::NONE_T), width_(0), height_(0) {}

Image::Image(int width, int height, Pixel::Type type) : data_(nullptr) {
  init(width, height, type);
}

Image::Image(Image&& src) :
  data_(src.data_), type_(src.type_), width_(src.width_), height_(src.height_)
{
  src.data_ = nullptr;
}

Image::~Image() {
  delete[] data_;
}

Image& Image::operator=(Image&& src) {
  delete[] data_;

  data_ = src.data_;
  type_ = src.type_;
  width_ = src.width_;
  height_ = src.height_;

  src.data_ = nullptr;
  src.type_ = Pixel::NONE_T;
  src.width_ = 0;
  src.height_ = 0;

  return *this;
}

void Image::init(int width, int height, Pixel::Type type) {
  assert(data_ == nullptr);
  width_ = width;
  height_ = height;
  type_ = type;
  std::size_t size = width * height * Pixel::size(type);
  data_ = new char[size];
  memset(data_, 0, size);
}

void Image::clear() {
  delete[] data_;
  data_ = nullptr;
  type_ = Pixel::NONE_T;
  width_ = 0;
  height_ = 0;
}

int Image::width() const {
  return width_;
}

int Image::height() const {
  return height_;
}

Pixel::Type Image::type() const {
  return type_;
}

void *Image::data() {
  return data_;
}

void *Image::getRowPtr(int row) {
  return data_ + (width_ * row) * Pixel::size(type_);
}

void *Image::getPixelPtr(int row, int col) {
  return data_ + (width_ * row + col) * Pixel::size(type_);
}

const void *Image::getPixelPtr(int row, int col) const {
  return data_ + (width_ * row + col) * Pixel::size(type_);
}

Fliperator::Fliperator(
  Image* image, bool row_major, bool row_order, bool col_order
) :
  image_(image),
  row_major_(row_major), row_order_(row_order), col_order_(col_order),
  done_(false)
{
  if(row_order)
    row_ = 0;
  else
    row_ = image->height() - 1;

  if(col_order)
    col_ = 0;
  else
    col_ = image->width() - 1;
}

void* Fliperator::operator*() {
  return image_->getPixelPtr(row_, col_);
}

// return true if we were on the last row
bool Fliperator::advanceRow() {
  if(row_order_) {
    if(row_ + 1 < image_->height()) {
      row_++;
      return false;
    } else {
      row_ = 0;
      return true;
    }
  } else {
    if(row_ > 0) {
      row_--;
      return false;
    } else {
      row_ = image_->height() - 1;
      return true;
    }
  }
}

// return true if we were on the last column
bool Fliperator::advanceCol() {
  if(col_order_) {
    if(col_ + 1 < image_->width()) {
      col_++;
      return false;
    } else {
      col_ = 0;
      return true;
    }
  } else {
    if(col_ > 0) {
      col_--;
      return false;
    } else {
      col_ = image_->width() - 1;
      return true;
    }
  }
}

Fliperator& Fliperator::operator++() {
  if(row_major_) {
    if(advanceCol()) {
      if(advanceRow())
        done_ = true;
    }
  } else {
    if(advanceRow()) {
      if(advanceCol())
        done_ = true;
    }
  }
  return *this;
}

bool Fliperator::isDone() {
  return done_;
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
        using namespace Pixel;
        case V8_T:     o << *static_cast<const V8     *>(p); break;
        case V16_T:    o << *static_cast<const V16    *>(p); break;
        case VA8_T:    o << *static_cast<const VA8    *>(p); break;
        case VA16_T:   o << *static_cast<const VA16   *>(p); break;
        case RGB8_T:   o << *static_cast<const RGB8   *>(p); break;
        case RGB16_T:  o << *static_cast<const RGB16  *>(p); break;
        case RGBf_T:   o << *static_cast<const RGBf   *>(p); break;
        case RGBA8_T:  o << *static_cast<const RGBA8  *>(p); break;
        case RGBA16_T: o << *static_cast<const RGBA16 *>(p); break;
        case RGBE8_T:  o << *static_cast<const RGBE8  *>(p); break;

        default:
          assert(0);
          return o;
      }
    }
    o << '\n';
  }
  return o;
}

#ifndef IMAGE_H
#define IMAGE_H

#include <cstdint>
#include <ostream>

namespace Pixel {
  /*
  Pixel naming scheme:
    V  = value (greyscale)
    A  = alpha
    R  = red
    G  = green
    B  = blue
    E  = exponent (shared by colors if present)
    8  = uint8_t
    16 = uint16_t
    f  = float
  */
  enum Type {
    NONE_T = 0,
    V8_T,
    V16_T,
    VA8_T,
    VA16_T,
    RGB8_T,
    RGB16_T,
    RGBA8_T,
    RGBA16_T,
    RGBE8_T,
    RGBf_T,
  };

  std::size_t size(Type t);
  const char * name(Type t);

  class V8     { public: uint8_t  V;                                        };
  class V16    { public: uint16_t V;                                        };
  class VA8    { public: uint8_t  V, A;       uint8_t  & operator[](int i); };
  class VA16   { public: uint16_t V, A;       uint16_t & operator[](int i); };
  class RGB8   { public: uint8_t  R, G, B;    uint8_t  & operator[](int i); };
  class RGB16  { public: uint16_t R, G, B;    uint16_t & operator[](int i); };
  class RGBA8  { public: uint8_t  R, G, B, A; uint8_t  & operator[](int i); };
  class RGBA16 { public: uint16_t R, G, B, A; uint16_t & operator[](int i); };
  class RGBE8  { public: uint8_t  R, G, B, E; uint8_t  & operator[](int i); };
  class RGBf   { public: float    R, G, B;    float    & operator[](int i); };
};

std::ostream& operator<<(std::ostream & o, const Pixel::V8     & p);
std::ostream& operator<<(std::ostream & o, const Pixel::V16    & p);
std::ostream& operator<<(std::ostream & o, const Pixel::VA8    & p);
std::ostream& operator<<(std::ostream & o, const Pixel::VA16   & p);
std::ostream& operator<<(std::ostream & o, const Pixel::RGB8   & p);
std::ostream& operator<<(std::ostream & o, const Pixel::RGB16  & p);
std::ostream& operator<<(std::ostream & o, const Pixel::RGBA8  & p);
std::ostream& operator<<(std::ostream & o, const Pixel::RGBA16 & p);
std::ostream& operator<<(std::ostream & o, const Pixel::RGBE8  & p);
std::ostream& operator<<(std::ostream & o, const Pixel::RGBf   & p);

// Images are stored in row-major order. Lower rows are "up", higher rows are
// "down", lower columns are "left", and higher columns are "right".
class Image {
private:
  char *mData;
  Pixel::Type mType;
  int mWidth;
  int mHeight;

public:
  Image(); // Create uninitialized image
  Image(int width, int height, Pixel::Type type); // Create initialized image
  Image(Image&& src);
  ~Image();
  void init(int width, int height, Pixel::Type type);
  void clear();
  int width() const;
  int height() const;
  Pixel::Type type() const;
  void *data();
  void *getRowPtr(int row);
  void *getPixelPtr(int row, int col);
  const void *getPixelPtr(int row, int col) const;

};

// iterate over pixels in an Image in a configurable order
class Fliperator {
private:
  Image* mImage;
  int mRow, mCol;
  bool mRowMajor, mRowOrder, mColOrder, mDone;

  bool advanceRow();
  bool advanceCol();

public:
  // rowMajor - if true, iterate in row-major order
  // rowOrder - if true, iterate from top to bottom
  // colOrder - if true, iterate from left to right
  // e.g.
  // (true, true, true) - left to right, then top to bottom (normal)
  // (true, true, false) - right to left, then top to bottom (horizontal flip)
  // (false, true, true) - top to bottom, then left to right (diagonal flip)
  // (false, false, false) - bottom to top, then right to left (diagonal flip)
  // (false, false, true) - bottom to top, then left to right (rotation)
  Fliperator(Image* image, bool rowMajor, bool rowOrder, bool colOrder);

  void* operator*();
  Fliperator& operator++();
  bool isDone();
};

std::ostream& operator<<(std::ostream& out, const Image& image);

#endif

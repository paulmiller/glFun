#ifndef IMAGE_H
#define IMAGE_H

#include <cstdint>
#include <ostream>

class Pixel {
public:
  // RTTI in every pixel would balloon image size, so we track pixel types
  // manually.
  enum Type {
    NONE = 0,
    V8,
    V16,
    VA8,
    VA16,
    RGB8,
    RGB16,
    RGBA8,
    RGBA16,
  };

  static std::size_t size(Type t);
  static const char * name(Type t);
};

class PixelV8 : public Pixel {
public:
  uint8_t V;
};

class PixelV16 : public Pixel {
public:
  uint16_t V;
};

class PixelVA8 : public Pixel {
public:
  uint8_t V, A;
};

class PixelVA16 : public Pixel {
public:
  uint16_t V, A;
};

class PixelRGB8 : public Pixel {
public:
  uint8_t R, G, B;
};

class PixelRGB16 : public Pixel {
public:
  uint16_t R, G, B;
};

class PixelRGBA8 : public Pixel {
public:
  uint8_t R, G, B, A;
};

class PixelRGBA16 : public Pixel {
public:
  uint16_t R, G, B, A;
};

std::ostream& operator<<(std::ostream & o, const PixelV8 & p);
std::ostream& operator<<(std::ostream & o, const PixelV16 & p);
std::ostream& operator<<(std::ostream & o, const PixelVA8 & p);
std::ostream& operator<<(std::ostream & o, const PixelVA16 & p);
std::ostream& operator<<(std::ostream & o, const PixelRGB8 & p);
std::ostream& operator<<(std::ostream & o, const PixelRGB16 & p);
std::ostream& operator<<(std::ostream & o, const PixelRGBA8 & p);
std::ostream& operator<<(std::ostream & o, const PixelRGBA16 & p);

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

std::ostream& operator<<(std::ostream& out, const Image& image);

#endif

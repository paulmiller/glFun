#ifndef IMAGE_PNG_H
#define IMAGE_PNG_H

#include "image.h"

#include <istream>

Image readPng(std::istream &input);
void writePng(std::ostream &output, Image &img);

#endif

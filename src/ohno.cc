#include "ohno.h"

OhNo::OhNo(const char *file, int line, const char *msg) :
  file_(file), line_(line), msg_(msg) {}

std::ostream& operator<<(std::ostream &o, const OhNo &ohno) {
  o << ohno.file_ << ':' << ohno.line_ << ' ' << ohno.msg_;
  return o;
}

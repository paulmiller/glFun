#ifndef OHNO_H
#define OHNO_H

#include <ostream>
#include <stdexcept>

#define OHNO(msg) OhNo(__FILE__, __LINE__, msg)

class OhNo : public std::exception {
private:
  const char *file_;
  const int line_;
  const char *msg_;

public:
  OhNo(const char *file, int line, const char *msg);

  friend std::ostream& operator<<(std::ostream&, const OhNo&);
};

#endif

#ifndef GLFUN_APPROX_CONTAINER_H
#define GLFUN_APPROX_CONTAINER_H

#include "catch.h"

#include <limits>

namespace glfun {

class ApproxContainer {
public:
  void SetEpsilon(double epsilon) {
    epsilon_ = epsilon;
  }

  void SetMargin(double margin) {
    margin_ = margin;
  }

  void SetScale(double scale) {
    scale_ = scale;
  }

protected:
  Approx ApproxValue(double d) const {
    Approx a = Approx(d);
    if(!std::isnan(epsilon_))
      a = a.epsilon(epsilon_);
    if(!std::isnan(margin_))
      a = a.margin(margin_);
    if(!std::isnan(scale_))
      a = a.scale(scale_);
    return a;
  }

private:
  static constexpr double NaN = std::numeric_limits<double>::quiet_NaN();

  // if not NaN, override the corresponding parameter in Catch's Approx class
  double epsilon_ = NaN;
  double margin_  = NaN;
  double scale_   = NaN;
};

} // namespace glfun

#endif

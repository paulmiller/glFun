#ifndef RAY_H
#define RAY_H

#include "camera.h"
#include "math/vector.h"

#include <vector>

class Ray {
public:
  Vector3f start;
  Vector3f direction; // unit vector
};

std::vector<Ray> MakeCameraRays(const Camera &camera);

#endif

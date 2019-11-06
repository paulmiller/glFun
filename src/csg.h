#ifndef CSG_H
#define CSG_H

#include "ray.h"

#include <vector>

class CsgNode {
public:
  virtual ~CsgNode() {}
};

class CsgUnionNode : public CsgNode {
};

class CsgPrimitive : public CsgNode {
public:
  class Hit {
  public:
    float distance; // from the start of the ray to the point of intersection
    bool entering; // weather the ray is entering or exiting the primitive
  };

  // find all intersections between "ray" and the surface of this primitive, and
  // append them in order onto "hits"
  virtual void IntersectRay(const Ray &ray, std::vector<Hit> *hits) = 0;
};

class CsgSphere : public CsgPrimitive {
public:
  float radius;

  CsgSphere(float radius) : radius(radius) {}

  void IntersectRay(const Ray &ray, std::vector<Hit> *hits) override;
};

class CsgCube : public CsgPrimitive {
public:
  void IntersectRay(const Ray &ray, std::vector<Hit> *hits) override;
};

std::vector<Ray> TestIntersectRay();

#endif

#ifndef CSG_H
#define CSG_H

#include "ray.h"

#include <memory>
#include <vector>

class CsgPrimitive;

class CsgNode {
public:
  class Hit {
  public:
    CsgPrimitive *primitive; // the primitive that was hit
    float distance; // from the start of the ray to the point of intersection
    bool entering; // weather the ray is entering or exiting the primitive

    bool operator<(Hit rhs) const { return distance < rhs.distance; }
    bool operator>(Hit rhs) const { return distance > rhs.distance; }
  };

  virtual ~CsgNode() {}

  // find all intersections between "ray" and the surface of this primitive, and
  // append them in order onto "hits"
  virtual void IntersectRay(const Ray &ray, std::vector<Hit> *hits) = 0;
};

class CsgUnion : public CsgNode {
public:
  std::unique_ptr<CsgNode> a, b;

  CsgUnion(std::unique_ptr<CsgNode> a, std::unique_ptr<CsgNode> b) :
    a(std::move(a)), b(std::move(b)) {}

  void IntersectRay(const Ray &ray, std::vector<Hit> *hits) override;
};

class CsgPrimitive : public CsgNode {
public:
  // return the unit normal vector at "pos", which must be somewhere on the
  // surface of this primitive
  virtual Vector3f GetNormal(Vector3f pos) = 0;
};

class CsgSphere : public CsgPrimitive {
public:
  float radius;

  CsgSphere(float radius) : radius(radius) {}

  void IntersectRay(const Ray &ray, std::vector<Hit> *hits) override;
  Vector3f GetNormal(Vector3f pos) override;
};

class CsgCube : public CsgPrimitive {
public:
  void IntersectRay(const Ray &ray, std::vector<Hit> *hits) override;
  Vector3f GetNormal(Vector3f pos) override;
};

std::vector<Ray> TestIntersectRay();

#endif

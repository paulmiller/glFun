#include "csg.h"

#include "math/util.h"

#include <cmath>
#include <algorithm>

// slab method
void CsgCube::IntersectRay(const Ray &ray, std::vector<Hit> *hits) {
  const Vector3f &S = ray.start, &D = ray.direction;

  static_assert(std::numeric_limits<float>::has_infinity);
  float t_max = std::numeric_limits<float>::infinity();
  float t_min = -t_max;

  if(D.x != 0) {
    float t1 = ( 1 - S.x) / D.x; // intersect ray with the x = 1 plane
    float t2 = (-1 - S.x) / D.x; // the x = -1 plane
    t_min = std::min(t1, t2);
    t_max = std::max(t1, t2);
  }

  if(D.y != 0) {
    float t1 = ( 1 - S.y) / D.y;
    float t2 = (-1 - S.y) / D.y;
    t_min = std::max(t_min, std::min(t1, t2));
    t_max = std::min(t_max, std::max(t1, t2));
  }

  if(D.z != 0) {
    float t1 = ( 1 - S.z) / D.z;
    float t2 = (-1 - S.z) / D.z;
    t_min = std::max(t_min, std::min(t1, t2));
    t_max = std::min(t_max, std::max(t1, t2));
  }

  if(t_min < t_max) {
    if(t_min >= 0) {
      // the ray starts outside the cube, enters it at t_min, and exits at t_max
      hits->push_back({t_min, true});
      hits->push_back({t_max, false});
    } else if(t_max >= 0) {
      // the ray starts inside the cube and exits at t_max
      hits->push_back({t_max, false});
    }
  }
}

/*
sphere equation:
  r² = x² + y² + z²

ray equation:
  S + t⋅D

solve for t:
  r² = (Sx + t⋅Dx)² + (Sy + t⋅Dy)² + (Sz + t⋅Dz)²
  r² = (Sx² + 2⋅Sx⋅t⋅Dx + t²⋅Dx²) + (Sy² + 2⋅Sy⋅t⋅Dy + t²⋅Dy²) + (Sz² + ...
  r² = (Dx²+Dy²+Dz²)⋅t² + (2⋅Sx⋅Dx + 2⋅Sy⋅Dy + 2⋅Sz⋅Dz)⋅t + (Sx²+Sy²+Sz²)
  0  = (Dx²+Dy²+Dz²)⋅t² + 2⋅(Sx⋅Dx + Sy⋅Dy + Sz⋅Dz)⋅t + (Sx²+Sy²+Sz² - r²)
  ...then use the quadratic formula
*/
void CsgSphere::IntersectRay(const Ray &ray, std::vector<Hit> *hits) {
  const Vector3f &S = ray.start, &D = ray.direction;

  float a = D.x*D.x + D.y*D.y + D.z*D.z;
  float b = 2 * (S.x*D.x + S.y*D.y + S.z*D.z);
  float c = S.x*S.x + S.y*S.y + S.z*S.z - radius*radius;

  float square = b*b - 4*a*c; // the part under the quadratic formula's radical
  if(square < 0) return;
  float root = sqrt(square);
  float t1 = (-b - root) / (2*a);
  float t2 = (-b + root) / (2*a);

  if(t2 >= 0) {
    if(t1 >= 0)
      hits->push_back({t1, true});
    hits->push_back({t2, false});
  }
}

#include <iostream>

std::vector<Ray> TestIntersectRay() {
  int rows = 40, cols = 40;

  CsgCube cube;

  Camera camera;
  camera.setResolution(cols, rows);
  camera.setFrustum(0.01, 100.0, Tau_f/4, 1);
  camera.look(Vector3f{3,3,0}, Vector3f{-1,-1,0}, Vector3f{0,1,0});
  std::vector<Ray> rays = MakeCameraRays(camera);

  std::vector<int> hit_counts;
  hit_counts.reserve(rays.size());

  std::vector<CsgPrimitive::Hit> hits;
  for(const Ray &ray: rays) {
    cube.IntersectRay(ray, &hits);
    hit_counts.push_back(hits.size());
    hits.clear();
  }

  int i = 0;
  for(int row = 0; row < rows; row++) {
    for(int col = 0; col < cols; col++) {
      if(col) std::cout << ' ';
      std::cout << hit_counts[i];
      i++;
    }
    std::cout << '\n';
  }

  return rays;
}

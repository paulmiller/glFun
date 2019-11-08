#include "csg.h"

#include "math/util.h"
#include "image_png.h"

#include <algorithm>
#include <cmath>
#include <fstream>

std::ostream& operator<<(std::ostream &os, const CsgNode::Hit &hit) {
  os << (hit.entering ? u8"↘" : u8"↗");
  os << hit.distance;
  return os;
}

void CsgUnion::IntersectRay(const Ray &ray, std::vector<Hit> *hits) {
  // sounds bad, but can't think of a more concise variable name (ಠ ~ ಠ)
  std::vector<Hit> child_hits;
  a->IntersectRay(ray, &child_hits);
  size_t a_hit_num = child_hits.size();
  b->IntersectRay(ray, &child_hits);
  auto begin = child_hits.begin();
  std::inplace_merge(begin, begin + a_hit_num, child_hits.end());
  // "child_hits" is now sorted

  // find "start_inside", which is the number of child shapes (0-2) the ray
  // starts inside of
  int entries = 0, exits = 0;
  for(Hit &hit: child_hits) {
    if(hit.entering)
      entries++;
    else
      exits++;
  }
  int start_inside = exits - entries;
  assert(0 <= start_inside); assert(start_inside <= 2);

  int currently_inside = start_inside;
  for(Hit &hit: child_hits) {
    int previously_inside = currently_inside;
    if(hit.entering)
      currently_inside++;
    else
      currently_inside--;
    assert(0 <= currently_inside); assert(currently_inside <= 2);
    if(previously_inside == 0) {
      // we just entered the union-shape
      assert(1 <= currently_inside);
      hits->push_back(hit);
    } else if(currently_inside == 0) {
      // we just exited the union-shape
      assert(1 <= previously_inside);
      hits->push_back(hit);
    }
  }
}

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
      hits->push_back({this, t_min, true});
      hits->push_back({this, t_max, false});
    } else if(t_max >= 0) {
      // the ray starts inside the cube and exits at t_max
      hits->push_back({this, t_max, false});
    }
  }
}

Vector3f CsgCube::GetNormal(Vector3f pos) {
  float abs_x = std::abs(pos.x);
  float abs_y = std::abs(pos.y);
  float abs_z = std::abs(pos.z);
  if(abs_x > abs_y && abs_x > abs_z) {
    if(pos.x < 0)
      return -UnitX_Vector3f;
    else
      return UnitX_Vector3f;
  }

  if(abs_y > abs_z) {
    if(pos.y < 0)
      return -UnitY_Vector3f;
    else
      return UnitY_Vector3f;
  }

  if(pos.z < 0)
    return -UnitZ_Vector3f;
  else
    return UnitZ_Vector3f;
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
      hits->push_back({this, t1, true});
    hits->push_back({this, t2, false});
  }
}

Vector3f CsgSphere::GetNormal(Vector3f pos) {
  // if "pos" is on the sphere surface, then pos.len() == radius, so use that to
  // normalize "pos"
  return pos / radius;
}

std::vector<Ray> TestIntersectRay() {
  int rows = 1024, cols = 1024;

  CsgUnion combination {
    std::make_unique<CsgSphere>(1.3), std::make_unique<CsgCube>() };

  Camera camera;
  camera.setResolution(cols, rows);
  camera.setFrustum(0.01, 100.0, Tau_f/6, 1);
  camera.lookAt(Vector3f{1,2,3}, Zero_Vector3f, UnitY_Vector3f);
  std::vector<Ray> rays = MakeCameraRays(camera);

  Image img(cols, rows, Pixel::V8_T);
  Pixel::V8 *px = (Pixel::V8*) img.data();

  std::vector<CsgPrimitive::Hit> hits;
  for(const Ray &ray: rays) {
    combination.IntersectRay(ray, &hits);
    if(hits.size()) {
      Vector3f pos = ray.start + hits[0].distance * ray.direction;
      Vector3f normal = hits[0].primitive->GetNormal(pos);
      float v = std::clamp(dot(normal, UnitY_Vector3f) * 255.0f, 10.0f, 255.0f);
      *px = {uint8_t(v)};
    } else {
      *px = {0};
    }
    px++;
    hits.clear();
  }

  std::ofstream out("out.png", std::ofstream::binary);
  writePng(out, img);
  out.close();

  return rays;
}

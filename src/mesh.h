#ifndef MESH_H
#define MESH_H

#include "math3d.h"

#include <iostream>
#include <string>
#include <vector>

class UVCoord {
public:
  float u, v;

  UVCoord(float u, float v);
};

class Tri {
public:
  int vert_idxs[3];
  int normal_idxs[3];
  int uv_idxs[3];

  Tri(int v1, int v2, int v3);
  Tri(
    int v1, int v2, int v3,
    int n1, int n2, int n3,
    int t1, int t2, int t3
  );
};

class Mesh {
public:
  std::string name;
  std::vector<Vec3> verts;
  std::vector<Vec3> normals;
  std::vector<UVCoord> uvs;
  std::vector<Tri> tris;

public:
  Mesh(const char* name);
  // Check for intersection with a line segment
  // TODO currently a ray
  bool intersects(const Vec3 &start, const Vec3 &end) const;
  // Parse Wavefront OBJ format
  static std::vector<Mesh> parseObj(std::istream &input);
};

#endif

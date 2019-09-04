#ifndef MESH_H
#define MESH_H

#include "math3d.h"

#include <iostream>
#include <string>
#include <vector>

// a texture coordinate
class UVCoord {
public:
  float u, v;

  UVCoord(float u, float v);
};

// a triangle
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
  Tri(int (&v)[3], int (&n)[3], int (&t)[3]);
};

// a mesh of triangles
class TriMesh {
public:
  std::vector<Vec3> verts;
  std::vector<Vec3> normals;
  std::vector<UVCoord> uvs;
  std::vector<Tri> tris;

public:
  // Check for intersection with a line segment
  // TODO currently a ray
  bool intersects(const Vec3 &start, const Vec3 &end) const;
};

#endif

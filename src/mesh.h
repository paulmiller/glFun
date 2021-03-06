#ifndef MESH_H
#define MESH_H

#include "color.h"
#include "math/matrix.h"
#include "math/vector.h"

#include <iostream>
#include <string>
#include <vector>

// a texture coordinate
class UvCoord {
public:
  float u, v;

  UvCoord(float u, float v);
};

// a triangle
class Tri {
public:
  // indices into TriMesh's verts, normals, & uvs vectors
  int vert_idxs[3];
  int normal_idxs[3];
  union { // if TriMesh::has_color == false, use uv_idxs; otherwise, use color
    int uv_idxs[3];
    Color color;
  };

  Tri(int v1, int v2, int v3);
  Tri(
    int v1, int v2, int v3,
    int n1, int n2, int n3,
    int t1, int t2, int t3
  );
  Tri(
    int v1, int v2, int v3,
    int n1, int n2, int n3,
    Color c
  );
  Tri(int (&v)[3], int (&n)[3], int (&t)[3]);
};

// a mesh of triangles
class TriMesh {
public:
  std::vector<Vector3f> verts;
  std::vector<Vector3f> normals;
  std::vector<UvCoord> uvs;
  std::vector<Tri> tris;
  bool has_color = false;

  // Check for intersection with a line segment
  bool Intersects(const Vector3f &start, const Vector3f &end) const;
  void Transform(const Matrix4x4f &m);
  void Merge(const TriMesh &src);
};

#endif

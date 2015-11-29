#ifndef MESH_H
#define MESH_H

#include "math3d.h"

#include <iostream>
#include <string>
#include <vector>

class UVCoord {
public:
  float u, v;

  UVCoord(float u_, float v_);
};

class Tri {
public:
  int vertIdxs[3];
  int normalIdxs[3];
  int uvIdxs[3];

  Tri(int v1, int v2, int v3);
  Tri(
    int v1, int v2, int v3,
    int n1, int n2, int n3,
    int t1, int t2, int t3
  );
};

class Mesh {
public:
  std::string mName;
  std::vector<Vec3> mVerts;
  std::vector<Vec3> mNormals;
  std::vector<UVCoord> mUVs;
  std::vector<Tri> mTris;

public:
  Mesh(const char* name);
  static std::vector<Mesh> parseObj(std::istream &input);
};

#endif

#include "mesh.h"

#include "util.h"

#include <cstring>

UvCoord::UvCoord(float u, float v) : u(u), v(v) {}

Tri::Tri(int v1, int v2, int v3) :
  vert_idxs{v1, v2, v3},
  normal_idxs{-1, -1, -1},
  uv_idxs{-1, -1, -1}
{}

Tri::Tri(
  int v1, int v2, int v3,
  int n1, int n2, int n3,
  int t1, int t2, int t3
) :
  vert_idxs{v1, v2, v3},
  normal_idxs{n1, n2, n3},
  uv_idxs{t1, t2, t3}
{}

Tri::Tri(int (&v)[3], int (&n)[3], int (&t)[3]) {
  memcpy(vert_idxs, &v, sizeof(vert_idxs));
  memcpy(normal_idxs, &n, sizeof(normal_idxs));
  memcpy(uv_idxs, &t, sizeof(uv_idxs));
}

namespace {
  // Test whether a ray starting at E and extending in direction D intersects a
  // triangle with vertices A, B, and C.
  bool intersects(
    const Vec3 &E, const Vec3 &D,
    const Vec3 &A, const Vec3 &B, const Vec3 &C
  ) {
    /* Solve:
    E + tD = A + b(B-A) + c(C-A)

    tD - b(B-A) - c(C-A) = A-E
    tD + b(A-B) + c(A-C) = A-E

    [           ]   [ t ]   [     ]
    [ D A-B A-C ] x [ b ] = [ A-E ]
    [           ]   [ c ]   [     ] */

    Vec3 BA = A - B;
    Vec3 CA = A - C;
    Vec3 EA = A - E;

    float d = det(D, BA, CA);

    if(d == 0)
      return false;

    float t = det(EA, BA, CA) / d;
    float b = det( D, EA, CA) / d;
    float c = det( D, BA, EA) / d;

    return (0 <= t && t <= 1) && (0 <= b && b <= 1) && (0 <= c && c <= 1) &&
      (b + c <= 1);
  }

  /*
  assert(intersects(
    Vec3(0,0,0), Vec3(1,1,1),
    Vec3(1,0,0), Vec3(0,1,0), Vec3(0,0,1)
  ));
  assert(!intersects(
    Vec3(0,0,0), Vec3(-1,1,1),
    Vec3(1,0,0), Vec3(0,1,0), Vec3(0,0,1)
  ));
  assert(!intersects(
    Vec3(0,0,0), Vec3(1,1,1),
    Vec3(0,0,1), Vec3(1,0,1), Vec3(0,1,1)
  ));
  */
}

#include <iostream>

bool TriMesh::intersects(const Vec3 &start, const Vec3 &end) const {
  for(auto tri_it = tris.begin(); tri_it != tris.end(); ++tri_it) {
    const Vec3 &A = verts[tri_it->vert_idxs[0]];
    const Vec3 &B = verts[tri_it->vert_idxs[1]];
    const Vec3 &C = verts[tri_it->vert_idxs[2]];
    if(::intersects(start, end, A, B, C))
      return true;
  }
  return false;
}

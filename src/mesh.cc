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
    /*
    The line segment is given by:

    E + t(D-E)
    t ∈ [0,1]

    The triangle is given by:

    A + b(B-A) + c(C-A)
    b & c ∈ [0,1]
    b + c <= 1

    Set the triangle equal to the line and solve for b, c, & t:

    A + b(B-A) + c(C-A)          = E + t(D-E)
        b(B-A) + c(C-A) - t(D-E) = E          - A

    Rewritten with matrices:

    [               ]   [ b ]   [     ]
    [ B-A  C-A  D-E ] x [ c ] = [ E-A ]
    [               ]   [ t ]   [     ]

    [ b ]   [               ]-1   [     ]
    [ c ] = [ B-A  C-A  D-E ]   x [ E-A ]
    [ t ]   [               ]     [     ]
    */

    Mat3 mat(B-A, C-A, D-E);

    float det = mat.determinate();
    if(det == 0) return false;

    Mat3 inv = mat.transpose() / det;
    Vec3 bct = inv * Vec3(E-A);
    float b = bct.x;
    float c = bct.y;
    float t = bct.t;
    return
      0 <= b && b <= 1 &&
      0 <= c && c <= 1 &&
      0 <= t && t <= 1 &&
      b + c <= 1;
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

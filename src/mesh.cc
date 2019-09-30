#include "mesh.h"

#include "math/matrix_factories.h"
#include "math/matrix_vector_product.h"
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
    const Vector3f &E, const Vector3f &D,
    const Vector3f &A, const Vector3f &B, const Vector3f &C
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

    // TODO proper inverse matrix function
    Matrix3x3f m = MatrixFromColumnVectors(B-A, C-A, D-E);
    float d = m.determinant();
    if(d == 0) return false;
    m = m.transpose() / d;
    Vector3f bct = m * (E-A);
    float b = bct.x;
    float c = bct.y;
    float t = bct.z;
    return
      0 <= b && b <= 1 &&
      0 <= c && c <= 1 &&
      0 <= t && t <= 1 &&
      b + c <= 1;
  }

  /*
  assert(intersects(
    Vector3f(0,0,0), Vector3f(1,1,1),
    Vector3f(1,0,0), Vector3f(0,1,0), Vector3f(0,0,1)
  ));
  assert(!intersects(
    Vector3f(0,0,0), Vector3f(-1,1,1),
    Vector3f(1,0,0), Vector3f(0,1,0), Vector3f(0,0,1)
  ));
  assert(!intersects(
    Vector3f(0,0,0), Vector3f(1,1,1),
    Vector3f(0,0,1), Vector3f(1,0,1), Vector3f(0,1,1)
  ));
  */
}

#include <iostream>

bool TriMesh::intersects(const Vector3f &start, const Vector3f &end) const {
  for(auto tri_it = tris.begin(); tri_it != tris.end(); ++tri_it) {
    const Vector3f &A = verts[tri_it->vert_idxs[0]];
    const Vector3f &B = verts[tri_it->vert_idxs[1]];
    const Vector3f &C = verts[tri_it->vert_idxs[2]];
    if(::intersects(start, end, A, B, C))
      return true;
  }
  return false;
}

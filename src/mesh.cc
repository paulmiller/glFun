#include "mesh.h"

#include "math/matrix_factories.h"
#include "math/matrix_vector_product.h"
#include "util.h"

#include <cstring>

UvCoord::UvCoord(float u, float v) : u(u), v(v) {}

Tri::Tri(int v1, int v2, int v3) :
  vert_idxs{v1, v2, v3},
  normal_idxs{-1, -1, -1}
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

Tri::Tri(
  int v1, int v2, int v3,
  int n1, int n2, int n3,
  Color c
) :
  vert_idxs{v1, v2, v3},
  normal_idxs{n1, n2, n3},
  color(c)
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

bool TriMesh::Intersects(const Vector3f &start, const Vector3f &end) const {
  for(auto tri_it = tris.begin(); tri_it != tris.end(); ++tri_it) {
    const Vector3f &A = verts[tri_it->vert_idxs[0]];
    const Vector3f &B = verts[tri_it->vert_idxs[1]];
    const Vector3f &C = verts[tri_it->vert_idxs[2]];
    if(::intersects(start, end, A, B, C))
      return true;
  }
  return false;
}

void TriMesh::Transform(const Matrix4x4f &m) {
  for(Vector3f &v3: verts) {
    Vector4f v4 = Vector4f{v3.x, v3.y, v3.z, 1};
    v4 = m * v4;
    v3 = v4.divideByW();
  }
}

void TriMesh::Merge(const TriMesh &src) {
  assert(has_color == src.has_color);

  size_t original_verts_size   = verts.size();
  size_t original_normals_size = normals.size();
  size_t original_uvs_size     = uvs.size();
  size_t original_tris_size    = tris.size();

  verts.reserve(original_verts_size + src.verts.size());
  normals.reserve(original_normals_size + src.normals.size());
  if(!has_color)
    uvs.reserve(original_uvs_size + src.uvs.size());
  tris.reserve(original_tris_size + src.tris.size());

  verts.insert(verts.end(), src.verts.begin(), src.verts.end());
  normals.insert(normals.end(), src.normals.begin(), src.normals.end());
  if(!has_color)
    uvs.insert(uvs.end(), src.uvs.begin(), src.uvs.end());

  for(const Tri &tri: src.tris) {
    Tri offset_tri = tri;
    for(int i = 0; i < 3; i++) {
      offset_tri.vert_idxs[i] += original_verts_size;
      if(offset_tri.normal_idxs[i] != -1)
        offset_tri.normal_idxs[i] += original_normals_size;
      if(!has_color) {
        if(offset_tri.uv_idxs[i] != -1)
          offset_tri.uv_idxs[i] += original_uvs_size;
      }
    }
    tris.push_back(offset_tri);
  }
}

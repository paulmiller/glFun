#include "mesh.h"

#include "util.h"

UVCoord::UVCoord(float u, float v) : u(u), v(v) {}

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

Mesh::Mesh(const char* name) : name(name) {}

std::vector<Mesh> Mesh::parseObj(std::istream &input) {
  static const int MAX_LINE = 4096;

  std::vector<Mesh> objects;
  char line[MAX_LINE];
  char object_name[MAX_LINE];
  while(true) {
    input.getline(line, sizeof(line));

    if(input.eof()) {
      break;
    }

    if (input.fail()) {
      std::cout << "objMesh: input fail\n";
      break;
    }

    // Ensure there is at least 1 object in case this file doesn't have "o"
    // lines
    auto provideObject = [&objects]() {
      if(objects.empty()) {
        objects.emplace_back("");
      }
    };

    float x, y, z, u, v;
    unsigned v1, v2, v3, n1, n2, n3, t1, t2, t3;
    if(
      hasPrefix("#", line) ||
      hasPrefix("g", line) ||
      hasPrefix("mtllib", line) ||
      hasPrefix("usemtl", line)
    ) {
      // Ignore comments, groups, and materials.
    } else if(1 == sscanf(line, "o %s", object_name)) {
      // Start a new object
      objects.emplace_back(object_name);
    } else if(3 == sscanf(line, "v %f %f %f", &x, &y, &z)) {
      // Add a vertex
      provideObject();
      objects.back().verts.emplace_back(x, y, z);
    } else if(2 == sscanf(line, "vt %f %f", &u, &v)) {
      // Add a UV coordinate
      provideObject();
      objects.back().uvs.emplace_back(u, 1.0f - v);
    } else if(3 == sscanf(line, "vn %f %f %f", &x, &y, &z)) {
      // Add a normal vector
      provideObject();
      objects.back().normals.emplace_back(x, y, z);
    } else if(6 ==
        sscanf(line, "f %u %u %u", &v1, &v2, &v3)) {
      // TODO Add a triangle
      provideObject();
      objects.back().tris.emplace_back(v1-1, v2-1, v3-1);
    } else if(6 == sscanf(line, "f %u/%u %u/%u %u/%u",
        &v1, &t1, &v2, &t2, &v3, &t3)) {
      // Add a triangle with UV
      provideObject();
      objects.back().tris.emplace_back(
          int(v1)-1, int(v2)-1, int(v3)-1,
                 -1,        -1,        -1,
          int(t1)-1, int(t2)-1, int(t3)-1
      );
    } else if(6 == sscanf(line, "f %u//%u %u//%u %u//%u",
        &v1, &n1, &v2, &n2, &v3, &n3)) {
      // Add a triangle with normals
      provideObject();
      objects.back().tris.emplace_back(
        int(v1)-1, int(v2)-1, int(v3)-1, 
        int(n1)-1, int(n2)-1, int(n3)-1,
               -1,        -1,        -1
      );
    } else if(9 == sscanf(line, "f %u/%u/%u %u/%u/%u %u/%u/%u",
        &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3)) {
      // Add a triangle with UV and normals
      provideObject();
      objects.back().tris.emplace_back(
          int(v1)-1, int(v2)-1, int(v3)-1,
          int(n1)-1, int(n2)-1, int(n3)-1,
          int(t1)-1, int(t2)-1, int(t3)-1
      );
    } else {
      std::cout << "objMesh: unrecognized line: " << line << std::endl;;
    }
  }

  // Verify each object's triangles' vert, normal, and UV indices are within
  // bounds.
  for(auto object_it = objects.begin(); object_it != objects.end(); object_it++) {
    int vert_num = object_it->verts.size();
    int normal_num = object_it->normals.size();
    int uv_num = object_it->uvs.size();
      std::cout << "objMesh: loaded object \"" << object_it->name << "\" with "
          << vert_num << " vertices, " << normal_num << " normals, "
          << uv_num << " UVs, " << object_it->tris.size() << " triangles\n";
    int removed = 0;
    for(auto tri_it = object_it->tris.begin(); tri_it != object_it->tris.end();) {
      bool in_bounds = true;
      for(int i = 0; i < 3; i++) {
        if(tri_it->vert_idxs[i] >= vert_num ||
            tri_it->normal_idxs[i] >= normal_num ||
            tri_it->uv_idxs[i] >= uv_num) {
          in_bounds = false;
          break;
        }
      }
      if(in_bounds) {
        tri_it++;
      } else {
        tri_it = object_it->tris.erase(tri_it);
        removed++;
      }
    }
    if(removed) {
      std::cout << "objMesh: object \"" << object_it->name
          << "\" indices out of range; removed " << removed << " triangle(s)\n";
    }
  }

  return objects;
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

bool Mesh::intersects(const Vec3 &start, const Vec3 &end) const {
  for(auto tri_it = tris.begin(); tri_it != tris.end(); ++tri_it) {
    const Vec3 &A = verts[tri_it->vert_idxs[0]];
    const Vec3 &B = verts[tri_it->vert_idxs[1]];
    const Vec3 &C = verts[tri_it->vert_idxs[2]];
    if(::intersects(start, end, A, B, C))
      return true;
  }
  return false;
}

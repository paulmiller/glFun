#include "mesh.h"

#include "util.h"

UVCoord::UVCoord(float u_, float v_) : u(u_), v(v_) {}

Tri::Tri(int v1, int v2, int v3) :
  vertIdxs{v1, v2, v3},
  normalIdxs{-1, -1, -1},
  uvIdxs{-1, -1, -1}
{}

Tri::Tri(
  int v1, int v2, int v3,
  int n1, int n2, int n3,
  int t1, int t2, int t3
) :
  vertIdxs{v1, v2, v3},
  normalIdxs{n1, n2, n3},
  uvIdxs{t1, t2, t3}
{}

Mesh::Mesh(const char* name) : mName(name) {}

std::vector<Mesh> Mesh::parseObj(std::istream &input) {
  static const int MAX_LINE = 4096;

  std::vector<Mesh> objects;
  char line[MAX_LINE];
  char objectName[MAX_LINE];
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
    } else if(1 == sscanf(line, "o %s", objectName)) {
      // Start a new object
      objects.emplace_back(objectName);
    } else if(3 == sscanf(line, "v %f %f %f", &x, &y, &z)) {
      // Add a vertex
      provideObject();
      objects.back().mVerts.emplace_back(x, y, z);
    } else if(2 == sscanf(line, "vt %f %f", &u, &v)) {
      // Add a UV coordinate
      provideObject();
      objects.back().mUVs.emplace_back(u, v);
    } else if(3 == sscanf(line, "vn %f %f %f", &x, &y, &z)) {
      // Add a normal vector
      provideObject();
      objects.back().mNormals.emplace_back(x, y, z);
    } else if(6 ==
        sscanf(line, "f %u %u %u", &v1, &v2, &v3)) {
      // TODO Add a triangle
      provideObject();
      objects.back().mTris.emplace_back(v1-1, v2-1, v3-1);
    } else if(6 == sscanf(line, "f %u/%u %u/%u %u/%u",
        &v1, &t1, &v2, &t2, &v3, &t3)) {
      // Add a triangle with UV
      provideObject();
      objects.back().mTris.emplace_back(
          int(v1)-1, int(v2)-1, int(v3)-1,
                 -1,        -1,        -1,
          int(t1)-1, int(t2)-1, int(t3)-1
      );
    } else if(6 == sscanf(line, "f %u//%u %u//%u %u//%u",
        &v1, &n1, &v2, &n2, &v3, &n3)) {
      // Add a triangle with normals
      provideObject();
      objects.back().mTris.emplace_back(
        int(v1)-1, int(v2)-1, int(v3)-1, 
        int(n1)-1, int(n2)-1, int(n3)-1,
               -1,        -1,        -1
      );
    } else if(9 == sscanf(line, "f %u/%u/%u %u/%u/%u %u/%u/%u",
        &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3)) {
      // Add a triangle with UV and normals
      provideObject();
      objects.back().mTris.emplace_back(
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
  for(auto objectIt = objects.begin(); objectIt != objects.end(); objectIt++) {
    int vertNum = objectIt->mVerts.size();
    int normalNum = objectIt->mNormals.size();
    int uvNum = objectIt->mUVs.size();
      std::cout << "objMesh: loaded object \"" << objectIt->mName << "\" with "
          << vertNum << " vertices, " << normalNum << " normals, "
          << uvNum << " UVs, " << objectIt->mTris.size() << " triangles\n";
    int removed = 0;
    for(auto triIt = objectIt->mTris.begin(); triIt != objectIt->mTris.end();) {
      bool inBounds = true;
      for(int i = 0; i < 3; i++) {
        if(triIt->vertIdxs[i] >= vertNum ||
            triIt->normalIdxs[i] >= normalNum ||
            triIt->uvIdxs[i] >= uvNum) {
          inBounds = false;
          break;
        }
      }
      if(inBounds) {
        triIt++;
      } else {
        triIt = objectIt->mTris.erase(triIt);
        removed++;
      }
    }
    if(removed) {
      std::cout << "objMesh: object \"" << objectIt->mName
          << "\" indices out of range; removed " << removed << " triangle(s)\n";
    }
  }

  return objects;
}


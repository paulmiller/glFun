#include "../src/mesh_obj.h"

#include <cassert>
#include <climits>
#include <cstdio>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

/*!re2c
  re2c:indent:string = '  ';
  re2c:define:YYCTYPE = char;
  re2c:yyfill:enable = 0;
  re2c:flags:tags = 1;

  D = [0-9];
  N = D+;                           // natural number
  R = ('-'? (D+'.'?D* | D*'.'?D+)); // real number
  S = [ \t];                        // whitespace
  E = ([\n\r]+ | '\x00');           // end of line or null terminator
*/

#define YYMTAGP(tags) tags.push_back(YYCURSOR);

namespace {
  typedef std::vector<const char*> Mtag;

  // MtagPtr wraps an Mtag vector and overrides operator= to take a pointer
  // rather than copying the value. Then we can have an MtagPtr called "tags",
  // and when re2c generates the code "tags = yyt<N>", it will invoke our
  // operator=, and avoid making a copy of the yyt<N>.
  class MtagPtr {
  public:
    MtagPtr() : ptr_(nullptr) {}

    Mtag& operator=(Mtag& o) {
      ptr_ = &o;
      return o;
    }

    Mtag* operator->() {
      return ptr_;
    }

    const char* operator[](int i) {
      return (*ptr_)[i];
    }

    Mtag::iterator begin() { return ptr_->begin(); }
    Mtag::iterator end() { return ptr_->end(); }

  private:
    Mtag* ptr_;
  };
}

WavFrObj::ObjFace::ObjFace(std::vector<ObjVert> &&verts) :
  verts(std::move(verts)) {}

WavFrObj::ObjObject::ObjObject(std::string &&name) :
  name(std::move(name)), min_sides(0), max_sides(0) {}

void WavFrObj::ObjObject::addFace(std::vector<ObjVert> &&verts) {
  int sides = int(verts.size());
  assert(sides >= 3);
  if(min_sides == 0 || sides < min_sides) min_sides = sides;
  if(max_sides == 0 || sides > max_sides) max_sides = sides;
  faces.emplace_back(std::move(verts));
}

TriMesh WavFrObj::ObjObject::getTriMesh(const WavFrObj *source) const {
  TriMesh mesh;
  std::unordered_map<int, int> vert_id_map;
  std::unordered_map<int, int> normal_id_map;
  std::unordered_map<int, int> uv_id_map;
  int next_vert_id = 0, next_normal_id = 0, next_uv_id = 0;
  for(const ObjFace &face: faces) {
    int vert_idxs[3];
    int uv_idxs[3];
    int normal_idxs[3];
    int vert_num = 0;
    for(const ObjVert &vert: face.verts) {
      auto vert_it = vert_id_map.find(vert.vert_id);
      if(vert_it == vert_id_map.end()) {
        vert_it = vert_id_map.insert(
          std::make_pair(vert.vert_id, next_vert_id)).first;
        next_vert_id++;
        mesh.verts.push_back(source->verts_[vert.vert_id]);
        assert(mesh.verts.size() == size_t(next_vert_id));
      }
      vert_idxs[vert_num] = vert_it->second;

      if(vert.uv_id == -1) {
        uv_idxs[vert_num] = -1;
      } else {
        auto uv_it = uv_id_map.find(vert.uv_id);
        if(uv_it == uv_id_map.end()) {
          uv_it = uv_id_map.insert(
            std::make_pair(vert.uv_id, next_uv_id)).first;
          next_uv_id++;
          mesh.uvs.push_back(source->uvs_[vert.uv_id]);
          assert(mesh.uvs.size() == size_t(next_uv_id));
        }
        uv_idxs[vert_num] = uv_it->second;
      }

      if(vert.normal_id == -1) {
        normal_idxs[vert_num] = -1;
      } else {
        auto normal_it = normal_id_map.find(vert.normal_id);
        if(normal_it == normal_id_map.end()) {
          normal_it = normal_id_map.insert(
            std::make_pair(vert.normal_id, next_normal_id)).first;
          next_normal_id++;
          mesh.normals.push_back(source->normals_[vert.normal_id]);
          assert(mesh.normals.size() == size_t(next_normal_id));
        }
        normal_idxs[vert_num] = normal_it->second;
      }

      vert_num++;
      if(vert_num == 3) break; // TODO: triangulate faces with > 3 sides
    }

    mesh.tris.emplace_back(vert_idxs, normal_idxs, uv_idxs);
  }
  std::cout << "WavFrObj::ObjObject::getTriMesh created mesh with "
    << mesh.verts.size() << " vertices, " << mesh.uvs.size() << " UVs, "
    << mesh.normals.size() << " normals, " << mesh.tris.size() << " tris\n";
  return mesh;
}

void WavFrObj::clear() {
  verts_.clear();
  normals_.clear();
  uvs_.clear();
  objects_.clear();
}

void WavFrObj::parseFrom(const std::string& input) {
  clear();

  const char* const start = input.c_str();

  const char *YYMARKER = nullptr;
  const char *YYCURSOR = start;
  const char *YYLIMIT = YYCURSOR + input.size();

  // tag variables
  const char *name_start, *name_end; // demarcate an object name
  const char /*!stags:re2c format = "*@@ = nullptr"; separator = ", ";*/;
  Mtag /*!mtags:re2c format = "@@"; separator = ", ";*/;

  // After matching, re2c will set "tags" to refer to one of the m-tag vectors,
  // above.
  MtagPtr tags;

  static const char* const badFaceMsg =
    "WavFrObj::parseFrom failed: face with out-of-bounds value at offset ";

  // lambda which gets the current offset into the input
  auto getOffset = [&]() -> uintptr_t { return uintptr_t(YYCURSOR - start); };

  int total_faces = 0;

  while(YYCURSOR < YYLIMIT) {
    /*!re2c
    * { 
      std::cout << "WavFrObj::parseFrom failed: unrecognized line at offset "
        << getOffset() << std::endl;
      clear();
      return;
    }

    'v' (S+ #tags R){3} E {
      assert(tags->size() == 3);
      verts_.push_back(Vector3f{
        strtof(tags[0], nullptr),
        strtof(tags[1], nullptr),
        strtof(tags[2], nullptr)
      });
      tags->clear();
      continue;
    }

    'vt' (S+ #tags R){2} E {
      assert(tags->size() == 2);
      float u = strtof(tags[0], nullptr);
      float v = strtof(tags[1], nullptr);
      uvs_.emplace_back(u, 1.0f - v);
      tags->clear();
      continue;
    }

    'vn' (S+ #tags R){3} E {
      assert(tags->size() == 3);
      normals_.emplace_back(Vector3f{
        strtof(tags[0], nullptr),
        strtof(tags[1], nullptr),
        strtof(tags[2], nullptr)
      });
      tags->clear();
      continue;
    }

    // Parse a line of the form "f v v v...". Each element of the "tags"
    // vector points to the start of a vertex number "v".
    'f' (S+ #tags N){3,} E {
      assert(tags->size() >= 3);
      std::vector<ObjVert> face_verts;
      face_verts.reserve(tags->size());
      for(const char* str: tags) {
        unsigned long v = strtoul(str, nullptr, 10);
        if(v == 0 || v > INT_MAX) {
          std::cout << badFaceMsg << getOffset() << std::endl;
          return;
        }
        face_verts.push_back({int(v)-1, -1, -1});
      }
      tags->clear();
      addFaceToCurrentObject(std::move(face_verts));
      total_faces++;
      continue;
    }

    // Parse a line of the form "f v/t v/t v/t...". Each element of "tags"
    // points to the start of a vertex/texture coordinate pair "v/t".
    'f' (S+ #tags N '/' N){3,} E {
      assert(tags->size() >= 3);
      std::vector<ObjVert> face_verts;
      face_verts.reserve(tags->size());
      for(const char* str: tags) {
        unsigned long v, t;
        assert(2 == sscanf(str, "%lu/%lu", &v, &t)); // Parse a v/t pair.
        if(v == 0 || v > INT_MAX ||
           t == 0 || t > INT_MAX) {
          std::cout << badFaceMsg << getOffset() << std::endl;
          return;
        }
        face_verts.push_back({int(v)-1, int(t)-1, -1});
      }
      tags->clear();
      addFaceToCurrentObject(std::move(face_verts));
      total_faces++;
      continue;
    }

    // Parse a line of the form "f v//n v//n v//n...". Each element of the
    // "tags" vector points to the start of a vertex/normal pair "v//n".
    'f' (S+ #tags N '//' N){3,} E {
      assert(tags->size() >= 3);
      std::vector<ObjVert> face_verts;
      face_verts.reserve(tags->size());
      for(const char* str: tags) {
        unsigned long v, n;
        assert(2 == sscanf(str, "%zu//%zu", &v, &n)); // Parse a v//n pair.
        if(v == 0 || v > INT_MAX ||
           n == 0 || n > INT_MAX) {
          std::cout << badFaceMsg << getOffset() << std::endl;
          return;
        }
        face_verts.push_back({int(v)-1, -1, int(n)-1});
      }
      tags->clear();
      addFaceToCurrentObject(std::move(face_verts));
      total_faces++;
      continue;
    }

    // Parse a line of the form "f v/t/n v/t/n v/t/n...". Each element of the
    // "tags" vector points to the start of a "v/t/n" triple.
    'f' (S+ #tags N '/' N '/' N){3,} E {
      assert(tags->size() >= 3);
      std::vector<ObjVert> face_verts;
      face_verts.reserve(tags->size());
      for(const char* str: tags) {
        unsigned long v, t, n;
        // Parse a v/t/n triple.
        assert(3 == sscanf(str, "%zu/%zu/%zu", &v, &t, &n));
        if(v == 0 || v > INT_MAX ||
           t == 0 || t > INT_MAX ||
           n == 0 || n > INT_MAX) {
          std::cout << badFaceMsg << getOffset() << std::endl;
          return;
        }
        face_verts.push_back({int(v)-1, int(t)-1, int(n)-1});
      }
      tags->clear();
      addFaceToCurrentObject(std::move(face_verts));
      total_faces++;
      continue;
    }

    'o' S+ @name_start .* @name_end E {
      assert(name_start < name_end);
      std::string name = std::string(name_start, name_end - name_start);
      std::cout << "WavFrObj::parseFrom found object \"" << name << "\"\n";
      objects_.emplace_back(std::move(name));
      continue;
    }

    // ignore comments, groups, smoothing, & materials
    [#gs(mtllib)(usemtl)] .* E {
      continue;
    }

    E {
      continue;
    }
    */
  }

  std::cout << "WavFrObj::parseFrome found " << objects_.size()
    << " objects, " << verts_.size() << " vertices, " << uvs_.size()
    << " UVs, " << normals_.size() << " normals, " << total_faces << " faces\n";

  sanitize();
}

TriMesh WavFrObj::getTriMesh(std::string name) const {
  for(const ObjObject& object: objects_) {
    if(object.name == name) {
      return object.getTriMesh(this);
    }
  }
  std::cout << "WavFrObj::getTriMesh couldn't find object \"" << name << "\"\n";
  return TriMesh();
}

void WavFrObj::addFaceToCurrentObject(std::vector<ObjVert> &&verts) {
  if(objects_.size() == 0)
    objects_.emplace_back("");
  objects_.back().addFace(std::move(verts));
}

void WavFrObj::sanitize() {
  // Verify all vert_id, normal_id, and uv_id indices are within bounds.
  int verts_size   = int(verts_.size());
  int uvs_size     = int(uvs_.size());
  int normals_size = int(normals_.size());
  for(ObjObject &object: objects_) {
    int removed = 0;
    for(auto face_it = object.faces.begin();
        face_it != object.faces.end();) {
      bool in_bounds = true;
      for(ObjVert &vert: face_it->verts) {
        if(vert.vert_id   >= verts_size ||
           vert.uv_id     >= uvs_size   ||
           vert.normal_id >= normals_size) {
          in_bounds = false;
          break;
        }
      }
      if(in_bounds) {
        face_it++;
      } else {
        face_it = object.faces.erase(face_it);
        removed++;
      }
    }
    if(removed) {
      std::cout << "WavFrObj::sanitize object \"" << object.name
                << "\" had out-of-bounds indices; removed " << removed
                << " faces\n";
    }
  }
}

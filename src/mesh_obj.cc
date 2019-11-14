#include "mesh_obj.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <unordered_map>

void WavFrObj::ObjObject::addFace(std::vector<ObjVert> verts) {
  int sides = int(verts.size());
  assert(sides >= 3);
  if(min_sides == 0 || sides < min_sides) min_sides = sides;
  if(max_sides == 0 || sides > max_sides) max_sides = sides;
  faces.emplace_back(std::move(verts));
}

TriMesh WavFrObj::ObjObject::GetTriMesh(const WavFrObj *source) const {
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
  std::cout << "WavFrObj::ObjObject::GetTriMesh created mesh with "
    << mesh.verts.size() << " vertices, " << mesh.uvs.size() << " UVs, "
    << mesh.normals.size() << " normals, " << mesh.tris.size() << " tris\n";
  return mesh;
}

void WavFrObj::Clear() {
  verts_.clear();
  normals_.clear();
  uvs_.clear();
  objects_.clear();
}

TriMesh WavFrObj::GetTriMesh(std::string name) const {
  for(const ObjObject& object: objects_) {
    if(object.name == name) {
      return object.GetTriMesh(this);
    }
  }
  std::cout << "WavFrObj::GetTriMesh couldn't find object \"" << name << "\"\n";
  return TriMesh();
}

void WavFrObj::AddObjectFromTriMesh(std::string name, const TriMesh &mesh) {
  int original_verts_size = verts_.size();
  int original_uvs_size = uvs_.size();
  int original_normals_size = normals_.size();

  verts_  .reserve(original_verts_size   + mesh.verts  .size());
  uvs_    .reserve(original_uvs_size     + mesh.uvs    .size());
  normals_.reserve(original_normals_size + mesh.normals.size());

  verts_  .insert(verts_  .end(), mesh.verts  .begin(), mesh.verts  .end());
  uvs_    .insert(uvs_    .end(), mesh.uvs    .begin(), mesh.uvs    .end());
  normals_.insert(normals_.end(), mesh.normals.begin(), mesh.normals.end());

  objects_.emplace_back(std::move(name));
  ObjObject &object = objects_.back();

  for(const Tri &tri: mesh.tris) {
    object.faces.emplace_back();
    ObjFace &face = object.faces.back();

    for(int i = 0; i < 3; i++) {
      face.verts.push_back(ObjVert {
        tri.vert_idxs  [i] + original_verts_size,
        tri.uv_idxs    [i] + original_uvs_size,
        tri.normal_idxs[i] + original_normals_size
      });
    }
  }
}

std::string WavFrObj::Export() const {
  std::stringstream output;

  for(const Vector3f &vert: verts_)
    output << "v " << vert.x << ' ' << vert.y << ' ' << vert.z << '\n';
  for(const Vector3f &normal: normals_)
    output << "vn " << normal.x << ' ' << normal.y << ' ' << normal.z << '\n';
  for(const UvCoord &uv: uvs_)
    output << "vt " << uv.u << ' ' << uv.v << '\n';

  bool have_multiple_objects = objects_.size() > 1;
  int anonymous_object_ordinal = 1;

  for(const ObjObject &object: objects_) {
    if(object.name.empty()) {
      // if multiple objects, ensure all objects have names
      if(have_multiple_objects) {
        output << "o default" << anonymous_object_ordinal;
        anonymous_object_ordinal++;
      }
    } else {
      output << "o " << object.name << '\n';
    }

    for(const ObjFace &face: object.faces) {
      output << 'f';
      for(const ObjVert &face_vert: face.verts) {
        output << ' ' << (face_vert.vert_id + 1);

        if(face_vert.uv_id != -1)
          output << '/' << (face_vert.uv_id + 1);

        if(face_vert.normal_id != -1) {
          if(face_vert.uv_id == -1)
            output << "//";
          else
            output << '/';
          output << (face_vert.normal_id + 1);
        }
      }
      output << '\n';
    }
  }

  return output.str();
}

void WavFrObj::AddFaceToCurrentObject(std::vector<ObjVert> verts) {
  if(objects_.size() == 0)
    objects_.emplace_back("");
  objects_.back().addFace(std::move(verts));
}

void WavFrObj::Sanitize() {
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
      std::cout << "WavFrObj::Sanitize object \"" << object.name
                << "\" had out-of-bounds indices; removed " << removed
                << " faces\n";
    }
  }
}

#ifndef MESH_OBJ_H
#define MESH_OBJ_H

#include "math/vector.h"
#include "mesh.h"

#include <string>
#include <vector>

// parses a Wavefront OBJ file and holds the result
class WavFrObj {
public:
  class ObjVert {
  public:
    // Indexes into "verts_", "uvs_", and "normals_". "uv_id" and "normal_id"
    // may be -1 if absent for this vertex, but "vert_id" must be present. These
    // are 0-indexed, although OBJ files use 1-indexing.
    int vert_id;
    int uv_id;
    int normal_id;
  };

  class ObjFace {
  public:
    std::vector<ObjVert> verts;

    ObjFace() {}
    ObjFace(std::vector<ObjVert> &&verts) :
      verts(std::move(verts)) {}
  };

  class ObjObject {
  public:
    std::string name;
    std::vector<ObjFace> faces;
    int min_sides;
    int max_sides;

    ObjObject(std::string &&name) :
      name(std::move(name)), min_sides(0), max_sides(0) {}

    void addFace(std::vector<ObjVert> &&verts);

  private:
    TriMesh GetTriMesh(const WavFrObj *source) const;

    friend WavFrObj;
  };

  void Clear();

  // extract the object named "name", all its faces, and all the vertex info
  // those faces refer to, and repack them into a TriMesh
  TriMesh GetTriMesh(std::string name) const;

  void AddObjectFromTriMesh(std::string name, const TriMesh &mesh);

  // write a Wavefront OBJ string
  std::string Export() const;

  // read a Wavefront OBJ string
  void ParseFrom(const std::string &input);

private:
  // used by ParseFrom
  void AddFaceToCurrentObject(std::vector<ObjVert> &&verts);
  void Sanitize();

  std::vector<Vector3f> verts_; // vertex positions
  std::vector<UvCoord> uvs_; // texture coordinates (with V component flipped)
  std::vector<Vector3f> normals_; // normal vectors
  std::vector<ObjObject> objects_;
};

#endif

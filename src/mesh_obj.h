#ifndef MESH_OBJ_H
#define MESH_OBJ_H

#include "math3d.h"
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

    ObjFace(std::vector<ObjVert> &&verts);
  };

  class ObjObject {
  public:
    std::string name;
    std::vector<ObjFace> faces;
    int min_sides;
    int max_sides;

    ObjObject(std::string &&name);
    void addFace(std::vector<ObjVert> &&verts);

  private:
    TriMesh getTriMesh(const WavFrObj *source) const;

    friend WavFrObj;
  };

  void clear();
  void parseFrom(const std::string &input);
  // extract the object named "name", all its faces, and all the vertex info
  // those faces refer to, and repack them into a TriMesh
  TriMesh getTriMesh(std::string name) const;

private:
  void addFaceToCurrentObject(std::vector<ObjVert> &&verts);
  void sanitize();

  std::vector<Vec3> verts_; // vertex positions
  std::vector<UVCoord> uvs_; // texture coordinates (with V component flipped)
  std::vector<Vec3> normals_; // normal vectors
  std::vector<ObjObject> objects_;
};

#endif

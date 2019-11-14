#include "half_curve_mesh.h"

#include <algorithm>
#include <cassert>
#include <memory>

const HalfCurveMesh::Vertex &HalfCurveMesh::GetVertex(int id) const {
  assert(id >= 0);
  assert(id < vertices_.size());
  return vertices_[id];
}

const Vector3d &HalfCurveMesh::GetVertexPosition(int id) const {
  assert(id >= 0);
  assert(id < vertex_positions_.size());
  return vertex_positions_[id];
}

const Vector3d &HalfCurveMesh::GetVertexNormal(int id) const {
  assert(id >= 0);
  assert(id < vertex_normals_.size());
  return vertex_normals_[id];
}

const HalfCurveMesh::HalfCurve &HalfCurveMesh::GetCurve(int id) const {
  int index = id >> 2;
  assert(index >= 0);
  switch(static_cast<CurveType>(id & 0b11)) {
  case CurveType::Linear:
    assert(index < linear_curves_.size());
    return linear_curves_[index];
  case CurveType::Circular:
    assert(index < circular_curves_.size());
    return circular_curves_[index];
  case CurveType::Elliptical:
    assert(index < elliptical_curves_.size());
    return elliptical_curves_[index];
  default:
    assert(0);
    return *reinterpret_cast<HalfCurve*>(0);
  }
}

const HalfCurveMesh::LinearCurve &HalfCurveMesh::GetLinearCurve(int id) const {
  assert(static_cast<CurveType>(id & 0b11) == CurveType::Linear);
  int index = id >> 2;
  assert(index >= 0);
  assert(index < linear_curves_.size());
  return linear_curves_[index];
}

const HalfCurveMesh::Face &HalfCurveMesh::GetFace(int id) const {
  assert(id >= 0);
  assert(id < faces_.size());
  return faces_[id];
}

const HalfCurveMesh::Object &HalfCurveMesh::GetObject(int id) const {
  assert(id >= 0);
  assert(id < objects_.size());
  return objects_[id];
}

int HalfCurveMesh::AddVertex(int position_id) {
  assert(position_id >= 0);
  assert(position_id < vertex_positions_.size());
  vertices_.push_back(Vertex{position_id, -1});
  return vertices_.size() - 1;
}

int HalfCurveMesh::AddVertexPosition(const Vector3d &position) {
  vertex_positions_.push_back(position);
  return vertex_positions_.size() - 1;
}

int HalfCurveMesh::AddVertexNormal(const Vector3d &normal) {
  vertex_normals_.push_back(normal);
  return vertex_normals_.size() - 1;
}

int HalfCurveMesh::AddLinearCurve() {
  linear_curves_.push_back({{-1, -1, -1, -1, -1}});
  return MakeCurveId(linear_curves_.size() - 1, CurveType::Linear);
}

int HalfCurveMesh::AddFace(int object_id) {
  faces_.push_back({-1, object_id});
  return faces_.size() - 1;
}

int HalfCurveMesh::AddObject(std::string name) {
  objects_.push_back({std::move(name)});
  return objects_.size() - 1;
}

#ifndef NDEBUG
void HalfCurveMesh::Check() const {
  // Every time a mesh element with ID = X is referenced by some other element,
  // mark vector[X] = true in the corresponding vector. They should become all
  // true; there should be no unused elements.
  std::vector<bool> vertices_used(vertices_.size());
  std::vector<bool> vertex_positions_used(vertex_positions_.size());
  std::vector<bool> vertex_normals_used(vertex_normals_.size());
  std::vector<bool> faces_used(faces_.size());
  std::vector<bool> objects_used(objects_.size());

  // Every time a vertex with ID = X is found to refer to one of its outgoing
  // curves, mark vertex_curves_found[X] = true. It should become all true.
  std::vector<bool> vertex_curves_found(vertices_.size());

  size_t linear_curve_num = linear_curves_.size();
  for(int curve_index = 0; curve_index < linear_curve_num; curve_index++) {
    int curve_id = MakeCurveId(curve_index, CurveType::Linear);
    const LinearCurve &curve = GetLinearCurve(curve_id);
    const Face &face = GetFace(curve.face_id);
    const Vertex &vertex = GetVertex(curve.vertex_id);
    const Vector3d vertex_normal = GetVertexNormal(curve.normal_id);
    const Vector3d vertex_position = GetVertexPosition(vertex.position_id);
    GetObject(face.object_id);

    vertices_used[curve.vertex_id] = true;
    vertex_positions_used[vertex.position_id] = true;
    vertex_normals_used[curve.normal_id] = true;
    faces_used[curve.face_id] = true;
    objects_used[face.object_id] = true;

    assert(vertex_position.isfinite());
    assert(vertex_normal.isfinite());

    const HalfCurve &twin_curve = GetCurve(curve.twin_curve_id);
    assert(twin_curve.twin_curve_id == curve_id);

    // origin_vertex is the vertex this HalfCurve points away from
    int origin_vertex_id = twin_curve.vertex_id;
    const Vertex &origin_vertex = GetVertex(origin_vertex_id);
    if(origin_vertex.curve_id == curve_id)
      vertex_curves_found[origin_vertex_id] = true;

    // walk the next_curve_id list
    int curve_num = 0;
    int current_curve_id = curve_id;
    bool found_face_curve = false;
    do {
      const HalfCurve &current_curve = GetCurve(current_curve_id);
      assert(current_curve.face_id == curve.face_id);
      if(current_curve_id == face.curve_id)
        found_face_curve = true;
      current_curve_id = current_curve.next_curve_id;
      curve_num++;
    } while(current_curve_id != curve_id);
    assert(curve_num >= 3);
    assert(found_face_curve);
  }

  // TODO other curve types
  assert(circular_curves_.size() == 0);
  assert(elliptical_curves_.size() == 0);

  // no unused elements
  auto end = vertices_used.end();
  assert(end == std::find(vertices_used.begin(), end, false));
  end = vertex_positions_used.end();
  assert(end == std::find(vertex_positions_used.begin(), end, false));
  end = vertex_normals_used.end();
  assert(end == std::find(vertex_normals_used.begin(), end, false));
  end = faces_used.end();
  assert(end == std::find(faces_used.begin(), end, false));
  end = objects_used.end();
  assert(end == std::find(objects_used.begin(), end, false));

  // all vertices had a curve_id pointing to one of their outgoing curves
  end = vertex_curves_found.end();
  assert(end == std::find(vertex_curves_found.begin(), end, false));
}
#endif

WavFrObj HalfCurveMesh::MakeWavFrObj() const {
  // WavFrObj only supports polygons
  assert(circular_curves_.size() == 0);
  assert(elliptical_curves_.size() == 0);

  size_t num_vertex_positions = vertex_positions_.size();
  std::vector<Vector3f> wavfr_vertices;
  wavfr_vertices.reserve(num_vertex_positions);
  for(const Vector3d &pos: vertex_positions_)
    wavfr_vertices.push_back(
      Vector3f{float(pos.x), float(pos.y), float(pos.z)});

  size_t num_vertex_normals = vertex_normals_.size();
  std::vector<Vector3f> wavfr_normals;
  wavfr_normals.reserve(num_vertex_normals);
  for(const Vector3d &normal: vertex_normals_)
    wavfr_normals.push_back(
      Vector3f{float(normal.x), float(normal.y), float(normal.z)});

  size_t num_objects = objects_.size();
  std::vector<WavFrObj::ObjObject> wavfr_objects;
  wavfr_objects.reserve(num_objects);
  for(const Object &object: objects_)
    wavfr_objects.push_back(WavFrObj::ObjObject(object.name));

  for(const Face &face: faces_) {
    std::vector<WavFrObj::ObjVert> wavfr_face_verts;

    int first_curve_id = face.curve_id;
    int curve_id = first_curve_id;
    do {
      const HalfCurve &curve = GetCurve(curve_id);
      const Vertex &vertex = GetVertex(curve.vertex_id);
      wavfr_face_verts.push_back(
        WavFrObj::ObjVert{vertex.position_id, -1, curve.normal_id});
      curve_id = curve.next_curve_id;
    } while(curve_id != first_curve_id);

    wavfr_objects[face.object_id].addFace(
      std::move(wavfr_face_verts));
  }

  return WavFrObj(std::move(wavfr_vertices), std::vector<UvCoord>(),
    std::move(wavfr_normals), std::move(wavfr_objects));
}

void HalfCurveMesh::Bisect(Vector3d normal) {
  // TODO
}

namespace {

// RXDY = sqrt(X)/Y
constexpr double R2D2 = 0.7071067811865475244008443621048490392848; // beep boop
constexpr double R3D2 = 0.8660254037844386467637231707529361834714;
constexpr double R6D4 = 0.6123724356957945245493210186764728479915;
constexpr double R10D4 = 0.7905694150420948329997233861081796334299;

const double AlignedPlaneOffsets[] = {
  -1, -R3D2, -R2D2, -R6D4, -0.5, 0, 0.5, R6D4, R2D2, R3D2, 1 };

const double CylinderRadii[] = { R2D2, R10D4, R3D2, 1 };

} // namespace

/*
each cell's components are indexed like so:

    vertices:                        faces:     * - - - - - - *
                                               /             /
                                              /      5      /
       7 - - - - - 5                     *   /             /- *      *
      /|          /|                    /|  * - - - - - - *   |     /|
     / |         / |                   / |      |             |    / |
    6 - - - - - 4  |                  /  |      |      1      |   /  |
    |  |        |  |                 *   |  * - - - - - - *   |  *   |
    |  3 - - - -|- 1       Z         | 3 |  |             |   |  | 2 |
    | /         | /        | X       |   *  |             | - *  |   *
    |/          |/         |/        |  /   |      0      |      |  /
    2 - - - - - 0      Y - *         | /    |             |      | /
                                     |/     |             | - *  |/
                                     *      * - - - - - - *  /   *
                          14                  /      4      /
    half-edges:     * - - - - - - *          /             /
                   /             /          * - - - - - - *
               10 /             / 8         
                 /             /
                * - - - - - - *
                      12
                          15
        *           * - - - - - - *           *
    11 /|           |             |        9 /|
      / |           |             |         / |
     /  | 22     23 | 13          | 19     /  | 18
    *   |       * - - - - - - *   |       *   |
    |   |       |   |         |   |       |   |
    |   *       |   * - - - - | - *       |   *
 20 |  /     21 |          7  | 17     16 |  /
    | /         |             |           | /
    |/ 3        |             |           |/ 1
    *           * - - - - - - *           *
                       5
                           6
                    * - - - - - - *
                   /             /
                2 /             / 0
                 /             /
                * - - - - - - *
                       4
*/
HalfCurveMesh MakeAlignedCells() {
  HalfCurveMesh mesh;

  // create the 6 axis-aligned normal vectors
  int x_pos = mesh.AddVertexNormal( UnitX_Vector3d);
  int x_neg = mesh.AddVertexNormal(-UnitX_Vector3d);
  int y_pos = mesh.AddVertexNormal( UnitY_Vector3d);
  int y_neg = mesh.AddVertexNormal(-UnitY_Vector3d);
  int z_pos = mesh.AddVertexNormal( UnitZ_Vector3d);
  int z_neg = mesh.AddVertexNormal(-UnitZ_Vector3d);

  constexpr size_t size = std::size(AlignedPlaneOffsets);

  // a unique_ptr to a size x size x size array of vertex position IDs
  std::unique_ptr<int[][size][size]> position_ids =
    std::make_unique<int[][size][size]>(size);

  // populate vertex positions at every intersection of 3 axis-aligned planes
  for(size_t zi = 0; zi < size; zi++) {
    double z = AlignedPlaneOffsets[zi];
    for(size_t yi = 0; yi < size; yi++) {
      double y = AlignedPlaneOffsets[yi];
      for(size_t xi = 0; xi < size; xi++) {
        double x = AlignedPlaneOffsets[xi];
        int id = mesh.AddVertexPosition(Vector3d{x,y,z});
        position_ids.get()[zi][yi][xi] = id;
      }
    }
  }

  for(size_t zi = 0; zi < size-1; zi++) {
    for(size_t yi = 0; yi < size-1; yi++) {
      for(size_t xi = 0; xi < size-1; xi++) {
        std::string object_name = std::to_string(zi) + '-' +
          std::to_string(yi) + '-' + std::to_string(xi);
        int object_id = mesh.AddObject(object_name);

        int face_ids[6];
        for(int i = 0; i < 6; i++)
          face_ids[i] = mesh.AddFace(object_id);

        int vertex_ids[8];
        vertex_ids[0] = mesh.AddVertex(position_ids.get()[zi  ][yi  ][xi  ]);
        vertex_ids[1] = mesh.AddVertex(position_ids.get()[zi  ][yi  ][xi+1]);
        vertex_ids[2] = mesh.AddVertex(position_ids.get()[zi  ][yi+1][xi  ]);
        vertex_ids[3] = mesh.AddVertex(position_ids.get()[zi  ][yi+1][xi+1]);
        vertex_ids[4] = mesh.AddVertex(position_ids.get()[zi+1][yi  ][xi  ]);
        vertex_ids[5] = mesh.AddVertex(position_ids.get()[zi+1][yi  ][xi+1]);
        vertex_ids[6] = mesh.AddVertex(position_ids.get()[zi+1][yi+1][xi  ]);
        vertex_ids[7] = mesh.AddVertex(position_ids.get()[zi+1][yi+1][xi+1]);

        // create the 12 edges (2 half-edges each) of the cell
        int curve_ids[24];
        for(int i = 0; i < 24; i++)
          curve_ids[i] = mesh.AddLinearCurve();

        // a macro to help fill in the edge data
        #define set_linear_curve(this, twin, next, face, vert, norm) \
          mesh.GetCurve(curve_ids[this]) = HalfCurveMesh::LinearCurve{{ \
            curve_ids[twin], curve_ids[next], face_ids[face], \
            vertex_ids[vert], norm }};

        //               this twin next face vert norm
        set_linear_curve(   0,   1,   4,   4,   0, z_neg)
        set_linear_curve(   1,   0,  18,   2,   1, y_neg)
        set_linear_curve(   2,   3,   6,   4,   3, z_neg)
        set_linear_curve(   3,   2,  20,   3,   2, y_pos)
        set_linear_curve(   4,   5,   2,   4,   2, z_neg)
        set_linear_curve(   5,   4,  17,   0,   0, x_neg)
        set_linear_curve(   6,   7,   0,   4,   1, z_neg)
        set_linear_curve(   7,   6,  23,   1,   3, x_pos)
        set_linear_curve(   8,   9,  14,   5,   5, z_pos)
        set_linear_curve(   9,   8,  16,   2,   4, y_neg)
        set_linear_curve(  10,  11,  12,   5,   6, z_pos)
        set_linear_curve(  11,  10,  22,   3,   7, y_pos)
        set_linear_curve(  12,  13,   8,   5,   4, z_pos)
        set_linear_curve(  13,  12,  21,   0,   6, x_neg)
        set_linear_curve(  14,  15,  10,   5,   7, z_pos)
        set_linear_curve(  15,  14,  19,   1,   5, x_pos)
        set_linear_curve(  16,  17,   1,   2,   0, y_neg)
        set_linear_curve(  17,  16,  13,   0,   4, x_neg)
        set_linear_curve(  18,  19,   9,   2,   5, y_neg)
        set_linear_curve(  19,  18,   7,   1,   1, x_pos)
        set_linear_curve(  20,  21,  11,   3,   6, y_pos)
        set_linear_curve(  21,  20,   5,   0,   2, x_neg)
        set_linear_curve(  22,  23,   3,   3,   3, y_pos)
        set_linear_curve(  23,  22,  15,   1,   7, x_pos)

        #undef set_linear_curve

        mesh.GetFace(face_ids[0]).curve_id = curve_ids[5];
        mesh.GetFace(face_ids[1]).curve_id = curve_ids[7];
        mesh.GetFace(face_ids[2]).curve_id = curve_ids[1];
        mesh.GetFace(face_ids[3]).curve_id = curve_ids[3];
        mesh.GetFace(face_ids[4]).curve_id = curve_ids[2];
        mesh.GetFace(face_ids[5]).curve_id = curve_ids[8];

        mesh.GetVertex(vertex_ids[0]).curve_id = curve_ids[1];
        mesh.GetVertex(vertex_ids[1]).curve_id = curve_ids[0];
        mesh.GetVertex(vertex_ids[2]).curve_id = curve_ids[2];
        mesh.GetVertex(vertex_ids[3]).curve_id = curve_ids[3];
        mesh.GetVertex(vertex_ids[4]).curve_id = curve_ids[8];
        mesh.GetVertex(vertex_ids[5]).curve_id = curve_ids[9];
        mesh.GetVertex(vertex_ids[6]).curve_id = curve_ids[11];
        mesh.GetVertex(vertex_ids[7]).curve_id = curve_ids[10];
      }
    }
  }

  return mesh;
}

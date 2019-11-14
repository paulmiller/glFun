#ifndef HALF_CURVE_MESH_H
#define HALF_CURVE_MESH_H

#include "math/vector.h"
#include "mesh_obj.h"

#include <array>
#include <string>
#include <vector>

enum class Axis { X, Y, Z };
constexpr std::array<Axis, 3> IterableAxes = { Axis::X, Axis::Y, Axis::Z };

class Cylinder {
public:
  Axis perpendicular;
  double radius;
};

// a half-edge data structure which supports both polygons and edges/faces which
// lie on the surfaces of origin-centered unit spheres and origin-centered,
// axis-aligned cylinders
class HalfCurveMesh {
public:
  class Vertex {
  public:
    int position_id;
    int curve_id;
  };

  class HalfCurve {
  public:
    // The least-significant 2 bits are the CurveType. The other bits are
    // indices into the corresponding *_curves_ vectors.
    int twin_curve_id;
    int next_curve_id; 

    // indices into faces_, vertices_, normals_ vectors
    int face_id;
    int vertex_id;
    int normal_id;
  };

  class LinearCurve : public HalfCurve {
  };

  // on the surface of the sphere
  class CircularCurve : public HalfCurve {
  };

  // on the surface of a cylinder
  class EllipticalCurve : public HalfCurve {
  public:
    int cylinder_id;
  };

  enum class CurveType { Linear = 0b00, Circular = 0b01, Elliptical = 0b10 };

  class Face {
  public:
    int curve_id;
    int object_id;
  };

  class Object {
  public:
    std::string name;
  };

  const Vertex &GetVertex(int id) const;
  const Vector3d &GetVertexPosition(int id) const;
  const Vector3d &GetVertexNormal(int id) const;
  const HalfCurve &GetCurve(int id) const;
  const LinearCurve &GetLinearCurve(int id) const;
  const Face &GetFace(int id) const;
  const Object &GetObject(int id) const;

  Vertex &GetVertex(int id) {
    return const_cast<Vertex&>(
      const_cast<const HalfCurveMesh*>(this)->GetVertex(id)); }
  Vector3d &GetVertexPosition(int id) {
    return const_cast<Vector3d&>(
      const_cast<const HalfCurveMesh*>(this)->GetVertexPosition(id)); }
  Vector3d &GetVertexNormal(int id) {
    return const_cast<Vector3d&>(
      const_cast<const HalfCurveMesh*>(this)->GetVertexNormal(id)); }
  HalfCurve &GetCurve(int id) {
    return const_cast<HalfCurve&>(
      const_cast<const HalfCurveMesh*>(this)->GetCurve(id)); }
  LinearCurve &GetLinearCurve(int id) {
    return const_cast<LinearCurve&>(
      const_cast<const HalfCurveMesh*>(this)->GetLinearCurve(id)); }
  Face &GetFace(int id) {
    return const_cast<Face&>(
      const_cast<const HalfCurveMesh*>(this)->GetFace(id)); }
  Object &GetObject(int id) {
    return const_cast<Object&>(
      const_cast<const HalfCurveMesh*>(this)->GetObject(id)); }

  int AddVertex(int position_id);
  int AddVertexPosition(const Vector3d &position);
  int AddVertexNormal(const Vector3d &normal); // must be unit vector
  int AddLinearCurve();
  int AddFace(int object_id);
  int AddObject(std::string name);

  // assert data structure invariants
  #ifndef NDEBUG
  void Check() const;
  #endif

  WavFrObj MakeWavFrObj() const;

  // bisect all objects by the plane passing through the origin and
  // perpendicular to "normal"
  void Bisect(Vector3d normal);

private:
  int MakeCurveId(int edge_index, CurveType type) const {
    return edge_index << 2 | static_cast<int>(type);
  }

  std::vector<Vertex> vertices_;
  std::vector<Vector3d> vertex_positions_;
  std::vector<Vector3d> vertex_normals_;
  std::vector<LinearCurve> linear_curves_;
  std::vector<CircularCurve> circular_curves_;
  std::vector<EllipticalCurve> elliptical_curves_;
  std::vector<Cylinder> cylinders_;
  std::vector<Face> faces_;
  std::vector<Object> objects_;
};

HalfCurveMesh MakeAlignedCells();

#endif

#ifndef HALF_CURVE_MESH_H
#define HALF_CURVE_MESH_H

#include "math/vector.h"
#include "mesh_obj.h"

#include <array>
#include <cstdlib>
#include <string>
#include <type_traits>
#include <unordered_set>
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
  struct Vertex;
  struct HalfCurve;
  struct Face;
  struct Object;

  enum class CurveType { Linear = 0b00, Circular = 0b01, Elliptical = 0b10 };

  // TODO iterators for walking surrounding HalfCurves?
  struct Vertex {
    Vector3d *position;
    HalfCurve *curve;
  };

  struct HalfCurve {
    CurveType type;

    // indices into half_curves_ vector
    HalfCurve *twin_curve;
    HalfCurve *next_curve; 

    // indices into faces_, vertices_, normals_ vectors
    Face *face;
    Vertex *vertex;
    Vector3d *normal;
  };

  // TODO iterators for surrounding HalfCurves and Vertices?
  struct Face {
    HalfCurve *curve;
    Object *object;
  };

  struct Object {
    std::string name;
  };

  template<typename T>
  class ComponentIndex {
  public:
    static constexpr size_t Null = std::numeric_limits<size_t>::max();

    size_t value;

    ComponentIndex() : value(Null) {}
    ComponentIndex(const ComponentIndex &i) : value(i.value) {}
    explicit ComponentIndex(size_t s) : value(s) {}

    bool IsNull() const { return value == Null; }

    bool operator< (ComponentIndex<T> rhs) const { return value <  rhs.value; }
    bool operator> (ComponentIndex<T> rhs) const { return value >  rhs.value; }
    bool operator<=(ComponentIndex<T> rhs) const { return value <= rhs.value; }
    bool operator>=(ComponentIndex<T> rhs) const { return value >= rhs.value; }
    bool operator!=(ComponentIndex<T> rhs) const { return value != rhs.value; }
    bool operator==(ComponentIndex<T> rhs) const { return value == rhs.value; }

    bool operator< (size_t rhs) const { return value <  rhs; }
    bool operator> (size_t rhs) const { return value >  rhs; }
    bool operator<=(size_t rhs) const { return value <= rhs; }
    bool operator>=(size_t rhs) const { return value >= rhs; }
    bool operator!=(size_t rhs) const { return value != rhs; }
    bool operator==(size_t rhs) const { return value == rhs; }

    ComponentIndex<T> &operator++() { ++value; return *this; }
  };

  using VertexIndex = ComponentIndex<Vertex>;
  using HalfCurveIndex = ComponentIndex<HalfCurve>;
  using FaceIndex = ComponentIndex<Face>;
  using ObjectIndex = ComponentIndex<Object>;

  class VertexPositionIndex : public ComponentIndex<Vector3d> {
  public:
    VertexPositionIndex() {}
    explicit VertexPositionIndex(size_t s) : ComponentIndex<Vector3d>(s) {}
    VertexPositionIndex(ComponentIndex<Vector3d> i) :
      ComponentIndex<Vector3d>(i) {}
  };

  class VertexNormalIndex : public ComponentIndex<Vector3d> {
  public:
    VertexNormalIndex() {}
    explicit VertexNormalIndex(size_t s) : ComponentIndex<Vector3d>(s) {}
    VertexNormalIndex(ComponentIndex<Vector3d> i) :
      ComponentIndex<Vector3d>(i) {}
  };

  const Vertex    &operator[](VertexIndex         i) const { return get(i); }
  const Vector3d  &operator[](VertexPositionIndex i) const { return get(i); }
  const Vector3d  &operator[](VertexNormalIndex   i) const { return get(i); }
  const HalfCurve &operator[](HalfCurveIndex      i) const { return get(i); }
  const Face      &operator[](FaceIndex           i) const { return get(i); }
  const Object    &operator[](ObjectIndex         i) const { return get(i); }

  Vertex    &operator[](VertexIndex         i) { return get(i); }
  Vector3d  &operator[](VertexPositionIndex i) { return get(i); }
  Vector3d  &operator[](VertexNormalIndex   i) { return get(i); }
  HalfCurve &operator[](HalfCurveIndex      i) { return get(i); }
  Face      &operator[](FaceIndex           i) { return get(i); }
  Object    &operator[](ObjectIndex         i) { return get(i); }

  VertexIndex AddVertex();
  VertexPositionIndex AddVertexPosition(const Vector3d &position);
  VertexNormalIndex AddVertexNormal(const Vector3d &normal);
  HalfCurveIndex AddHalfCurve();
  FaceIndex AddFace();
  ObjectIndex AddObject(std::string name);

  // assert data structure invariants
  #ifndef NDEBUG
  void Check() const;
  #endif

  std::unordered_set<Face*> FindConnectedFaces(Face *start_face);

  WavFrObj MakeWavFrObj() const;

  Vector3d CenterOfBoundingBox(FaceIndex face_index) const;

  // Cut the curve (and its twin) at a point along its length specified by t,
  // 0 < t < 1. Return the index of the newly created Vertex. Don't compute new
  // normals.
  VertexIndex CutCurve(HalfCurveIndex curve_index, double t);

  // Connect 2 vertices across a face. The given vertices must be on the given
  // face. Return the Index of one of the newly created HalfCurves. Do nothing
  // and return the null Index if the vertices are already connected. Don't
  // compute new normals.
  HalfCurveIndex CutFace(
    FaceIndex face_idx, VertexIndex vertex_a_idx, VertexIndex vertex_b_idx);

  void LoopCut(std::unordered_set<HalfCurveIndex> curve_idxs);

  // Bisect all objects by the plane passing through the origin and
  // perpendicular to "normal". "normal" need not be a unit vector. Return the
  // IDs of all the HalfCurves lying on the plane after the bisect.
  std::unordered_set<HalfCurveIndex> Bisect(const Vector3d &normal);

private:
  #define hcm_define_index_getter(ComponentType, IndexType, List) \
    IndexType IndexOf(ComponentType *c) const {                   \
      return List.IndexOf(c);                                     \
    }

  #define hcm_define_component_getters(ComponentType, IndexType, List) \
    const ComponentType &get(IndexType i) const {                      \
      assert(i < List.size());                                         \
      return List[i];                                                  \
    }                                                                  \
    ComponentType &get(IndexType i) {                                  \
      assert(i < List.size());                                         \
      return List[i];                                                  \
    }                                                                  \
    hcm_define_index_getter(ComponentType, IndexType, List)

  hcm_define_component_getters(Vertex, VertexIndex, vertices_)
  hcm_define_component_getters(HalfCurve, HalfCurveIndex, half_curves_)
  hcm_define_component_getters(Face, FaceIndex, faces_)
  hcm_define_component_getters(Object, ObjectIndex, objects_)

  // don't define IndexOf for positions and normals because the shared Vector3d
  // argument makes the functions ambiguous
  #undef hcm_define_index_getter
  #define hcm_define_index_getter(ComponentType, IndexType, List)

  hcm_define_component_getters(Vector3d, VertexPositionIndex, vertex_positions_)
  hcm_define_component_getters(Vector3d, VertexNormalIndex, vertex_normals_)

  #undef hcm_define_component_getters
  #undef hcm_define_index_getter

  template<typename T>
  class ComponentList {
  public:
    ComponentList() {}

    ComponentList(ComponentList<T> &&src)
    : list_(src.list_), size_(src.size_), capacity_(src.capacity_) {
      src.list_ = nullptr;
      src.size_ = 0;
      src.capacity_ = 0;
    }

    ~ComponentList() {
      if(std::is_pod<T>::value)
        free(list_);
      else
        delete[] list_;
    }

    size_t size() const { return size_; }

    T &operator[](ComponentIndex<T> i) {
      assert(i < size_);
      return list_[i.value];
    }

    const T &operator[](ComponentIndex<T> i) const {
      assert(i < size_);
      return list_[i.value];
    }

    // iterator may be invalidated by Append
    T *begin() { return list_; }
    T *end() { return list_ + size_; }
    const T *begin() const { return list_; }
    const T *end() const { return list_ + size_; }

    // Return a pointer to the appended component, and an offset. If the offset
    // is non-zero, then the "list_" was moved by realloc, and all pointers into
    // "list_" need to be updated by that offset.
    std::tuple<ComponentIndex<T>, uintptr_t> Append(T e);

    ComponentIndex<T> IndexOf(T *ptr) const {
      assert(ptr >= list_);
      return ComponentIndex<T>(ptr - list_);
    }

    #ifndef NDEBUG
    void CheckPtr(T *ptr) const {
      assert(ptr >= list_);
      assert(ptr < list_ + size_);
    }
    #endif

  private:
    T *list_ = nullptr;
    size_t size_ = 0, capacity_ = 0;
  };

  ComponentList<Vertex> vertices_;
  ComponentList<Vector3d> vertex_positions_;
  ComponentList<Vector3d> vertex_normals_;
  ComponentList<HalfCurve> half_curves_;
  ComponentList<Face> faces_;
  ComponentList<Object> objects_;
};

namespace std {
  template<typename T>
  struct hash<HalfCurveMesh::ComponentIndex<T>> {
    size_t
    operator()(const HalfCurveMesh::ComponentIndex<T> &i) const noexcept {
      return i.value;
    }
  };
}

HalfCurveMesh MakeAlignedCells();

#endif

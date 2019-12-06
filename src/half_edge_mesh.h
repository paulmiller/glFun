#ifndef HALF_EDGE_MESH_H
#define HALF_EDGE_MESH_H

#include "math/vector.h"
#include "mesh_obj.h"

#include <array>
#include <cstdlib>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <vector>

class HalfEdgeMesh {
public:
  struct Vertex;
  struct HalfEdge;
  struct Face;
  struct Object;

  // how to interpolate normals along an edge
  enum class NormalType {
    // The normal is constant along the length of this edge. Eeach end of this
    // edgemust have the same normal.
    Constant = 0,
    // This edge approximates an arc on the surface of a unit sphere centered at
    // the origin. Both ends of this edge must be on the sphere.
    Spherical,
    // This edge approximates an ellipse on the surface of an axis-aligned
    // cylinder whose center passes through the origin. Each end of this edge
    // must be equidistant from the axis. Edges on the surface and parallel to
    // the cylinder should be Constant rather than Cylindrical.
    X_Cylindrical, Y_Cylindrical, Z_Cylindrical
  };

  // TODO iterators for walking surrounding HalfEdges?
  struct Vertex {
    Vector3d *position;
    HalfEdge *edge;
  };

  struct HalfEdge {
    // indices into half_edges_ vector
    HalfEdge *twin_edge;
    HalfEdge *next_edge; 

    // indices into faces_, vertices_, normals_ vectors
    Face *face;
    Vertex *vertex;
    Vector3d *normal; // normal at "vertex"

    NormalType type;
  };

  // TODO iterators for surrounding HalfEdges and Vertices?
  struct Face {
    HalfEdge *edge;
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
  using HalfEdgeIndex = ComponentIndex<HalfEdge>;
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
  const HalfEdge &operator[](HalfEdgeIndex      i) const { return get(i); }
  const Face      &operator[](FaceIndex           i) const { return get(i); }
  const Object    &operator[](ObjectIndex         i) const { return get(i); }

  Vertex    &operator[](VertexIndex         i) { return get(i); }
  Vector3d  &operator[](VertexPositionIndex i) { return get(i); }
  Vector3d  &operator[](VertexNormalIndex   i) { return get(i); }
  HalfEdge &operator[](HalfEdgeIndex      i) { return get(i); }
  Face      &operator[](FaceIndex           i) { return get(i); }
  Object    &operator[](ObjectIndex         i) { return get(i); }

  VertexIndex AddVertex();
  VertexPositionIndex AddVertexPosition(const Vector3d &position);
  VertexNormalIndex AddVertexNormal(const Vector3d &normal);
  HalfEdgeIndex AddHalfEdge();
  FaceIndex AddFace();
  ObjectIndex AddObject(std::string name);

  // assert data structure invariants
  #ifndef NDEBUG
  void CheckPtrs() const;
  void CheckAll() const;
  #endif

  std::unordered_set<Face*> FindConnectedFaces(Face *start_face);

  WavFrObj MakeWavFrObj() const;

  Vector3d CenterOfBoundingBox(FaceIndex face_index) const;

  // Cut the edge (and its twin) at a point along its length specified by t,
  // 0 < t < 1. Return the index of the newly created Vertex. Don't compute new
  // normals.
  VertexIndex CutEdge(HalfEdgeIndex edge_index, double t);

  // Connect 2 vertices across a face. The given vertices must be on the given
  // face. Return the Index of one of the newly created HalfEdges. Do nothing
  // and return the null Index if the vertices are already connected. Don't
  // compute new normals.
  HalfEdgeIndex CutFace(
    FaceIndex face_idx, VertexIndex vertex_a_idx, VertexIndex vertex_b_idx);

  void LoopCut(std::unordered_set<HalfEdgeIndex> edge_idxs);

  // Bisect all objects by the plane passing through the origin and
  // perpendicular to "normal". "normal" need not be a unit vector. Return the
  // IDs of all the HalfEdges lying on the plane after the bisect.
  std::unordered_set<HalfEdgeIndex> Bisect(const Vector3d &normal);

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
  hcm_define_component_getters(HalfEdge, HalfEdgeIndex, half_edges_)
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
  ComponentList<HalfEdge> half_edges_;
  ComponentList<Face> faces_;
  ComponentList<Object> objects_;
};

namespace std {
  template<typename T>
  struct hash<HalfEdgeMesh::ComponentIndex<T>> {
    size_t
    operator()(const HalfEdgeMesh::ComponentIndex<T> &i) const noexcept {
      return i.value;
    }
  };
}

HalfEdgeMesh MakeAlignedCells();

#endif

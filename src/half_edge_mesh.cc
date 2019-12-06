#include "half_edge_mesh.h"

#include "scoped_timer.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <memory>
#include <stack>
#include <unordered_map>

HalfEdgeMesh::VertexIndex HalfEdgeMesh::AddVertex() {
  auto [index, realloc_offset] = vertices_.Append({});
  if(realloc_offset) {
    for(HalfEdge &edge: half_edges_) {
      if(edge.vertex)
        edge.vertex = (Vertex*)(uintptr_t(edge.vertex) + realloc_offset);
    }
  }
  return index;
}

HalfEdgeMesh::VertexPositionIndex
HalfEdgeMesh::AddVertexPosition(const Vector3d &position) {
  auto [index, realloc_offset] = vertex_positions_.Append(position);
  if(realloc_offset) {
    for(Vertex &vertex: vertices_) {
      if(vertex.position)
        vertex.position =
          (Vector3d*)(uintptr_t(vertex.position) + realloc_offset);
    }
  }
  return index;
}

HalfEdgeMesh::VertexNormalIndex
HalfEdgeMesh::AddVertexNormal(const Vector3d &normal) {
  auto [index, realloc_offset] = vertex_normals_.Append(normal);
  if(realloc_offset) {
    for(HalfEdge &edge: half_edges_) {
      if(edge.normal)
        edge.normal = (Vector3d*)(uintptr_t(edge.normal) + realloc_offset);
    }
  }
  return index;
}

HalfEdgeMesh::HalfEdgeIndex HalfEdgeMesh::AddHalfEdge() {
  auto [index, realloc_offset] = half_edges_.Append({});
  if(realloc_offset) {
    for(Vertex &vertex: vertices_) {
      if(vertex.edge)
        vertex.edge = (HalfEdge*)(uintptr_t(vertex.edge) + realloc_offset);
    }
    for(HalfEdge &other_edge: half_edges_) {
      if(other_edge.twin_edge)
        other_edge.twin_edge =
          (HalfEdge*)(uintptr_t(other_edge.twin_edge) + realloc_offset);
      if(other_edge.next_edge)
        other_edge.next_edge =
          (HalfEdge*)(uintptr_t(other_edge.next_edge) + realloc_offset);
    }
    for(Face &face: faces_) {
      if(face.edge)
        face.edge = (HalfEdge*)(uintptr_t(face.edge) + realloc_offset);
    }
  }
  return index;
}

HalfEdgeMesh::FaceIndex HalfEdgeMesh::AddFace() {
  auto [index, realloc_offset] = faces_.Append({});
  if(realloc_offset) {
    for(HalfEdge &edge: half_edges_) {
      if(edge.face)
        edge.face = (Face*)(uintptr_t(edge.face) + realloc_offset);
    }
  }
  return index;
}

HalfEdgeMesh::ObjectIndex HalfEdgeMesh::AddObject(std::string name) {
  auto [index, realloc_offset] = objects_.Append({std::move(name)});
  if(realloc_offset) {
    for(Face &face: faces_) {
      if(face.object)
        face.object = (Object*)(uintptr_t(face.object) + realloc_offset);
    }
  }
  return index;
}

#ifndef NDEBUG
void HalfEdgeMesh::CheckPtrs() const {
  for(const Vertex &vertex: vertices_) {
    vertex_positions_.CheckPtr(vertex.position);
    half_edges_.CheckPtr(vertex.edge);
  }
  for(const HalfEdge &edge: half_edges_) {
    vertices_.CheckPtr(edge.vertex);
    vertex_normals_.CheckPtr(edge.normal);
    half_edges_.CheckPtr(edge.twin_edge);
    half_edges_.CheckPtr(edge.next_edge);
    faces_.CheckPtr(edge.face);
  }
  for(const Face &face: faces_) {
    half_edges_.CheckPtr(face.edge);
    objects_.CheckPtr(face.object);
  }
}

void HalfEdgeMesh::CheckAll() const {
  PrintingScopedTimer timer("HalfEdgeMesh::CheckAll");

  CheckPtrs();

  // Every time a mesh component with index = X is referenced by some other
  // component, mark vector[X] = true in the corresponding vector. They should
  // become all true; there should be no unused elements.
  std::vector<bool> vertices_used(vertices_.size());
  std::vector<bool> vertex_positions_used(vertex_positions_.size());
  std::vector<bool> vertex_normals_used(vertex_normals_.size());
  std::vector<bool> faces_used(faces_.size());
  std::vector<bool> objects_used(objects_.size());

  for(const HalfEdge &edge: half_edges_) {
    {
      auto &vp = vertex_positions_;
      auto &vn = vertex_normals_;
      vertices_used         [    IndexOf(edge.vertex          ).value ] = true;
      faces_used            [    IndexOf(edge.face            ).value ] = true;
      objects_used          [    IndexOf(edge.face->object    ).value ] = true;
      vertex_positions_used [ vp.IndexOf(edge.vertex->position).value ] = true;
      vertex_normals_used   [ vn.IndexOf(edge.normal          ).value ] = true;
    }

    assert(edge.vertex->position->isfinite());
    assert(edge.normal->isfinite());

    assert(&edge != edge.twin_edge);
    assert(&edge == edge.twin_edge->twin_edge);
    assert(edge.next_edge != edge.twin_edge);
    assert(edge.twin_edge->next_edge != &edge);
    assert(edge.face != edge.twin_edge->face);
    assert(edge.vertex != edge.twin_edge->vertex);
    assert(edge.face->object == edge.twin_edge->face->object);

    // square of the minimum allowable distance between Vertices
    // (v.len2() < 0.0001) == (v.len() < 0.01)
    constexpr double min2 = 0.0001;

    const Vector3d *start = edge.twin_edge->vertex->position;
    const Vector3d *end = edge.vertex->position;
    // TODO threshold?
    assert((*end - *start).len2() >= min2);

    // compare to every other edge on the same object and ensure they're
    // different (slow)
    for(const HalfEdge &other_edge: half_edges_) {
      if(&other_edge == &edge) continue;
      if(&other_edge == edge.twin_edge) continue;
      if(other_edge.face->object != edge.face->object) continue;

      const Vector3d *other_start = other_edge.twin_edge->vertex->position;
      const Vector3d *other_end = other_edge.vertex->position;

      // TODO threshold?
      assert((*start - *other_start).len2() >= min2 ||
        (*end - *other_end).len2() >= min2);
      assert((*start - *other_end).len2() >= min2 ||
        (*end - *other_start).len2() >= min2);
    }

    // walk the HalfEdges surrounding edge.face
    int edge_num = 0;
    bool found_face_edge = false;
    const HalfEdge *previous_edge = nullptr;
    const HalfEdge *current_edge = &edge;
    do {
      assert(current_edge->face == edge.face);
      if(current_edge == edge.face->edge)
        found_face_edge = true;
      previous_edge = current_edge;
      current_edge = current_edge->next_edge;
      edge_num++;
    } while(current_edge != &edge);
    assert(edge_num >= 3);
    assert(found_face_edge);
    assert(previous_edge != nullptr);

    // check normals
    switch(edge.type) {
    case NormalType::Constant:
      assert( *(edge.normal) == *(previous_edge->normal) );
      break;
    case NormalType::Spherical:
      assert(start->len2() == end->len2());
      break;
    case NormalType::X_Cylindrical:
      assert(start->y * start->y + start->z * start->z ==
        end->y * end->y + end->z * end->z);
      break;
    case NormalType::Y_Cylindrical:
      assert(start->z * start->z + start->z * start->z ==
        end->z * end->z + end->z * end->z);
      break;
    case NormalType::Z_Cylindrical:
      assert(start->x * start->x + start->y * start->y ==
        end->x * end->x + end->y * end->y);
      break;
    default:
      assert(0);
    }

    // walk the HalfEdges surrounding edge.vertex
    bool found_this_edge = false;
    const HalfEdge *first_outgoing_edge = edge.vertex->edge;
    const HalfEdge *outgoing_edge = first_outgoing_edge;
    do {
      const HalfEdge *incoming_edge = outgoing_edge->twin_edge;
      assert(incoming_edge->vertex == edge.vertex);
      if(incoming_edge == &edge)
        found_this_edge = true;
      outgoing_edge = incoming_edge->next_edge;
    } while(outgoing_edge != first_outgoing_edge);
    assert(found_this_edge);
  }

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
}
#endif // #ifndef NDEBUG

std::unordered_set<HalfEdgeMesh::Face*>
HalfEdgeMesh::FindConnectedFaces(Face *start_face) {
  std::unordered_set<Face*> visited;
  std::stack<Face*> stack;
  stack.push(start_face);
  while(!stack.empty()) {
    Face *current_face = stack.top();
    stack.pop();
    visited.insert(current_face);

    HalfEdge *start_edge = current_face->edge;
    HalfEdge *current_edge = start_edge;
    do {
      Face *next_face = current_edge->twin_edge->face;
      if(!visited.count(next_face))
        stack.push(next_face);
      current_edge = current_edge->next_edge;
    } while(current_edge != start_edge);
  }
  return visited;
}

WavFrObj HalfEdgeMesh::MakeWavFrObj() const {
  PrintingScopedTimer timer("HalfEdgeMesh::MakeWavFrObj");

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

    const HalfEdge *first_edge = face.edge;
    const HalfEdge *edge = first_edge;
    do {
      size_t position_index =
        vertex_positions_.IndexOf(edge->vertex->position).value;
      size_t normal_index = vertex_normals_.IndexOf(edge->normal).value;
      wavfr_face_verts.push_back(
        WavFrObj::ObjVert{int(position_index), -1, int(normal_index)});
      edge = edge->next_edge;
    } while(edge != first_edge);

    size_t object_index = IndexOf(face.object).value;
    wavfr_objects[object_index].addFace(
      std::move(wavfr_face_verts));
  }

  return WavFrObj(std::move(wavfr_vertices), std::vector<UvCoord>(),
    std::move(wavfr_normals), std::move(wavfr_objects));
}

Vector3d HalfEdgeMesh::CenterOfBoundingBox(FaceIndex face_index) const {
  constexpr double inf = std::numeric_limits<double>::infinity();
  double x_min = inf, x_max = -inf,
         y_min = inf, y_max = -inf,
         z_min = inf, z_max = -inf;

  HalfEdge *first_edge = get(face_index).edge;
  HalfEdge *edge = first_edge;
  do {
    Vector3d position = *(edge->vertex->position);
    x_min = std::min(x_min, position.x); x_max = std::max(x_max, position.x);
    y_min = std::min(y_min, position.y); y_max = std::max(y_max, position.y);
    z_min = std::min(z_min, position.z); z_max = std::max(z_max, position.z);
    edge = edge->next_edge;
  } while(edge != first_edge);

  assert(std::isfinite(x_min)); assert(std::isfinite(x_max));
  assert(std::isfinite(y_min)); assert(std::isfinite(y_max));
  assert(std::isfinite(z_min)); assert(std::isfinite(z_max));

  return Vector3d{ (x_min+x_max)/2, (y_min+y_max)/2, (z_min+z_max)/2 };
}

HalfEdgeMesh::VertexIndex
HalfEdgeMesh::CutEdge(HalfEdgeIndex edge_index, double t) {
  VertexIndex new_vertex_index = AddVertex();
  HalfEdgeIndex new_edge_a_index = AddHalfEdge(); 
  HalfEdgeIndex new_edge_b_index = AddHalfEdge();

  // reallocs may invalidate pointers, so add all components before getting
  // pointers

  HalfEdge *edge = &get(edge_index);
  HalfEdge *twin_edge = edge->twin_edge;
  HalfEdge *new_edge_a = &get(new_edge_a_index);
  HalfEdge *new_edge_b = &get(new_edge_b_index);

  Vertex *start = edge->twin_edge->vertex;
  Vertex *end = edge->vertex;
  Vertex *new_vertex = &get(new_vertex_index);

  // the edges now look like this:
  //          _ _ _ _ _ _
  //        🡕    edge    🡖
  // start *               * end
  //        🡔 _ _ _ _ _ _ 🡗
  //        edge.twin_edge

  // TODO deduplicate positions
  Vector3d start_position = *(start->position);
  Vector3d end_position = *(end->position);
  Vector3d new_vertex_position =
    start_position + t * (end_position - start_position);
  new_vertex->position = &get(AddVertexPosition(new_vertex_position));
  edge->vertex = new_vertex;
  edge->twin_edge->vertex = new_vertex;

  // the edges now look like this:
  //    _ _ _
  //  🡕 edge 🡖
  // *         * new     *
  //            🡔 _ _ _ 🡗
  //            twin_edge


  new_edge_a->twin_edge = edge->twin_edge;
  new_edge_a->next_edge = edge->next_edge;
  new_edge_a->face = edge->face;
  new_edge_a->vertex = end;
  new_edge_a->normal = edge->normal;

  new_edge_b->twin_edge = edge;
  new_edge_b->next_edge = twin_edge->next_edge;
  new_edge_b->face = twin_edge->face;
  new_edge_b->vertex = start;
  new_edge_b->normal = twin_edge->normal;

  edge->twin_edge->twin_edge = new_edge_a;
  edge->twin_edge->next_edge = new_edge_b;

  edge->twin_edge = new_edge_b;
  edge->next_edge = new_edge_a;

  new_vertex->edge = new_edge_a;

  // the edges now look like this:
  //    _ _ _     _ _ _
  //  🡕 edge 🡖 🡕 new a 🡖
  // *         *         *
  //  🡔 _ _ _ 🡗 🡔 _ _ _ 🡗
  //    new b   twin_edge

  /*
  #ifndef NDEBUG
  CheckAll();
  #endif
  */

  return new_vertex_index;
}

HalfEdgeMesh::HalfEdgeIndex HalfEdgeMesh::CutFace(
  FaceIndex face_idx, VertexIndex vertex_a_idx, VertexIndex vertex_b_idx
) {
  assert(vertex_a_idx != vertex_b_idx);

  Vertex *vertex_a = &get(vertex_a_idx);
  Vertex *vertex_b = &get(vertex_b_idx);

  HalfEdgeIndex edge_a_in_index, edge_a_out_index;
  HalfEdgeIndex edge_b_in_index, edge_b_out_index;
  {
    // reallocs may invalidate these pointers, so confine them to this block
    Face *face = &get(face_idx);
    HalfEdge *edge_a_in = nullptr, *edge_a_out = nullptr;
    HalfEdge *edge_b_in = nullptr, *edge_b_out = nullptr;
    HalfEdge *first_edge = face->edge;
    HalfEdge *current_edge = first_edge;
    do {
      if(current_edge->vertex == vertex_a) {
        edge_a_in = current_edge;
        edge_a_out = current_edge->next_edge;
      }
      if(current_edge->vertex == vertex_b) {
        edge_b_in = current_edge;
        edge_b_out = current_edge->next_edge;
      }
      current_edge = current_edge->next_edge;
    } while(current_edge != first_edge);
    assert(edge_a_in); assert(edge_a_out);
    assert(edge_b_in); assert(edge_b_out);

    if(edge_a_in == edge_b_out || edge_a_out == edge_b_in)
      return HalfEdgeIndex();

    edge_a_in_index = IndexOf(edge_a_in);
    edge_a_out_index = IndexOf(edge_a_out);
    edge_b_in_index = IndexOf(edge_b_in);
    edge_b_out_index = IndexOf(edge_b_out);
  }

  // invalidates Face, HalfEdge pointers
  FaceIndex new_face_idx = AddFace();
  // TODO support other edge types
  HalfEdgeIndex new_edge_idx = AddHalfEdge();
  HalfEdgeIndex new_edge_twin_idx = AddHalfEdge();

  HalfEdge *new_edge = &get(new_edge_idx);
  HalfEdge *new_edge_twin = &get(new_edge_twin_idx);
  Face *face = &get(face_idx);
  Face *new_face = &get(new_face_idx);

  HalfEdge *edge_a_in = &get(edge_a_in_index);
  HalfEdge *edge_a_out = &get(edge_a_out_index);
  HalfEdge *edge_b_in = &get(edge_b_in_index);
  HalfEdge *edge_b_out = &get(edge_b_out_index);

  // the face now looks like this:
  //
  //              /             \
  //  edge_a_in /               \ edge_b_out
  //            /                 \
  //  vertex_a *       face        * vertex_b
  //            \                 /
  // edge_a_out \               / edge_b_in
  //              \             /

  new_face->object = face->object;

  face->edge = new_edge;
  new_face->edge = new_edge_twin;

  new_edge->twin_edge = new_edge_twin;
  new_edge->next_edge = edge_b_out;
  new_edge->face = face;
  new_edge->vertex = vertex_b;
  new_edge->normal = edge_b_in->normal;

  new_edge_twin->twin_edge = new_edge;
  new_edge_twin->next_edge = edge_a_out;
  new_edge_twin->face = new_face;
  new_edge_twin->vertex = vertex_a;
  new_edge_twin->normal = edge_a_in->normal;

  edge_a_in->next_edge = new_edge;
  edge_b_in->next_edge = new_edge_twin;

  HalfEdge *current_edge = edge_a_out;
  do {
    current_edge->face = new_face;
    current_edge = current_edge->next_edge;
  } while(current_edge != new_edge_twin);

  // the faces now look like this:
  //
  //              /    face     \
  //  edge_a_in /               \ edge_b_out
  //            /    new_edge    \
  //  vertex_a * - - - - - - - - - * vertex_b
  //            \ new_edge_twin  /
  // edge_a_out \               / edge_b_in
  //              \  new_face   /

  /*
  #ifndef NDEBUG
  CheckAll();
  #endif
  */

  return new_edge_idx;
}

void HalfEdgeMesh::LoopCut(std::unordered_set<HalfEdgeIndex> edge_idxs) {
  PrintingScopedTimer timer("HalfEdgeMesh::LoopCut");

  #ifndef NDEBUG
  // "edge_idxs" must contain only matched pairs of HalfEdges
  for(HalfEdgeIndex edge_idx: edge_idxs) {
    HalfEdgeIndex twin_idx = IndexOf(get(edge_idx).twin_edge);
    assert(edge_idxs.count(twin_idx));
  }

  // ensure every Vertex has 0 or 2 incoming and outgoing edges in "edge_idxs"
  // i.e. there is at most 1 cutting path through each Vertex
  for(Vertex &vertex: vertices_) {
    int outgoing_cut_edges = 0, incoming_cut_edges = 0;

    HalfEdge *first_outgoing_edge = vertex.edge;
    HalfEdge *outgoing_edge = first_outgoing_edge;
    do {
      HalfEdge *incoming_edge = outgoing_edge->twin_edge;

      if(edge_idxs.count(IndexOf(outgoing_edge)))
        outgoing_cut_edges++;
      if(edge_idxs.count(IndexOf(incoming_edge)))
        incoming_cut_edges++;

      outgoing_edge = incoming_edge->next_edge;
    } while(outgoing_edge != first_outgoing_edge);

    assert(outgoing_cut_edges == incoming_cut_edges);
    assert(outgoing_cut_edges == 0 || outgoing_cut_edges == 2);
  }
  #endif

  // a map from each Index in "edge_idxs" to the Index of the next HalfEdge in
  // the loop
  std::unordered_map<HalfEdgeIndex, HalfEdgeIndex> next_loop_edge_idxs;

  // populate "next_loop_edge_idxs"
  for(HalfEdgeIndex edge_idx: edge_idxs) {
    assert(!next_loop_edge_idxs.count(edge_idx));
    HalfEdge *edge = &get(edge_idx);
    // find the HalfEdge following "edge" in the loop: this is the HalfEdge
    // exiting the Vertex "edge->vertex", which is not the twin of "edge", and
    // is in the set of loop edges "edge_idxs"
    HalfEdge *first_outgoing_edge = edge->vertex->edge;
    HalfEdge *outgoing_edge = first_outgoing_edge;
    while(outgoing_edge == edge->twin_edge ||
        !edge_idxs.count(IndexOf(outgoing_edge))) {
      outgoing_edge = outgoing_edge->twin_edge->next_edge;
      // the loop should terminate before getting back to
      // "first_outgoing_edge_id"
      if(outgoing_edge == first_outgoing_edge) {
        std::cout << "HalfEdgeMesh::LoopCut failed: couldn't follow loop "
          "through vertex at " << *(edge->vertex->position) << '\n';
        return;
      }
    }
    next_loop_edge_idxs[edge_idx] = IndexOf(outgoing_edge);
  }

  // when splitting a Vertex, one side gets the original Vertex and marks it as
  // "claimed" here, and subsequent visits must create new Vertices
  std::unordered_set<VertexIndex> claimed_vertex_indices;

  // Splitting an Object is the same, except it's possible for a single Object
  // to be cut by multiple, unconnected loops. We don't know how many new
  // Objects are needed until all loops are cut. So when making a cut, add the
  // new Face to "new_face_indices". These Faces, and all the Faces connected to
  // them, will get Objects assigned at the end.
  std::unordered_set<FaceIndex> new_face_indices;

  // make the cuts
  while(!edge_idxs.empty()) {
    // take an arbitrary HalfEdge and cut its associated loop
    HalfEdgeIndex first_edge_idx = *(edge_idxs.begin());

    FaceIndex new_face_index = AddFace();
    new_face_indices.insert(new_face_index);
    Face *new_face = &get(new_face_index); // invalidates Face pointers

    // may be reassigned to a new Object later
    new_face->object = get(first_edge_idx).face->object;

    // remember the 1st 3 vertices along the loop to get a normal vector later
    Vector3d sample_vertex_positions[3];
    int sample_vertices = 0;

    VertexIndex saved_split_vertex_index;
    HalfEdgeIndex prev_edge_idx;
    HalfEdgeIndex edge_idx = first_edge_idx;
    do {
      // TODO pick these to avoid NaN normals
      if(sample_vertices < 3) {
        Vector3d *position = get(edge_idx).vertex->position;
        sample_vertex_positions[sample_vertices] = *position;
        sample_vertices++;
      }

      // invalidates HalfEdge pointers
      HalfEdge *new_twin_edge = &get(AddHalfEdge()); 
      HalfEdge *edge = &get(edge_idx);
      HalfEdge *old_twin_edge = edge->twin_edge;

      new_twin_edge->twin_edge = edge;
      new_twin_edge->face = new_face;

      VertexIndex old_start_vertex_index = IndexOf(old_twin_edge->vertex);
      if(claimed_vertex_indices.count(old_start_vertex_index)) {
        // invalidates Vertex pointers
        VertexIndex new_start_vertex_index = AddVertex();
        Vertex *new_start_vertex = &get(new_start_vertex_index);
        new_start_vertex->position = get(old_start_vertex_index).position;
        new_start_vertex->edge = edge;
        new_twin_edge->vertex = new_start_vertex;

        // Update all the HalfEdges on this side of the cut, that used to point
        // to the claimed Vertex, to point to the new Vertex, starting with the
        // "previous" HalfEdge. Or if we can't, because this is the first
        // iteration, and "prev_edge_idx" is null, then save the new Vertex so
        // we can update it later.
        if(prev_edge_idx.IsNull()) {
          saved_split_vertex_index = new_start_vertex_index;
        } else {
          HalfEdge *first_incoming_edge = &get(prev_edge_idx);
          HalfEdge *incoming_edge = first_incoming_edge;
          for(;;) {
            assert(incoming_edge->vertex == old_twin_edge->vertex);
            incoming_edge->vertex = new_start_vertex;
            if(edge_idxs.count(IndexOf(incoming_edge->next_edge)))
              break;
            incoming_edge = incoming_edge->next_edge->twin_edge;
            // we should end the fan before going all the way around the Vertex
            assert(incoming_edge != first_incoming_edge);
          }
        }
      } else {
        claimed_vertex_indices.insert(old_start_vertex_index);
        old_twin_edge->vertex->edge = edge;
        new_twin_edge->vertex = old_twin_edge->vertex;
      }

      // "edge" points at "new_twin_edge", but "old_twin_edge" may still
      // point at "edge". This will be fixed when "old_twin_edge"'s loop comes
      // up for cutting.
      edge->twin_edge = new_twin_edge;
      new_face->edge = new_twin_edge;

      edge_idxs.erase(edge_idx);
      prev_edge_idx = edge_idx;
      edge_idx = next_loop_edge_idxs[edge_idx];
    } while(edge_idx != first_edge_idx);
    assert(new_face->edge != nullptr);

    if(!saved_split_vertex_index.IsNull()) {
      // Update the HalfEdges on this side of the cut for the saved Vertex. The
      // difference is that we can't check "edge_idxs" to see when we've
      // reached the end of the fan, because all the HalfEdges on this side of
      // the loop cut have been removed from "edge_idxs". But since the
      // "previous" HalfEdge is now the one right behind "loop_start_edge" in
      // the loop, we can use "loop_start_edge" to mark the end of the fan.
      assert(!prev_edge_idx.IsNull());
      Vertex *new_start_vertex = &get(saved_split_vertex_index);
      HalfEdge *first_incoming_edge = &get(prev_edge_idx);
      HalfEdge *incoming_edge = first_incoming_edge;
      HalfEdge *loop_start_edge = &get(edge_idx);
      for(;;) {
        assert(incoming_edge->vertex != new_start_vertex);
        incoming_edge->vertex = new_start_vertex;
        if(incoming_edge->next_edge == loop_start_edge)
          break;
        incoming_edge = incoming_edge->next_edge->twin_edge;
        // we should end the fan before going all the way around the Vertex
        assert(incoming_edge != first_incoming_edge);
      }
    }

    assert(sample_vertices == 3);
    Vector3d &a = sample_vertex_positions[0];
    Vector3d &b = sample_vertex_positions[1];
    Vector3d &c = sample_vertex_positions[2];
    Vector3d face_normal = cross(c - a, b - a).unit();
    assert(face_normal.isfinite());
    // invalidates previous normal pointers
    Vector3d *new_normal = &get(AddVertexNormal(face_normal));

    edge_idx = first_edge_idx;
    do {
      HalfEdge *edge = &get(edge_idx);
      HalfEdge *new_twin_edge = edge->twin_edge;

      new_twin_edge->next_edge = get(prev_edge_idx).twin_edge;
      new_twin_edge->normal = new_normal;

      prev_edge_idx = edge_idx;
      edge_idx = next_loop_edge_idxs[edge_idx];
    } while(edge_idx != first_edge_idx);
  }

  std::unordered_set<ObjectIndex> claimed_object_indices;
  while(!new_face_indices.empty()) {
    FaceIndex new_face_index = *(new_face_indices.begin());
    new_face_indices.erase(new_face_index);
    Face *new_face = &get(new_face_index);

    ObjectIndex old_object_index = IndexOf(new_face->object);
    Object *new_object = nullptr;
    if(claimed_object_indices.count(old_object_index)) {
      std::string name = get(old_object_index).name + "-cut";
      // invalidates Object pointers
      new_object = &get(AddObject(std::move(name)));
    } else {
      claimed_object_indices.insert(old_object_index);
    }

    std::unordered_set<Face*> faces = FindConnectedFaces(new_face);
    for(Face *face: faces) {
      new_face_indices.erase(IndexOf(face));
      if(new_object)
        face->object = new_object;
    }
  }

  /*
  #ifndef NDEBUG
  CheckAll();
  #endif
  */
}

std::unordered_set<HalfEdgeMesh::HalfEdgeIndex>
HalfEdgeMesh::Bisect(const Vector3d &normal) {
  PrintingScopedTimer timer("HalfEdgeMesh::Bisect");

  // Objects which should be ignored, because they don't pass through the
  // bisecting plane (though they may have components inside the plane)
  std::unordered_set<ObjectIndex> ignored_objects;

  // populate "ignored_objects"
  // TODO edged HalfEdges may pass through plane despite all Vertices being on
  // one side
  {
    // an Object's location relative to the bisecting plane
    enum Location : char {
      Unknown = 0,
      InFront, // all Vertices are on or in front of the plane
      Behind,  // all Vertices are on or behind the plane
      Through  // Object has Vertices both in front and behind
    };

    // Find each Object's location by checking all its Vertices. Objects
    // contained entirely inside the plane will remain "Unknown".
    size_t objects_size = objects_.size();
    std::vector<Location> object_locations(objects_size);
    for(const Vertex &vertex: vertices_) {
      size_t object_index = IndexOf(vertex.edge->face->object).value;
      Location object_location = object_locations[object_index];

      // if we've already found this Object's Vertices on both sides, don't
      // check the remaining Vertices
      if(object_location == Through) continue;

      double d = dot(normal, *(vertex.position));
      if(d == 0 || std::isnan(d)) continue;
      Location vertex_location = (d < 0 ? Behind : InFront);

      if(object_location != vertex_location) {
        if(object_location == Unknown) {
          // this Vertex is on the same side as the previous Vertices
          object_locations[object_index] = vertex_location;
        } else {
          // this Vertex is on a different side as the previous Vertices
          object_locations[object_index] = Through;
        }
      }
    }

    // ignore Objects which don't pass through the plane
    for(size_t i = 0; i < objects_size; i++) {
      if(object_locations[i] != Through)
        ignored_objects.insert(ObjectIndex(i));
    }
  }

  // all vertices lying on the plane: both new vertices created to bisect
  // edges, and existing vertices that happened to be on the plane already
  std::unordered_set<VertexIndex> planar_vertex_indices;

  // all edges (and their twins) lying on the plane: new edges bisecting
  // faces, and existing edges
  std::unordered_set<HalfEdgeIndex> planar_edge_indices;

  // a set of HalfEdge IDs to skip, because we already checked their twin
  std::unordered_set<HalfEdge*> checked_twin_edges;

  // TODO support other edge types
  size_t edge_num = half_edges_.size();
  for(HalfEdgeIndex edge_index(0); edge_index < edge_num; ++edge_index) {
    HalfEdge *edge = &get(edge_index);
    if(ignored_objects.count(IndexOf(edge->face->object))) continue;
    if(checked_twin_edges.count(edge)) continue;
    checked_twin_edges.insert(edge->twin_edge);

    // line equation: S + t⋅D
    Vector3d S = *(edge->twin_edge->vertex->position);
    Vector3d D = *(edge->vertex->position) - S;

    // plane equation: 0 = a⋅x + b⋅y + c⋅z 
    // (where abc are the xyz componets of normal)
    //
    // solve for t:
    // 0 = a⋅(Sx + t⋅Dx) + b⋅(Sy + t⋅Dy) + c⋅(Sz + t⋅Dz)
    // 0 = a⋅Sx + a⋅t⋅Dx + b⋅Sy + b⋅t⋅Dy + c⋅Sz + c⋅t⋅Dz
    // 0 = t⋅(a⋅Dx + b⋅Dy + c⋅Dz) + a⋅Sx + b⋅Sy + c⋅Sz
    //        a⋅Sx + b⋅Sy + c⋅Sz       dot(normal, S)
    // t = - -------------------- = - ----------------
    //        a⋅Dx + b⋅Dy + c⋅Dz       dot(normal, D)

    // if the line parallel to the plane, then "normal" and D are at right
    // angles, and dot_D == 0
    double dot_D = dot(normal, D);
    // if the dot_D == 0 && dot_S == 0, then the line is inside the plane
    double dot_S = dot(normal, S);
    if(dot_D == 0) {
      if(dot_S == 0) {
        planar_edge_indices.insert(edge_index);
        planar_edge_indices.insert(IndexOf(edge->twin_edge));
      }
    } else {
      double t = - dot_S / dot_D;

      // TODO threshold?
      constexpr double epsilon = 0.0001;

      // does the intersection lie within the line segment?
      if(epsilon < t && t < 1-epsilon) {
        // Invalidates HalfEdge pointers. We got "edge_num" before the for
        // loop, so we won't iterate over any new HalfEdges appended by
        // CutEdge.
        planar_vertex_indices.insert(CutEdge(edge_index, t));
      }

      // does the intersection lie at one end of the segment?
      if(-epsilon < t && t < epsilon) {
        VertexIndex start_vertex = IndexOf(get(edge_index).twin_edge->vertex);
        planar_vertex_indices.insert(start_vertex);
      } else if(1-epsilon < t && t < 1+epsilon) {
        VertexIndex end_vertex = IndexOf(get(edge_index).vertex);
        planar_vertex_indices.insert(end_vertex);
      }
    }
  }

  #ifndef NDEBUG
  CheckAll();
  #endif

  std::vector<VertexIndex> planar_vertex_indices_on_this_face;
  size_t face_num = faces_.size();
  for(FaceIndex face_index(0); face_index < face_num; ++face_index) {
    int num_vertices = 0;
    HalfEdge *first_edge = get(face_index).edge;
    HalfEdge *current_edge = first_edge;
    do {
      VertexIndex vertex_index = IndexOf(current_edge->vertex);
      if(planar_vertex_indices.count(vertex_index))
        planar_vertex_indices_on_this_face.push_back(vertex_index);
      num_vertices++;
      current_edge = current_edge->next_edge;
    } while(current_edge != first_edge);

    // does this face have enough vertices for CutFace to work?
    if(num_vertices >= 4) {
      size_t num_vertices_on_plane = planar_vertex_indices_on_this_face.size();
      if(num_vertices_on_plane == 2) {
        // invalidates Face and HalfEdge pointers
        HalfEdgeIndex new_edge_index = CutFace(
          face_index,
          planar_vertex_indices_on_this_face[0],
          planar_vertex_indices_on_this_face[1]
        );
        if(!new_edge_index.IsNull()) {
          planar_edge_indices.insert(new_edge_index);
          planar_edge_indices.insert(IndexOf(get(new_edge_index).twin_edge));
        }
      } else if(num_vertices_on_plane > 2) {
        // TODO support concave faces
        std::cout << "HalfEdgeMesh::Bisect skipping face at "
          << CenterOfBoundingBox(face_index) << " with "
          << num_vertices_on_plane << " of " << num_vertices
          << " on the plane\n";
      }
    }

    planar_vertex_indices_on_this_face.clear();
  }

  #ifndef NDEBUG
  CheckAll();
  #endif

  return planar_edge_indices;
}

template<typename T>
std::tuple<HalfEdgeMesh::ComponentIndex<T>, uintptr_t>
HalfEdgeMesh::ComponentList<T>::Append(T e) {
  uintptr_t realloc_offset = 0; // unsigned so overflow is defined

  if(size_ == capacity_) {
    if(capacity_ == 0)
      capacity_ = 8;
    else
      capacity_ *= 2;

    T *new_list;
    if(std::is_pod<T>::value) {
      new_list = reinterpret_cast<T*>(
        std::realloc(list_, capacity_ * sizeof(T)));
    } else {
      new_list = new T[capacity_];
      for(size_t i = 0; i < size_; i++)
        new_list[i] = std::move(list_[i]);
      delete[] list_;
    }

    if(list_) {
      // Cast to uintptr_t before subtraction. Subtracting pointers directly
      // gives a result in units of sizeof(T) bytes, suitable for offsetting
      // an index into an array of T[]. However, "list_" and "new_list" may
      // not be divisible by sizeof(T) (if alignof(T) < sizeof(T)), and more
      // importantly may have different remainders divided by sizeof(T), so
      // pointer subtraction may give an inexact result.
      realloc_offset = uintptr_t(new_list) - uintptr_t(list_);
    }

    list_ = new_list;
  }

  list_[size_] = std::move(e);
  ComponentIndex<T> index(size_);
  size_++;
  return {index, realloc_offset};
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
HalfEdgeMesh MakeAlignedCells() {
  PrintingScopedTimer timer("MakeAlignedCells");
  HalfEdgeMesh mesh;

  using VertexIndex         = HalfEdgeMesh::VertexIndex;
  using VertexNormalIndex   = HalfEdgeMesh::VertexNormalIndex;
  using VertexPositionIndex = HalfEdgeMesh::VertexPositionIndex;
  using HalfEdgeIndex      = HalfEdgeMesh::HalfEdgeIndex;
  using FaceIndex           = HalfEdgeMesh::FaceIndex;
  using ObjectIndex         = HalfEdgeMesh::ObjectIndex;

  // create the 6 axis-aligned normal vectors
  VertexNormalIndex x_pos = mesh.AddVertexNormal( UnitX_Vector3d);
  VertexNormalIndex x_neg = mesh.AddVertexNormal(-UnitX_Vector3d);
  VertexNormalIndex y_pos = mesh.AddVertexNormal( UnitY_Vector3d);
  VertexNormalIndex y_neg = mesh.AddVertexNormal(-UnitY_Vector3d);
  VertexNormalIndex z_pos = mesh.AddVertexNormal( UnitZ_Vector3d);
  VertexNormalIndex z_neg = mesh.AddVertexNormal(-UnitZ_Vector3d);

  constexpr size_t size = std::size(AlignedPlaneOffsets);

  // a unique_ptr to a size x size x size 3D array of VertexPositionIndex
  auto positions = std::make_unique<VertexPositionIndex[][size][size]>(size);

  // populate vertex positions at every intersection of 3 axis-aligned planes
  for(size_t zi = 0; zi < size; zi++) {
    double z = AlignedPlaneOffsets[zi];
    for(size_t yi = 0; yi < size; yi++) {
      double y = AlignedPlaneOffsets[yi];
      for(size_t xi = 0; xi < size; xi++) {
        double x = AlignedPlaneOffsets[xi];
        positions.get()[zi][yi][xi] = 
          mesh.AddVertexPosition(Vector3d{x,y,z});
      }
    }
  }

  for(size_t zi = 0; zi < size-1; zi++) {
    for(size_t yi = 0; yi < size-1; yi++) {
      for(size_t xi = 0; xi < size-1; xi++) {
        std::string object_name = std::to_string(zi) + '-' +
          std::to_string(yi) + '-' + std::to_string(xi);
        ObjectIndex object = mesh.AddObject(std::move(object_name));

        FaceIndex faces[6];
        for(int i = 0; i < 6; i++) {
          faces[i] = mesh.AddFace();
          mesh[faces[i]].object = &mesh[object];
        }

        VertexIndex vertices[8];
        vertices[0] = mesh.AddVertex();
        vertices[1] = mesh.AddVertex();
        vertices[2] = mesh.AddVertex();
        vertices[3] = mesh.AddVertex();
        vertices[4] = mesh.AddVertex();
        vertices[5] = mesh.AddVertex();
        vertices[6] = mesh.AddVertex();
        vertices[7] = mesh.AddVertex();

        mesh[vertices[0]].position = &mesh[positions.get()[zi  ][yi  ][xi  ]];
        mesh[vertices[1]].position = &mesh[positions.get()[zi  ][yi  ][xi+1]];
        mesh[vertices[2]].position = &mesh[positions.get()[zi  ][yi+1][xi  ]];
        mesh[vertices[3]].position = &mesh[positions.get()[zi  ][yi+1][xi+1]];
        mesh[vertices[4]].position = &mesh[positions.get()[zi+1][yi  ][xi  ]];
        mesh[vertices[5]].position = &mesh[positions.get()[zi+1][yi  ][xi+1]];
        mesh[vertices[6]].position = &mesh[positions.get()[zi+1][yi+1][xi  ]];
        mesh[vertices[7]].position = &mesh[positions.get()[zi+1][yi+1][xi+1]];

        // create the 12 edges (2 half-edges each) of the cell
        HalfEdgeIndex edges[24];
        for(int i = 0; i < 24; i++)
          edges[i] = mesh.AddHalfEdge();

        // a macro to help fill in the edge data
        #define hcm_set_linear_edge(this, twin, next, face_, vert, norm) { \
          HalfEdgeMesh::HalfEdge &edge = mesh[edges[this]];                \
          edge.twin_edge = &mesh[edges[twin]];                             \
          edge.next_edge = &mesh[edges[next]];                             \
          edge.face = &mesh[faces[face_]];                                 \
          edge.vertex = &mesh[vertices[vert]];                             \
          edge.normal = &mesh[norm];                                       \
        }

        //                   this twin next face vert norm
        hcm_set_linear_edge(   0,   1,   4,   4,   0, z_neg)
        hcm_set_linear_edge(   1,   0,  18,   2,   1, y_neg)
        hcm_set_linear_edge(   2,   3,   6,   4,   3, z_neg)
        hcm_set_linear_edge(   3,   2,  20,   3,   2, y_pos)
        hcm_set_linear_edge(   4,   5,   2,   4,   2, z_neg)
        hcm_set_linear_edge(   5,   4,  17,   0,   0, x_neg)
        hcm_set_linear_edge(   6,   7,   0,   4,   1, z_neg)
        hcm_set_linear_edge(   7,   6,  23,   1,   3, x_pos)
        hcm_set_linear_edge(   8,   9,  14,   5,   5, z_pos)
        hcm_set_linear_edge(   9,   8,  16,   2,   4, y_neg)
        hcm_set_linear_edge(  10,  11,  12,   5,   6, z_pos)
        hcm_set_linear_edge(  11,  10,  22,   3,   7, y_pos)
        hcm_set_linear_edge(  12,  13,   8,   5,   4, z_pos)
        hcm_set_linear_edge(  13,  12,  21,   0,   6, x_neg)
        hcm_set_linear_edge(  14,  15,  10,   5,   7, z_pos)
        hcm_set_linear_edge(  15,  14,  19,   1,   5, x_pos)
        hcm_set_linear_edge(  16,  17,   1,   2,   0, y_neg)
        hcm_set_linear_edge(  17,  16,  13,   0,   4, x_neg)
        hcm_set_linear_edge(  18,  19,   9,   2,   5, y_neg)
        hcm_set_linear_edge(  19,  18,   7,   1,   1, x_pos)
        hcm_set_linear_edge(  20,  21,  11,   3,   6, y_pos)
        hcm_set_linear_edge(  21,  20,   5,   0,   2, x_neg)
        hcm_set_linear_edge(  22,  23,   3,   3,   3, y_pos)
        hcm_set_linear_edge(  23,  22,  15,   1,   7, x_pos)

        #undef set_linear_edge

        mesh[faces[0]].edge = &mesh[edges[5]];
        mesh[faces[1]].edge = &mesh[edges[7]];
        mesh[faces[2]].edge = &mesh[edges[1]];
        mesh[faces[3]].edge = &mesh[edges[3]];
        mesh[faces[4]].edge = &mesh[edges[2]];
        mesh[faces[5]].edge = &mesh[edges[8]];

        mesh[vertices[0]].edge = &mesh[edges[1]];
        mesh[vertices[1]].edge = &mesh[edges[0]];
        mesh[vertices[2]].edge = &mesh[edges[2]];
        mesh[vertices[3]].edge = &mesh[edges[3]];
        mesh[vertices[4]].edge = &mesh[edges[8]];
        mesh[vertices[5]].edge = &mesh[edges[9]];
        mesh[vertices[6]].edge = &mesh[edges[11]];
        mesh[vertices[7]].edge = &mesh[edges[10]];
      }
    }
  }

  #ifndef NDEBUG
  mesh.CheckAll();
  #endif

  return mesh;
}

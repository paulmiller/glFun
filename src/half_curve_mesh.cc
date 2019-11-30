#include "half_curve_mesh.h"

#include "scoped_timer.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <memory>
#include <stack>
#include <unordered_map>

HalfCurveMesh::VertexIndex HalfCurveMesh::AddVertex() {
  auto [index, realloc_offset] = vertices_.Append({});
  if(realloc_offset) {
    for(HalfCurve &curve: half_curves_) {
      if(curve.vertex)
        curve.vertex = (Vertex*)(uintptr_t(curve.vertex) + realloc_offset);
    }
  }
  return index;
}

HalfCurveMesh::VertexPositionIndex
HalfCurveMesh::AddVertexPosition(const Vector3d &position) {
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

HalfCurveMesh::VertexNormalIndex
HalfCurveMesh::AddVertexNormal(const Vector3d &normal) {
  auto [index, realloc_offset] = vertex_normals_.Append(normal);
  if(realloc_offset) {
    for(HalfCurve &curve: half_curves_) {
      if(curve.normal)
        curve.normal = (Vector3d*)(uintptr_t(curve.normal) + realloc_offset);
    }
  }
  return index;
}

HalfCurveMesh::HalfCurveIndex HalfCurveMesh::AddHalfCurve() {
  auto [index, realloc_offset] = half_curves_.Append({});
  if(realloc_offset) {
    for(Vertex &vertex: vertices_) {
      if(vertex.curve)
        vertex.curve = (HalfCurve*)(uintptr_t(vertex.curve) + realloc_offset);
    }
    for(HalfCurve &other_curve: half_curves_) {
      if(other_curve.twin_curve)
        other_curve.twin_curve =
          (HalfCurve*)(uintptr_t(other_curve.twin_curve) + realloc_offset);
      if(other_curve.next_curve)
        other_curve.next_curve =
          (HalfCurve*)(uintptr_t(other_curve.next_curve) + realloc_offset);
    }
    for(Face &face: faces_) {
      if(face.curve)
        face.curve = (HalfCurve*)(uintptr_t(face.curve) + realloc_offset);
    }
  }
  return index;
}

HalfCurveMesh::FaceIndex HalfCurveMesh::AddFace() {
  auto [index, realloc_offset] = faces_.Append({});
  if(realloc_offset) {
    for(HalfCurve &curve: half_curves_) {
      if(curve.face)
        curve.face = (Face*)(uintptr_t(curve.face) + realloc_offset);
    }
  }
  return index;
}

HalfCurveMesh::ObjectIndex HalfCurveMesh::AddObject(std::string name) {
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
void HalfCurveMesh::Check() const {
  PrintingScopedTimer timer("HalfCurveMesh::Check");

  for(const HalfCurve &curve: half_curves_) {
    vertices_.CheckPtr(curve.vertex);
    vertex_positions_.CheckPtr(curve.vertex->position);
    half_curves_.CheckPtr(curve.vertex->curve);
    vertex_normals_.CheckPtr(curve.normal);
    half_curves_.CheckPtr(curve.twin_curve);
    half_curves_.CheckPtr(curve.next_curve);
    faces_.CheckPtr(curve.face);
    objects_.CheckPtr(curve.face->object);
    half_curves_.CheckPtr(curve.face->curve);
  }

  // Every time a mesh component with index = X is referenced by some other
  // component, mark vector[X] = true in the corresponding vector. They should
  // become all true; there should be no unused elements.
  std::vector<bool> vertices_used(vertices_.size());
  std::vector<bool> vertex_positions_used(vertex_positions_.size());
  std::vector<bool> vertex_normals_used(vertex_normals_.size());
  std::vector<bool> faces_used(faces_.size());
  std::vector<bool> objects_used(objects_.size());

  for(const HalfCurve &curve: half_curves_) {
    {
      auto &vp = vertex_positions_;
      auto &vn = vertex_normals_;
      vertices_used         [    IndexOf(curve.vertex          ).value ] = true;
      faces_used            [    IndexOf(curve.face            ).value ] = true;
      objects_used          [    IndexOf(curve.face->object    ).value ] = true;
      vertex_positions_used [ vp.IndexOf(curve.vertex->position).value ] = true;
      vertex_normals_used   [ vn.IndexOf(curve.normal          ).value ] = true;
    }

    assert(curve.vertex->position->isfinite());
    assert(curve.normal->isfinite());

    assert(&curve != curve.twin_curve);
    assert(curve.type == curve.twin_curve->type);
    assert(&curve == curve.twin_curve->twin_curve);
    assert(curve.next_curve != curve.twin_curve);
    assert(curve.twin_curve->next_curve != &curve);
    assert(curve.face != curve.twin_curve->face);
    assert(curve.vertex != curve.twin_curve->vertex);
    assert(curve.face->object == curve.twin_curve->face->object);

    const Vector3d *start = curve.twin_curve->vertex->position;
    const Vector3d *end = curve.vertex->position;
    // TODO threshold?
    assert((*end - *start).len2() > 0.001);

    // compare to every other curve on the same object and ensure they're
    // different (slow)
    for(const HalfCurve &other_curve: half_curves_) {
      if(&other_curve == &curve) continue;
      if(&other_curve == curve.twin_curve) continue;
      if(other_curve.face->object != curve.face->object) continue;

      const Vector3d *other_start = other_curve.twin_curve->vertex->position;
      const Vector3d *other_end = other_curve.vertex->position;

      // TODO threshold?
      assert((*start - *other_start).len2() > 0.001 ||
        (*end - *other_end).len2() > 0.001);
      assert((*start - *other_end).len2() > 0.001 ||
        (*end - *other_start).len2() > 0.001);
    }

    // walk the HalfCurves surrounding curve.face
    int curve_num = 0;
    bool found_face_curve = false;
    const HalfCurve *current_curve = &curve;
    do {
      assert(current_curve->face == curve.face);
      if(current_curve == curve.face->curve)
        found_face_curve = true;
      current_curve = current_curve->next_curve;
      curve_num++;
    } while(current_curve != &curve);
    assert(curve_num >= 3);
    assert(found_face_curve);

    // walk the HalfCurves surrounding curve.vertex
    bool found_this_curve = false;
    const HalfCurve *first_outgoing_curve = curve.vertex->curve;
    const HalfCurve *outgoing_curve = first_outgoing_curve;
    do {
      const HalfCurve *incoming_curve = outgoing_curve->twin_curve;
      assert(incoming_curve->vertex == curve.vertex);
      if(incoming_curve == &curve)
        found_this_curve = true;
      outgoing_curve = incoming_curve->next_curve;
    } while(outgoing_curve != first_outgoing_curve);
    assert(found_this_curve);
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

std::unordered_set<HalfCurveMesh::Face*>
HalfCurveMesh::FindConnectedFaces(Face *start_face) {
  std::unordered_set<Face*> visited;
  std::stack<Face*> stack;
  stack.push(start_face);
  while(!stack.empty()) {
    Face *current_face = stack.top();
    stack.pop();
    visited.insert(current_face);

    HalfCurve *start_curve = current_face->curve;
    HalfCurve *current_curve = start_curve;
    do {
      Face *next_face = current_curve->twin_curve->face;
      if(!visited.count(next_face))
        stack.push(next_face);
      current_curve = current_curve->next_curve;
    } while(current_curve != start_curve);
  }
  return visited;
}

WavFrObj HalfCurveMesh::MakeWavFrObj() const {
  PrintingScopedTimer timer("HalfCurveMesh::MakeWavFrObj");

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

    const HalfCurve *first_curve = face.curve;
    const HalfCurve *curve = first_curve;
    do {
      size_t position_index =
        vertex_positions_.IndexOf(curve->vertex->position).value;
      size_t normal_index = vertex_normals_.IndexOf(curve->normal).value;
      wavfr_face_verts.push_back(
        WavFrObj::ObjVert{int(position_index), -1, int(normal_index)});
      curve = curve->next_curve;
    } while(curve != first_curve);

    size_t object_index = IndexOf(face.object).value;
    wavfr_objects[object_index].addFace(
      std::move(wavfr_face_verts));
  }

  return WavFrObj(std::move(wavfr_vertices), std::vector<UvCoord>(),
    std::move(wavfr_normals), std::move(wavfr_objects));
}

Vector3d HalfCurveMesh::CenterOfBoundingBox(FaceIndex face_index) const {
  constexpr double inf = std::numeric_limits<double>::infinity();
  double x_min = inf, x_max = -inf,
         y_min = inf, y_max = -inf,
         z_min = inf, z_max = -inf;

  HalfCurve *first_curve = get(face_index).curve;
  HalfCurve *curve = first_curve;
  do {
    Vector3d position = *(curve->vertex->position);
    x_min = std::min(x_min, position.x); x_max = std::max(x_max, position.x);
    y_min = std::min(y_min, position.y); y_max = std::max(y_max, position.y);
    z_min = std::min(z_min, position.z); z_max = std::max(z_max, position.z);
    curve = curve->next_curve;
  } while(curve != first_curve);

  assert(std::isfinite(x_min)); assert(std::isfinite(x_max));
  assert(std::isfinite(y_min)); assert(std::isfinite(y_max));
  assert(std::isfinite(z_min)); assert(std::isfinite(z_max));

  return Vector3d{ (x_min+x_max)/2, (y_min+y_max)/2, (z_min+z_max)/2 };
}

HalfCurveMesh::VertexIndex
HalfCurveMesh::CutCurve(HalfCurveIndex curve_index, double t) {
  VertexIndex new_vertex_index = AddVertex();
  HalfCurveIndex new_curve_a_index = AddHalfCurve(); 
  HalfCurveIndex new_curve_b_index = AddHalfCurve();

  // reallocs may invalidate pointers, so add all components before getting
  // pointers

  HalfCurve *curve = &get(curve_index);
  HalfCurve *twin_curve = curve->twin_curve;
  HalfCurve *new_curve_a = &get(new_curve_a_index);
  HalfCurve *new_curve_b = &get(new_curve_b_index);

  Vertex *start = curve->twin_curve->vertex;
  Vertex *end = curve->vertex;
  Vertex *new_vertex = &get(new_vertex_index);

  // the curves now look like this:
  //          _ _ _ _ _ _
  //        🡕    curve    🡖
  // start *               * end
  //        🡔 _ _ _ _ _ _ 🡗
  //        curve.twin_curve

  // TODO deduplicate positions
  Vector3d start_position = *(start->position);
  Vector3d end_position = *(end->position);
  Vector3d new_vertex_position =
    start_position + t * (end_position - start_position);
  new_vertex->position = &get(AddVertexPosition(new_vertex_position));
  curve->vertex = new_vertex;
  curve->twin_curve->vertex = new_vertex;

  // the curves now look like this:
  //    _ _ _
  //  🡕 curve 🡖
  // *         * new     *
  //            🡔 _ _ _ 🡗
  //            twin_curve


  new_curve_a->type = CurveType::Linear; // TODO support other curve types
  new_curve_a->twin_curve = curve->twin_curve;
  new_curve_a->next_curve = curve->next_curve;
  new_curve_a->face = curve->face;
  new_curve_a->vertex = end;
  new_curve_a->normal = curve->normal;

  new_curve_b->type = CurveType::Linear;
  new_curve_b->twin_curve = curve;
  new_curve_b->next_curve = twin_curve->next_curve;
  new_curve_b->face = twin_curve->face;
  new_curve_b->vertex = start;
  new_curve_b->normal = twin_curve->normal;

  curve->twin_curve->twin_curve = new_curve_a;
  curve->twin_curve->next_curve = new_curve_b;

  curve->twin_curve = new_curve_b;
  curve->next_curve = new_curve_a;

  new_vertex->curve = new_curve_a;

  // the curves now look like this:
  //    _ _ _     _ _ _
  //  🡕 curve 🡖 🡕 new a 🡖
  // *         *         *
  //  🡔 _ _ _ 🡗 🡔 _ _ _ 🡗
  //    new b   twin_curve

  /*
  #ifndef NDEBUG
  Check();
  #endif
  */

  return new_vertex_index;
}

HalfCurveMesh::HalfCurveIndex HalfCurveMesh::CutFace(
  FaceIndex face_idx, VertexIndex vertex_a_idx, VertexIndex vertex_b_idx
) {
  assert(vertex_a_idx != vertex_b_idx);

  FaceIndex new_face_idx = AddFace();
  // TODO support other curve types
  HalfCurveIndex new_curve_idx = AddHalfCurve();
  HalfCurveIndex new_curve_twin_idx = AddHalfCurve();

  // reallocs may invalidate pointers, so add all components before getting
  // pointers

  Vertex *vertex_a = &get(vertex_a_idx);
  Vertex *vertex_b = &get(vertex_b_idx);
  HalfCurve *new_curve = &get(new_curve_idx);
  HalfCurve *new_curve_twin = &get(new_curve_twin_idx);
  Face *face = &get(face_idx);
  Face *new_face = &get(new_face_idx);

  HalfCurve *curve_a_in = nullptr, *curve_a_out = nullptr;
  HalfCurve *curve_b_in = nullptr, *curve_b_out = nullptr;
  HalfCurve *first_curve = face->curve;
  HalfCurve *current_curve = first_curve;
  do {
    if(current_curve->vertex == vertex_a) {
      curve_a_in = current_curve;
      curve_a_out = current_curve->next_curve;
    }
    if(current_curve->vertex == vertex_b) {
      curve_b_in = current_curve;
      curve_b_out = current_curve->next_curve;
    }
    current_curve = current_curve->next_curve;
  } while(current_curve != first_curve);
  assert(curve_a_in); assert(curve_a_out);
  assert(curve_b_in); assert(curve_b_out);

  if(curve_a_in == curve_b_out || curve_a_out == curve_b_in)
    return HalfCurveIndex();

  // the face now looks like this:
  //
  //              /             \
  //  curve_a_in /               \ curve_b_out
  //            /                 \
  //  vertex_a *       face        * vertex_b
  //            \                 /
  // curve_a_out \               / curve_b_in
  //              \             /

  new_face->object = face->object;

  face->curve = new_curve;
  new_face->curve = new_curve_twin;

  new_curve->twin_curve = new_curve_twin;
  new_curve->next_curve = curve_b_out;
  new_curve->face = face;
  new_curve->vertex = vertex_b;
  new_curve->normal = curve_b_in->normal;

  new_curve_twin->twin_curve = new_curve;
  new_curve_twin->next_curve = curve_a_out;
  new_curve_twin->face = new_face;
  new_curve_twin->vertex = vertex_a;
  new_curve_twin->normal = curve_a_in->normal;

  curve_a_in->next_curve = new_curve;
  curve_b_in->next_curve = new_curve_twin;

  current_curve = curve_a_out;
  do {
    current_curve->face = new_face;
    current_curve = current_curve->next_curve;
  } while(current_curve != new_curve_twin);

  // the faces now look like this:
  //
  //              /    face     \
  //  curve_a_in /               \ curve_b_out
  //            /    new_curve    \
  //  vertex_a * - - - - - - - - - * vertex_b
  //            \ new_curve_twin  /
  // curve_a_out \               / curve_b_in
  //              \  new_face   /

  /*
  #ifndef NDEBUG
  Check();
  #endif
  */

  return new_curve_idx;
}

void HalfCurveMesh::LoopCut(std::unordered_set<HalfCurveIndex> curve_idxs) {
  PrintingScopedTimer timer("HalfCurveMesh::LoopCut");

  #ifndef NDEBUG
  // "curve_idxs" must contain only matched pairs of HalfCurves
  for(HalfCurveIndex curve_idx: curve_idxs) {
    HalfCurveIndex twin_idx = IndexOf(get(curve_idx).twin_curve);
    assert(curve_idxs.count(twin_idx));
  }

  // ensure every Vertex has 0 or 2 ingoing and outgoing curves in "curve_idxs"
  // i.e. there is at most 1 cutting path through each Vertex
  for(Vertex &vertex: vertices_) {
    int outgoing_cut_curves = 0, incoming_cut_curves = 0;

    HalfCurve *first_outgoing_curve = vertex.curve;
    HalfCurve *outgoing_curve = first_outgoing_curve;
    do {
      HalfCurve *incoming_curve = outgoing_curve->twin_curve;

      if(curve_idxs.count(IndexOf(outgoing_curve)))
        outgoing_cut_curves++;
      if(curve_idxs.count(IndexOf(incoming_curve)))
        incoming_cut_curves++;

      outgoing_curve = incoming_curve->next_curve;
    } while(outgoing_curve != first_outgoing_curve);

    assert(outgoing_cut_curves == incoming_cut_curves);
    assert(outgoing_cut_curves == 0 || outgoing_cut_curves == 2);
  }
  #endif

  // a map from each Index in "curve_idxs" to the Index of the next HalfCurve in
  // the loop
  std::unordered_map<HalfCurveIndex, HalfCurveIndex> next_loop_curve_idxs;

  // populate "next_loop_curve_idxs"
  for(HalfCurveIndex curve_idx: curve_idxs) {
    assert(!next_loop_curve_idxs.count(curve_idx));
    HalfCurve *curve = &get(curve_idx);
    // find the HalfCurve following "curve" in the loop: this is the HalfCurve
    // exiting the Vertex "curve->vertex", which is not the twin of "curve", and
    // is in the set of loop curves "curve_idxs"
    HalfCurve *first_outgoing_curve = curve->vertex->curve;
    HalfCurve *outgoing_curve = first_outgoing_curve;
    while(outgoing_curve == curve->twin_curve ||
        !curve_idxs.count(IndexOf(outgoing_curve))) {
      outgoing_curve = outgoing_curve->twin_curve->next_curve;
      // the loop should terminate before getting back to
      // "first_outgoing_curve_id"
      if(outgoing_curve == first_outgoing_curve) {
        std::cout << "HalfCurveMesh::LoopCut failed: couldn't follow loop "
          "through vertex at " << *(curve->vertex->position) << '\n';
        return;
      }
    }
    next_loop_curve_idxs[curve_idx] = IndexOf(outgoing_curve);
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
  while(!curve_idxs.empty()) {
    // take an arbitrary HalfCurve and cut its associated loop
    HalfCurveIndex first_curve_idx = *(curve_idxs.begin());

    FaceIndex new_face_index = AddFace();
    new_face_indices.insert(new_face_index);
    Face *new_face = &get(new_face_index); // invalidates Face pointers

    // may be reassigned to a new Object later
    new_face->object = get(first_curve_idx).face->object;

    // remember the 1st 3 vertices along the loop to get a normal vector later
    Vector3d sample_vertex_positions[3];
    int sample_vertices = 0;

    VertexIndex saved_split_vertex_index;
    HalfCurveIndex prev_curve_idx;
    HalfCurveIndex curve_idx = first_curve_idx;
    do {
      // TODO pick these to avoid NaN normals
      if(sample_vertices < 3) {
        Vector3d *position = get(curve_idx).vertex->position;
        sample_vertex_positions[sample_vertices] = *position;
        sample_vertices++;
      }

      // invalidates HalfCurve pointers
      HalfCurve *new_twin_curve = &get(AddHalfCurve()); 
      HalfCurve *curve = &get(curve_idx);
      HalfCurve *old_twin_curve = curve->twin_curve;

      new_twin_curve->type = CurveType::Linear; // TODO other types
      new_twin_curve->twin_curve = curve;
      new_twin_curve->face = new_face;

      VertexIndex old_start_vertex_index = IndexOf(old_twin_curve->vertex);
      if(claimed_vertex_indices.count(old_start_vertex_index)) {
        // invalidates Vertex pointers
        VertexIndex new_start_vertex_index = AddVertex();
        Vertex *new_start_vertex = &get(new_start_vertex_index);
        new_start_vertex->position = get(old_start_vertex_index).position;
        new_start_vertex->curve = curve;
        new_twin_curve->vertex = new_start_vertex;

        // Update all the HalfCurves on this side of the cut, that used to point
        // to the claimed Vertex, to point to the new Vertex, starting with the
        // "previous" HalfCurve. Or if we can't, because this is the first
        // iteration, and "prev_curve_idx" is null, then save the new Vertex so
        // we can update it later.
        if(prev_curve_idx.IsNull()) {
          saved_split_vertex_index = new_start_vertex_index;
        } else {
          HalfCurve *first_incoming_curve = &get(prev_curve_idx);
          HalfCurve *incoming_curve = first_incoming_curve;
          for(;;) {
            assert(incoming_curve->vertex == old_twin_curve->vertex);
            incoming_curve->vertex = new_start_vertex;
            if(curve_idxs.count(IndexOf(incoming_curve->next_curve)))
              break;
            incoming_curve = incoming_curve->next_curve->twin_curve;
            // we should end the fan before going all the way around the Vertex
            assert(incoming_curve != first_incoming_curve);
          }
        }
      } else {
        claimed_vertex_indices.insert(old_start_vertex_index);
        old_twin_curve->vertex->curve = curve;
        new_twin_curve->vertex = old_twin_curve->vertex;
      }

      // "curve" points at "new_twin_curve", but "old_twin_curve" may still
      // point at "curve". This will be fixed when "old_twin_curve"'s loop comes
      // up for cutting.
      curve->twin_curve = new_twin_curve;
      new_face->curve = new_twin_curve;

      curve_idxs.erase(curve_idx);
      prev_curve_idx = curve_idx;
      curve_idx = next_loop_curve_idxs[curve_idx];
    } while(curve_idx != first_curve_idx);
    assert(new_face->curve != nullptr);

    if(!saved_split_vertex_index.IsNull()) {
      // Update the HalfCurves on this side of the cut for the saved Vertex. The
      // difference is that we can't check "curve_idxs" to see when we've
      // reached the end of the fan, because all the HalfCurves on this side of
      // the loop cut have been removed from "curve_idxs". But since the
      // "previous" HalfCurve is now the one right behind "loop_start_curve" in
      // the loop, we can use "loop_start_curve" to mark the end of the fan.
      assert(!prev_curve_idx.IsNull());
      Vertex *new_start_vertex = &get(saved_split_vertex_index);
      HalfCurve *first_incoming_curve = &get(prev_curve_idx);
      HalfCurve *incoming_curve = first_incoming_curve;
      HalfCurve *loop_start_curve = &get(curve_idx);
      for(;;) {
        assert(incoming_curve->vertex != new_start_vertex);
        incoming_curve->vertex = new_start_vertex;
        if(incoming_curve->next_curve == loop_start_curve)
          break;
        incoming_curve = incoming_curve->next_curve->twin_curve;
        // we should end the fan before going all the way around the Vertex
        assert(incoming_curve != first_incoming_curve);
      }
    }

    assert(sample_vertices == 3);
    Vector3d &a = sample_vertex_positions[0];
    Vector3d &b = sample_vertex_positions[1];
    Vector3d &c = sample_vertex_positions[2];
    Vector3d face_normal = cross(b - a, c - a).unit();
    assert(face_normal.isfinite());
    // invalidates previous normal pointers
    Vector3d *new_normal = &get(AddVertexNormal(face_normal));

    curve_idx = first_curve_idx;
    do {
      HalfCurve *curve = &get(curve_idx);
      HalfCurve *new_twin_curve = curve->twin_curve;

      new_twin_curve->next_curve = get(prev_curve_idx).twin_curve;
      new_twin_curve->normal = new_normal;

      prev_curve_idx = curve_idx;
      curve_idx = next_loop_curve_idxs[curve_idx];
    } while(curve_idx != first_curve_idx);
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

  #ifndef NDEBUG
  Check();
  #endif
}

std::unordered_set<HalfCurveMesh::HalfCurveIndex>
HalfCurveMesh::Bisect(const Vector3d &normal) {
  PrintingScopedTimer timer("HalfCurveMesh::Bisect");

  // all vertices lying on the plane: both new vertices created to bisect
  // curves, and existing vertices that happened to be on the plane already
  std::unordered_set<VertexIndex> planar_vertex_indices;

  // all curves (and their twins) lying on the plane: new curves bisecting
  // faces, and existing curves
  std::unordered_set<HalfCurveIndex> planar_curve_indices;

  // a set of HalfCurve IDs to skip, because we already checked their twin
  std::unordered_set<HalfCurve*> checked_twin_curves;

  // TODO support other curve types
  size_t curve_num = half_curves_.size();
  for(HalfCurveIndex curve_index(0); curve_index < curve_num; ++curve_index) {
    HalfCurve *curve = &get(curve_index);
    if(checked_twin_curves.count(curve)) continue;
    checked_twin_curves.insert(curve->twin_curve);

    // line equation: S + t⋅D
    Vector3d S = *(curve->twin_curve->vertex->position);
    Vector3d D = *(curve->vertex->position) - S;

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
    // angles, and denominator == 0
    double denominator = dot(normal, D);
    if(denominator == 0) {
      // is the line inside the plane?
      if(S.x + S.y + S.z == 0) {
        planar_curve_indices.insert(curve_index);
        planar_curve_indices.insert(IndexOf(curve->twin_curve));
      }
    } else {
      double t = - dot(normal, S) / denominator;

      // TODO threshold?
      constexpr double epsilon = 0.0001;

      // does the intersection lie within the line segment?
      if(epsilon < t && t < 1-epsilon) {
        // Invalidates HalfCurve pointers. We got "curve_num" before the for
        // loop, so we won't iterate over any new HalfCurves appended by
        // CutCurve.
        planar_vertex_indices.insert(CutCurve(curve_index, t));
      }

      // does the intersection lie at one end of the segment?
      if(-epsilon < t && t < epsilon) {
        VertexIndex start_vertex = IndexOf(get(curve_index).twin_curve->vertex);
        planar_vertex_indices.insert(start_vertex);
      } else if(1-epsilon < t && t < 1+epsilon) {
        VertexIndex end_vertex = IndexOf(get(curve_index).vertex);
        planar_vertex_indices.insert(end_vertex);
      }
    }
  }

  std::vector<VertexIndex> planar_vertex_indices_on_this_face;
  size_t face_num = faces_.size();
  for(FaceIndex face_index(0); face_index < face_num; ++face_index) {
    int num_vertices = 0;
    HalfCurve *first_curve = get(face_index).curve;
    HalfCurve *current_curve = first_curve;
    do {
      VertexIndex vertex_index = IndexOf(current_curve->vertex);
      if(planar_vertex_indices.count(vertex_index))
        planar_vertex_indices_on_this_face.push_back(vertex_index);
      num_vertices++;
      current_curve = current_curve->next_curve;
    } while(current_curve != first_curve);

    // does this face have enough vertices for CutFace to work?
    if(num_vertices >= 4) {
      size_t num_vertices_on_plane = planar_vertex_indices_on_this_face.size();
      if(num_vertices_on_plane == 2) {
        // invalidates Face and HalfCurve pointers
        HalfCurveIndex new_curve_index = CutFace(
          face_index,
          planar_vertex_indices_on_this_face[0],
          planar_vertex_indices_on_this_face[1]
        );
        if(!new_curve_index.IsNull()) {
          planar_curve_indices.insert(new_curve_index);
          planar_curve_indices.insert(IndexOf(get(new_curve_index).twin_curve));
        } else {
          // TODO remove
          std::cout << "-1\n";
        }
      } else if(num_vertices_on_plane > 2) {
        // TODO support concave faces
        std::cout << "HalfCurveMesh::Bisect skipping face at "
          << CenterOfBoundingBox(face_index) << " with " << num_vertices_on_plane
          << " of " << num_vertices << " on the plane\n";
      }
    }

    planar_vertex_indices_on_this_face.clear();
  }

  #ifndef NDEBUG
  Check();
  #endif

  return planar_curve_indices;
}

template<typename T>
std::tuple<HalfCurveMesh::ComponentIndex<T>, uintptr_t>
HalfCurveMesh::ComponentList<T>::Append(T e) {
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
HalfCurveMesh MakeAlignedCells() {
  PrintingScopedTimer timer("MakeAlignedCells");
  HalfCurveMesh mesh;

  using VertexIndex         = HalfCurveMesh::VertexIndex;
  using VertexNormalIndex   = HalfCurveMesh::VertexNormalIndex;
  using VertexPositionIndex = HalfCurveMesh::VertexPositionIndex;
  using HalfCurveIndex      = HalfCurveMesh::HalfCurveIndex;
  using FaceIndex           = HalfCurveMesh::FaceIndex;
  using ObjectIndex         = HalfCurveMesh::ObjectIndex;

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
        HalfCurveIndex curves[24];
        for(int i = 0; i < 24; i++)
          curves[i] = mesh.AddHalfCurve();

        // a macro to help fill in the edge data
        #define hcm_set_linear_curve(this, twin, next, face_, vert, norm) { \
          HalfCurveMesh::HalfCurve &curve = mesh[curves[this]];             \
          curve.type = HalfCurveMesh::CurveType::Linear;                    \
          curve.twin_curve = &mesh[curves[twin]];                           \
          curve.next_curve = &mesh[curves[next]];                           \
          curve.face = &mesh[faces[face_]];                                 \
          curve.vertex = &mesh[vertices[vert]];                             \
          curve.normal = &mesh[norm];                                       \
        }

        //                   this twin next face vert norm
        hcm_set_linear_curve(   0,   1,   4,   4,   0, z_neg)
        hcm_set_linear_curve(   1,   0,  18,   2,   1, y_neg)
        hcm_set_linear_curve(   2,   3,   6,   4,   3, z_neg)
        hcm_set_linear_curve(   3,   2,  20,   3,   2, y_pos)
        hcm_set_linear_curve(   4,   5,   2,   4,   2, z_neg)
        hcm_set_linear_curve(   5,   4,  17,   0,   0, x_neg)
        hcm_set_linear_curve(   6,   7,   0,   4,   1, z_neg)
        hcm_set_linear_curve(   7,   6,  23,   1,   3, x_pos)
        hcm_set_linear_curve(   8,   9,  14,   5,   5, z_pos)
        hcm_set_linear_curve(   9,   8,  16,   2,   4, y_neg)
        hcm_set_linear_curve(  10,  11,  12,   5,   6, z_pos)
        hcm_set_linear_curve(  11,  10,  22,   3,   7, y_pos)
        hcm_set_linear_curve(  12,  13,   8,   5,   4, z_pos)
        hcm_set_linear_curve(  13,  12,  21,   0,   6, x_neg)
        hcm_set_linear_curve(  14,  15,  10,   5,   7, z_pos)
        hcm_set_linear_curve(  15,  14,  19,   1,   5, x_pos)
        hcm_set_linear_curve(  16,  17,   1,   2,   0, y_neg)
        hcm_set_linear_curve(  17,  16,  13,   0,   4, x_neg)
        hcm_set_linear_curve(  18,  19,   9,   2,   5, y_neg)
        hcm_set_linear_curve(  19,  18,   7,   1,   1, x_pos)
        hcm_set_linear_curve(  20,  21,  11,   3,   6, y_pos)
        hcm_set_linear_curve(  21,  20,   5,   0,   2, x_neg)
        hcm_set_linear_curve(  22,  23,   3,   3,   3, y_pos)
        hcm_set_linear_curve(  23,  22,  15,   1,   7, x_pos)

        #undef set_linear_curve

        mesh[faces[0]].curve = &mesh[curves[5]];
        mesh[faces[1]].curve = &mesh[curves[7]];
        mesh[faces[2]].curve = &mesh[curves[1]];
        mesh[faces[3]].curve = &mesh[curves[3]];
        mesh[faces[4]].curve = &mesh[curves[2]];
        mesh[faces[5]].curve = &mesh[curves[8]];

        mesh[vertices[0]].curve = &mesh[curves[1]];
        mesh[vertices[1]].curve = &mesh[curves[0]];
        mesh[vertices[2]].curve = &mesh[curves[2]];
        mesh[vertices[3]].curve = &mesh[curves[3]];
        mesh[vertices[4]].curve = &mesh[curves[8]];
        mesh[vertices[5]].curve = &mesh[curves[9]];
        mesh[vertices[6]].curve = &mesh[curves[11]];
        mesh[vertices[7]].curve = &mesh[curves[10]];
      }
    }
  }

  #ifndef NDEBUG
  mesh.Check();
  #endif

  return mesh;
}

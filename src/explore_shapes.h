#ifndef EXPLORE_SHAPES_H
#define EXPLORE_SHAPES_H

#include "mesh.h"
#include "bool_voxel_volume.h"

#include <memory>
#include <unordered_set>

class Shape {
public:
  /*
  Shape(Shape &&shape) :
    voxels(std::move(shape.voxels)),
    hash(shape.hash),
    have_hash(shape.have_hash)
  {
    shape.have_hash = false;
  }
  */

  Shape(BoolVoxelVolume &&voxels, int generation) :
    voxels(voxels), hash(0), have_hash(false), generation(generation) {}
  BoolVoxelVolume voxels;
  uint64_t hash;
  bool have_hash;
  int generation;
};

class ShapeHasher {
public:
  size_t operator()(const std::unique_ptr<Shape> &shape) const;
};

class ShapeComparator {
public:
  size_t operator()(
    const std::unique_ptr<Shape> &a,
    const std::unique_ptr<Shape> &b) const;
};

typedef
  std::unordered_set<std::unique_ptr<Shape>, ShapeHasher, ShapeComparator>
  ShapeSet;

TriMesh ExploreShapes();

#endif

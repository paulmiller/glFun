#include "explore_shapes.h"

#include "scoped_timer.h"
#include "memory_usage.h"

#include "xxhash.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

namespace {

const int VolumeSize = 32;
const int MaxRounds = 6;

enum class UnaryOp {
  SweepX,
  RotateX,
  RotateY,
  RotateZ,
};

enum class BinaryOp {
  Union,
  Intersect,
  Subtract,
};

std::vector<UnaryOp> IterableUnaryOps = {
  UnaryOp::SweepX,
  UnaryOp::RotateX,
  UnaryOp::RotateY,
  UnaryOp::RotateZ,
};

std::vector<BinaryOp> IterableBinaryOps = {
  BinaryOp::Union,
  BinaryOp::Intersect,
  BinaryOp::Subtract
};

BoolVoxelVolume DoUnaryOp(UnaryOp op, const BoolVoxelVolume &voxels) {
  switch(op) {
  case UnaryOp::SweepX:
    return voxels.SweepX();
  case UnaryOp::RotateX:
    return voxels.RotateX();
  case UnaryOp::RotateY:
    return voxels.RotateY();
  case UnaryOp::RotateZ:
    return voxels.RotateZ();
  default:
    assert(0);
    return voxels;
  }
}

BoolVoxelVolume DoBinaryOp(BinaryOp op, const BoolVoxelVolume &a, const BoolVoxelVolume &b) {
  switch(op) {
  case BinaryOp::Union:
    return a.Union(b);
  case BinaryOp::Intersect:
    return a.Intersect(b);
  case BinaryOp::Subtract:
    return a.Subtract(b);
  default:
    assert(0);
    return a;
  }
}

} // namespace

size_t ShapeHasher::operator()(const std::unique_ptr<Shape> &shape) const {
  using VoxelWord = BoolVoxelVolume::VoxelWord;
  uint64_t hash;
  if(!shape->have_hash) {
    const std::vector<VoxelWord> &voxels = shape->voxels.GetVoxels();
    hash = XXH64(voxels.data(), voxels.size() * sizeof(VoxelWord), 0);
    shape->hash = hash;
    shape->have_hash = true;
  } else {
    hash = shape->hash;
  }
  return static_cast<size_t>(hash);
}

size_t ShapeComparator::operator()(
  const std::unique_ptr<Shape> &a,
  const std::unique_ptr<Shape> &b) const
{
  using VoxelWord = BoolVoxelVolume::VoxelWord;
  const std::vector<VoxelWord> &a_voxels = a->voxels.GetVoxels();
  const std::vector<VoxelWord> &b_voxels = b->voxels.GetVoxels();
  size_t a_size = a_voxels.size();
  assert(a_size == b_voxels.size());
  const VoxelWord *a_data = a_voxels.data();
  const VoxelWord *b_data = b_voxels.data();
  return memcmp(a_data, b_data, a_size * sizeof(VoxelWord)) == 0;
}

BoolVoxelVolume MakeSphere() {
  BoolVoxelVolume voxels(VolumeSize, VolumeSize, VolumeSize);
  for(int z = 0; z < VolumeSize; z++) {
    for(int y = 0; y < VolumeSize; y++) {
      for(int x = 0; x < VolumeSize; x++) {
        Vector3f v = voxels.CenterOf(x, y, z);
        if(v.x * v.x + v.y * v.y + v.z * v.z <= 1)
          voxels.Set(x,y,z);
      }
    }
  }
  return voxels;
}

TriMesh ExploreShapes() {
  ShapeSet shapes;
  ShapeSet new_shapes;

  shapes.insert(std::unique_ptr<Shape>(
    new Shape(MakeSphere(), 0)
  ));

  int rounds = 0;
  int repeats = 0;

  while(rounds < MaxRounds) {
    std::cout << "\nstart round " << rounds << '\n';
    PrintingScopedTimer round_timer(
      std::string("end round ") + std::to_string(rounds));

    for(const auto &shape: shapes) {
      for(auto op: IterableUnaryOps) {
        std::unique_ptr<Shape> new_shape(
          new Shape(DoUnaryOp(op, shape->voxels), shape->generation + 1)
        );

        if(new_shape->voxels.IsEmpty())
          continue;

        bool unique = (shapes.find(new_shape) == shapes.end());
        if(unique)
          unique = new_shapes.insert(std::move(new_shape)).second;
        if(!unique)
          repeats++;
      }
      for(auto op: IterableBinaryOps) {
        for(const auto &shape2: shapes) {
          if(shape.get() == shape2.get())
            continue;

          std::unique_ptr<Shape> new_shape(
            new Shape(
              DoBinaryOp(op, shape->voxels, shape2->voxels),
              std::max(shape->generation, shape2->generation) + 1
            )
          );

          if(new_shape->voxels.IsEmpty())
            continue;

          bool unique = (shapes.find(new_shape) == shapes.end());
          if(unique)
            unique = new_shapes.insert(std::move(new_shape)).second;
          if(!unique)
            repeats++;
        }
      }
    }
    if(new_shapes.size() > 0) {
      shapes.merge(new_shapes);
      new_shapes.clear();
    } else {
      break;
    }
    rounds++;

    std::cout << "size=" << shapes.size() << ", repeats=" << repeats << '\n';
    PrintMemoryUsage();
  }

  std::cout << "ExploreShapes size=" << shapes.size()
    << " rounds=" << rounds << " repeats=" << repeats << '\n';

  return TriMesh();
  /*
  auto block_timer_accumulator = AccumulatingScopedTimer::MakeAccumulator();
  auto merge_timer_accumulator = AccumulatingScopedTimer::MakeAccumulator();
  TriMesh combined_mesh;
  {
    PrintingScopedTimer mesh_timer("ExploreShapes mesh");

    combined_mesh.verts.reserve(100000000);
    combined_mesh.normals.reserve(10000);
    combined_mesh.tris.reserve(100000000);

    int generation_counts[MaxRounds+1] = {};
    Matrix4x4f mesh_offset = Identity_Matrix4x4f;
    for(auto shape = shapes.begin(); shape != shapes.end(); ++shape) {
      //TriMesh shape_mesh = (*shape)->voxels.CreateBlockMesh();
      TriMesh shape_mesh;
      {
        AccumulatingScopedTimer block_timer(block_timer_accumulator);
        shape_mesh = (*shape)->voxels.CreateBlockMesh();
      }
      int generation = (*shape)->generation;
      assert(generation <= MaxRounds);
      mesh_offset(0,3) = generation * 2.5;
      mesh_offset(2,3) = generation_counts[generation]++ * 2.5;
      shape_mesh.Transform(mesh_offset);
      {
        AccumulatingScopedTimer merge_timer(merge_timer_accumulator);
        combined_mesh.Merge(shape_mesh);
      }
    }
  }

  std::cout
    << "ExploreShapes CreateBlockMesh " << *block_timer_accumulator
    << "\nExploreShapes Merge " << *merge_timer_accumulator << '\n';

  std::cout << "total verts=" << combined_mesh.verts.size()
    << " tris=" << combined_mesh.tris.size() << '\n';

  PrintMemoryUsage();

  return combined_mesh;
  */
}

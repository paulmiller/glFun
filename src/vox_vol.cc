#include "vox_vol.h"

#include <cassert>
#include <cstring>
#include <memory>
#include <tuple>
#include <unordered_map>

namespace {
  typedef std::tuple<int, int, int> XYZ; // identifies a voxel or vertex

  class XyzHasher {
  public:
    size_t operator()(XYZ xyz) const {
      constexpr int shift = std::numeric_limits<int>::digits / 3;

      int x = std::get<0>(xyz);
      int y = std::get<1>(xyz);
      int z = std::get<2>(xyz);
      return int_hash_(x ^ (y << shift) ^ (z << (2*shift)));
    }

  private:
    std::hash<int> int_hash_;
  };
}

VoxelVolume::VoxelVolume(int x_size, int y_size, int z_size) :
  x_min_(-1), x_max_(1), y_min_(-1), y_max_(1), z_min_(-1), z_max_(1),
  x_size_(x_size), y_size_(y_size), z_size_(z_size),
  voxels_(x_size * y_size * z_size)
{
  assert(x_size > 0);
  assert(y_size > 0);
  assert(z_size > 0);
}

VoxelVolume::VoxelVolume(const VoxelVolume &v) :
  x_min_(v.x_min_), x_max_(v.x_max_),
  y_min_(v.y_min_), y_max_(v.y_max_),
  z_min_(v.z_min_), z_max_(v.z_max_),
  x_size_(v.x_size_), y_size_(v.y_size_), z_size_(v.z_size_),
  voxels_(v.voxels_)/*, packed_voxels_(v.packed_voxels_)*/ {} // TODO

VoxelVolume::VoxelVolume(VoxelVolume &&v) :
  x_min_(v.x_min_), x_max_(v.x_max_),
  y_min_(v.y_min_), y_max_(v.y_max_),
  z_min_(v.z_min_), z_max_(v.z_max_),
  x_size_(v.x_size_), y_size_(v.y_size_), z_size_(v.z_size_),
  voxels_(std::move(v.voxels_)), packed_voxels_(std::move(v.packed_voxels_)) {}

const std::vector<uint64_t>& VoxelVolume::GetPacked() {
  if(packed_voxels_.empty())
    Pack();
  return packed_voxels_;
}

float VoxelVolume::voxSizeX() { return (x_max_ - x_min_) / x_size_; }
float VoxelVolume::voxSizeY() { return (y_max_ - y_min_) / y_size_; }
float VoxelVolume::voxSizeZ() { return (z_max_ - z_min_) / z_size_; }

Vector3f VoxelVolume::CenterOf(int x, int y, int z) {
  return Vector3f {
    x_min_ + (x + 0.5f) * voxSizeX(),
    y_min_ + (y + 0.5f) * voxSizeY(),
    z_min_ + (z + 0.5f) * voxSizeZ()
  };
}

bool VoxelVolume::IsEmpty() const {
  // reinterpret_cast<uint64_t*>(vector.data()) should work
  static_assert(__STDCPP_DEFAULT_NEW_ALIGNMENT__ >= alignof(uint64_t));

  // TODO: check packed_voxels_ instead, if available?
  size_t num_bytes = voxels_.size();
  const char *bytes = voxels_.data();

  // for speed, check bytes in blocks of uint64_t at a time
  size_t num_blocks = num_bytes / sizeof(uint64_t);
  const uint64_t *blocks = reinterpret_cast<const uint64_t*>(bytes);
  for(size_t i = 0; i < num_blocks; i++) {
    if(blocks[i])
      return false;
  }

  // check any remaining bytes that don't fill up a uint64_t
  size_t num_checked_bytes = num_blocks * sizeof(uint64_t);
  size_t num_remaining_bytes = num_bytes % sizeof(uint64_t);
  for(size_t i = 0; i < num_remaining_bytes; i++) {
    if(bytes[num_checked_bytes + i])
      return false;
  }

  return true;
}

void VoxelVolume::Fill(char value) {
  memset(voxels_.data(), value, voxels_.size() * sizeof(char));
}

void VoxelVolume::SweepX() {
  const int y_stride = x_size_;
  char *start_voxel = voxels_.data();
  for(int z = 0; z < z_size_; z++) {
    for(int y = 0; y < y_size_; y++) {
      char *voxel = start_voxel;
      for(int x = 0; x < x_size_; x++) {
        if(*voxel) {
          memset(start_voxel, 1, x_size_ * sizeof(char));
          break;
        }
        voxel++;
      }
      start_voxel += y_stride;
    }
  }
}

void VoxelVolume::SweepY() {
  const int y_stride = x_size_;
  const int z_stride = x_size_ * y_size_;
  char *start_voxel = voxels_.data();
  for(int z = 0; z < z_size_; z++) {
    for(int x = 0; x < x_size_; x++) {
      bool found_nonzero = false;
      char *voxel = start_voxel;
      for(int y = 0; y < y_size_; y++) {
        if(*voxel) {
          found_nonzero = true;
          break;
        }
        voxel += y_stride;
      }
      if(found_nonzero) {
        voxel = start_voxel;
        for(int z = 0; z < z_size_; z++) {
          *voxel = 1;
          voxel += y_stride;
        }
      }
      start_voxel++;
    }
    // start_voxel is now at (x,y+1,z); reset it to (x,y,z+1)
    start_voxel += z_stride - y_stride;
  }
}

void VoxelVolume::SweepZ() {
  const int z_stride = x_size_ * y_size_;
  char *start_voxel = voxels_.data();
  for(int y = 0; y < y_size_; y++) {
    for(int x = 0; x < x_size_; x++) {
      bool found_nonzero = false;
      char *voxel = start_voxel;
      for(int z = 0; z < z_size_; z++) {
        if(*voxel) {
          found_nonzero = true;
          break;
        }
        voxel += z_stride;
      }
      if(found_nonzero) {
        voxel = start_voxel;
        for(int z = 0; z < z_size_; z++) {
          *voxel = 1;
          voxel += z_stride;
        }
      }
      start_voxel++;
    }
  }
}

VoxelVolume VoxelVolume::RotateX() const {
  assert(y_size_ == z_size_);

  const int y_stride = x_size_;
  const int z_stride = x_size_ * y_size_;

  VoxelVolume rotated(x_size_, y_size_, z_size_);
  const char *voxel = voxels_.data();
  for(int z = 0; z < z_size_; z++) {
    char *rotated_voxel = &rotated.at(0, y_size_ - 1 - z, 0);
    for(int y = 0; y < y_size_; y++) {
      memcpy(rotated_voxel, voxel, x_size_ * sizeof(char));
      voxel += y_stride;
      rotated_voxel += z_stride;
    }
  }
  return rotated;
}

VoxelVolume VoxelVolume::RotateY() const {
  assert(x_size_ == z_size_);

  const int z_stride = x_size_ * y_size_;

  VoxelVolume rotated(x_size_, y_size_, z_size_);
  const char *voxel = voxels_.data();
  for(int z = 0; z < z_size_; z++) {
    for(int y = 0; y < y_size_; y++) {
      char *rotated_voxel = &rotated.at(z, y, z_size_ - 1);
      for(int x = 0; x < x_size_; x++) {
        *rotated_voxel = *voxel;
        voxel++;
        rotated_voxel -= z_stride;
      }
    }
  }
  return rotated;
}

VoxelVolume VoxelVolume::RotateZ() const {
  assert(x_size_ == y_size_);

  const int y_stride = x_size_;

  VoxelVolume rotated(x_size_, y_size_, z_size_);
  const char *voxel = voxels_.data();
  for(int z = 0; z < z_size_; z++) {
    for(int y = 0; y < y_size_; y++) {
      char *rotated_voxel = &rotated.at(x_size_ - 1 - y, 0, z);
      for(int x = 0; x < x_size_; x++) {
        *rotated_voxel = *voxel;
        voxel++;
        rotated_voxel += y_stride;
      }
    }
  }
  return rotated;
}

void VoxelVolume::Union(const VoxelVolume &o) {
  assert(x_size_ == o.x_size_);
  assert(y_size_ == o.y_size_);
  assert(z_size_ == o.z_size_);

  size_t size = voxels_.size();
  assert(size == o.voxels_.size());

  char *voxel = voxels_.data();
  const char *o_voxel = o.voxels_.data();

  for(size_t i = 0; i < size; i++) {
    *voxel = *voxel || *o_voxel;
    voxel++;
    o_voxel++;
  }
}

void VoxelVolume::Intersect(const VoxelVolume &o) {
  assert(x_size_ == o.x_size_);
  assert(y_size_ == o.y_size_);
  assert(z_size_ == o.z_size_);

  size_t size = voxels_.size();
  assert(size == o.voxels_.size());

  char *voxel = voxels_.data();
  const char *o_voxel = o.voxels_.data();

  for(size_t i = 0; i < size; i++) {
    *voxel = *voxel && *o_voxel;
    voxel++;
    o_voxel++;
  }
}

void VoxelVolume::Subtract(const VoxelVolume &o) {
  assert(x_size_ == o.x_size_);
  assert(y_size_ == o.y_size_);
  assert(z_size_ == o.z_size_);

  size_t size = voxels_.size();
  assert(size == o.voxels_.size());

  char *voxel = voxels_.data();
  const char *o_voxel = o.voxels_.data();

  for(size_t i = 0; i < size; i++) {
    *voxel = *voxel && !*o_voxel;
    voxel++;
    o_voxel++;
  }
}

TriMesh VoxelVolume::CreateBlockMesh() {
  float voxel_x_size = voxSizeX();
  float voxel_y_size = voxSizeY();
  float voxel_z_size = voxSizeZ();

  TriMesh mesh;
  mesh.normals.reserve(6);
  mesh.normals.push_back( UnitX_Vector3f); constexpr int x_pos_normal = 0;
  mesh.normals.push_back(-UnitX_Vector3f); constexpr int x_neg_normal = 1;
  mesh.normals.push_back( UnitY_Vector3f); constexpr int y_pos_normal = 2;
  mesh.normals.push_back(-UnitY_Vector3f); constexpr int y_neg_normal = 3;
  mesh.normals.push_back( UnitZ_Vector3f); constexpr int z_pos_normal = 4;
  mesh.normals.push_back(-UnitZ_Vector3f); constexpr int z_neg_normal = 5;

  // map from a vertex's XYZ address within the volume to that vertex's offset
  // within mesh.verts
  std::unordered_map<XYZ, int, XyzHasher> vert_offsets;

  auto getVert = [=, &mesh, &vert_offsets](int x, int y, int z) -> int {
    auto key = std::make_tuple(x, y, z);
    auto found = vert_offsets.find(key);
    int offset;
    if(found == vert_offsets.end()) {
      mesh.verts.push_back(Vector3f {
        x_min_ + x * voxel_x_size,
        y_min_ + y * voxel_y_size,
        z_min_ + z * voxel_z_size
      });
      offset = mesh.verts.size() - 1;
      vert_offsets[key] = offset;
    } else {
      offset = found->second;
    }
    return offset;
  };

  // access voxels with pointer math because "voxels_[i]" is too slow
  char *voxel = voxels_.data();

  // If voxel[i] is the voxel at (x,y,z), then voxel[i+x_stride] is the voxel at
  // (x+1,y,z); voxel[i-x_stride] is at (x-1,y,z); voxel[i+y_stride] is at
  // (x,y+1,z); etc. This finds neighboring voxels faster than calling at().
  const int x_stride = 1;
  const int y_stride = x_size_;
  const int z_stride = x_size_ * y_size_;

  for(int z = 0; z < z_size_; z++) {
    for(int y = 0; y < y_size_; y++) {
      for(int x = 0; x < x_size_; x++) {
        if(*voxel) {
          if(x == 0 || !voxel[-x_stride]) {
            // create x_neg face
            int a = getVert(x, y, z);
            int b = getVert(x, y, z+1);
            int c = getVert(x, y+1, z);
            int d = getVert(x, y+1, z+1);
            mesh.tris.emplace_back(
              a, b, c,
              x_neg_normal, x_neg_normal, x_neg_normal,
              -1, -1, -1);
            mesh.tris.emplace_back(
              b, d, c,
              x_neg_normal, x_neg_normal, x_neg_normal,
              -1, -1, -1);
          }

          if(x == x_size_-1 || !voxel[x_stride]) {
            // create x_pos face
            int a = getVert(x+1, y, z);
            int b = getVert(x+1, y+1, z);
            int c = getVert(x+1, y, z+1);
            int d = getVert(x+1, y+1, z+1);
            mesh.tris.emplace_back(
              a, b, c,
              x_pos_normal, x_pos_normal, x_pos_normal,
              -1, -1, -1);
            mesh.tris.emplace_back(
              b, d, c,
              x_pos_normal, x_pos_normal, x_pos_normal,
              -1, -1, -1);
          }

          if(y == 0 || !voxel[-y_stride]) {
            // create y_neg face
            int a = getVert(x, y, z);
            int b = getVert(x+1, y, z);
            int c = getVert(x, y, z+1);
            int d = getVert(x+1, y, z+1);
            mesh.tris.emplace_back(
              a, b, c,
              y_neg_normal, y_neg_normal, y_neg_normal,
              -1, -1, -1);
            mesh.tris.emplace_back(
              b, d, c,
              y_neg_normal, y_neg_normal, y_neg_normal,
              -1, -1, -1);
          }

          if(y == y_size_-1 || !voxel[y_stride]) {
            // create y_pos face
            int a = getVert(x, y+1, z);
            int b = getVert(x, y+1, z+1);
            int c = getVert(x+1, y+1, z);
            int d = getVert(x+1, y+1, z+1);
            mesh.tris.emplace_back(
              a, b, c,
              y_pos_normal, y_pos_normal, y_pos_normal,
              -1, -1, -1);
            mesh.tris.emplace_back(
              b, d, c,
              y_pos_normal, y_pos_normal, y_pos_normal,
              -1, -1, -1);
          }

          if(z == 0 || !voxel[-z_stride]) {
            // create z_neg face
            int a = getVert(x, y, z);
            int b = getVert(x, y+1, z);
            int c = getVert(x+1, y, z);
            int d = getVert(x+1, y+1, z);
            mesh.tris.emplace_back(
              a, b, c,
              z_neg_normal, z_neg_normal, z_neg_normal,
              -1, -1, -1);
            mesh.tris.emplace_back(
              b, d, c,
              z_neg_normal, z_neg_normal, z_neg_normal,
              -1, -1, -1);
          }

          if(z == z_size_-1 || !voxel[z_stride]) {
            // create z_pos face
            int a = getVert(x, y, z+1);
            int b = getVert(x+1, y, z+1);
            int c = getVert(x, y+1, z+1);
            int d = getVert(x+1, y+1, z+1);
            mesh.tris.emplace_back(
              a, b, c,
              z_pos_normal, z_pos_normal, z_pos_normal,
              -1, -1, -1);
            mesh.tris.emplace_back(
              b, d, c,
              z_pos_normal, z_pos_normal, z_pos_normal,
              -1, -1, -1);
          }
        }

        voxel++;
      }
    }
  }

  return mesh;
}

void VoxelVolume::Pack() {
  assert(packed_voxels_.empty()); // meant to be called only once

  size_t size = voxels_.size();
  size_t full_words = size / 64;
  size_t remainder = size % 64;

  packed_voxels_.reserve(full_words + bool(remainder));

  char *voxel = voxels_.data();

  for(size_t i = 0; i < full_words; i++) {
    uint64_t packed_bits = 0;
    for(uint64_t j = 0; j < 64; j++) {
      packed_bits |= uint64_t(*voxel) << j;
      voxel++;
    }
    packed_voxels_.push_back(packed_bits);
  }

  uint64_t packed_bits = 0;
  for(uint64_t j = 0; j < remainder; j++) {
    packed_bits |= uint64_t(*voxel) << j;
    voxel++;
  }
  packed_voxels_.push_back(packed_bits);
}

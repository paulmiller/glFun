#include "vox_vol.h"

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

BoolVoxVol::BoolVoxVol(int x_size, int y_size, int z_size) :
  x_min_(-1), x_max_(1), y_min_(-1), y_max_(1), z_min_(-1), z_max_(1),
  x_size_(x_size), y_size_(y_size), z_size_(z_size),
  vox_(x_size * y_size * z_size)
{
  assert(x_size > 0);
  assert(y_size > 0);
  assert(z_size > 0);
}

float BoolVoxVol::voxSizeX() { return (x_max_ - x_min_) / x_size_; }
float BoolVoxVol::voxSizeY() { return (y_max_ - y_min_) / y_size_; }
float BoolVoxVol::voxSizeZ() { return (z_max_ - z_min_) / z_size_; }

Vec3 BoolVoxVol::centerOf(int x, int y, int z) {
  return Vec3(
    x_min_ + (x + 0.5f) * voxSizeX(),
    y_min_ + (y + 0.5f) * voxSizeY(),
    z_min_ + (z + 0.5f) * voxSizeZ()
  );
}

TriMesh BoolVoxVol::createBlockMesh() {
  float voxel_x_size = voxSizeX();
  float voxel_y_size = voxSizeY();
  float voxel_z_size = voxSizeZ();

  TriMesh mesh;
  mesh.normals.reserve(6);
  mesh.normals.push_back( Vec3::UNIT_X); constexpr int x_pos_normal = 0;
  mesh.normals.push_back(-Vec3::UNIT_X); constexpr int x_neg_normal = 1;
  mesh.normals.push_back( Vec3::UNIT_Y); constexpr int y_pos_normal = 2;
  mesh.normals.push_back(-Vec3::UNIT_Y); constexpr int y_neg_normal = 3;
  mesh.normals.push_back( Vec3::UNIT_Z); constexpr int z_pos_normal = 4;
  mesh.normals.push_back(-Vec3::UNIT_Z); constexpr int z_neg_normal = 5;

  // map from a vertex's XYZ address within the volume to that vertex's offset
  // within mesh.verts
  std::unordered_map<XYZ, int, XyzHasher> vert_offsets;

  auto getVert = [=, &mesh, &vert_offsets](int x, int y, int z) -> int {
    auto key = std::make_tuple(x, y, z);
    auto found = vert_offsets.find(key);
    int offset;
    if(found == vert_offsets.end()) {
      mesh.verts.emplace_back(
        x_min_ + x * voxel_x_size,
        y_min_ + y * voxel_y_size,
        z_min_ + z * voxel_z_size
      );
      offset = mesh.verts.size() - 1;
      vert_offsets[key] = offset;
    } else {
      offset = found->second;
    }
    return offset;
  };

  for(int z = 0; z < z_size_; z++) {
    for(int y = 0; y < y_size_; y++) {
      for(int x = 0; x < x_size_; x++) {
        if(!at(x, y, z)) continue;

        if(x == 0 || !at(x-1, y, z)) {
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

        if(x == x_size_-1 || !at(x+1, y, z)) {
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

        if(y == 0 || !at(x, y-1, z)) {
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

        if(y == y_size_-1 || !at(x, y+1, z)) {
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

        if(z == 0 || !at(x, y, z-1)) {
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

        if(z == z_size_-1 || !at(x, y, z+1)) {
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
    }
  }

  return mesh;
}
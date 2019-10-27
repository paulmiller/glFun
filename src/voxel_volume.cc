#include "voxel_volume.h"

#include <cassert>
#include <memory>

VoxelVolume::VoxelVolume(int x_size, int y_size, int z_size) :
  x_min_(-1), y_min_(-1), z_min_(-1),
  x_max_( 1), y_max_( 1), z_max_( 1),
  x_size_(x_size), y_size_(y_size), z_size_(z_size)
{
  assert(x_size > 0);
  assert(y_size > 0);
  assert(z_size > 0);
}

Vector3f VoxelVolume::CenterOf(int x, int y, int z) const {
  return Vector3f {
    x_min_ + (x + 0.5f) * VoxelXSize(),
    y_min_ + (y + 0.5f) * VoxelYSize(),
    z_min_ + (z + 0.5f) * VoxelZSize()
  };
}

TriMesh VoxelVolume::CreateBlockMesh() {
  float voxel_x_size = VoxelXSize();
  float voxel_y_size = VoxelYSize();
  float voxel_z_size = VoxelZSize();

  TriMesh mesh;
  mesh.normals.reserve(6);
  mesh.normals.push_back( UnitX_Vector3f); constexpr int x_pos_normal = 0;
  mesh.normals.push_back(-UnitX_Vector3f); constexpr int x_neg_normal = 1;
  mesh.normals.push_back( UnitY_Vector3f); constexpr int y_pos_normal = 2;
  mesh.normals.push_back(-UnitY_Vector3f); constexpr int y_neg_normal = 3;
  mesh.normals.push_back( UnitZ_Vector3f); constexpr int z_pos_normal = 4;
  mesh.normals.push_back(-UnitZ_Vector3f); constexpr int z_neg_normal = 5;

  // "vert_offsets" holds a 3D array mapping each vertex's XYZ address within
  // the volume to that vertex's offset within mesh.verts. An offset of -1 means
  // that vertex hasn't been created in mesh.verts. Since there are vertices
  // surrounding every voxel, "vert_offsets" is bigger by 1 in every dimension
  // than the grid of voxels.
  int verts_x_size = x_size_ + 1;
  int verts_y_size = y_size_ + 1;
  int verts_z_size = z_size_ + 1;
  int verts_size = verts_x_size * verts_y_size * verts_z_size;
  auto vert_offsets = std::make_unique<int[]>(verts_size);
  for(int i = 0; i < verts_size; i++)
    vert_offsets[i] = -1;

  // "getVert" looks up a value in "vert_offsets", creating a new vertex if
  // there is none
  auto getVert = [=, &mesh, &vert_offsets](int x, int y, int z) -> int {
    int i = ((z * verts_y_size) + y) * verts_x_size + x;
    assert(i < verts_size);
    int *offset = vert_offsets.get() + i;
    if(*offset == -1) {
      mesh.verts.push_back(Vector3f {
        x_min_ + x * voxel_x_size,
        y_min_ + y * voxel_y_size,
        z_min_ + z * voxel_z_size
      });
      *offset = mesh.verts.size() - 1;
    }
    return *offset;
  };

  for(int z = 0; z < z_size_; z++) {
    for(int y = 0; y < y_size_; y++) {
      for(int x = 0; x < x_size_; x++) {
        if(GetBool(x,y,z)) {
          Color c = GetColor(x,y,z); // TODO apply color

          if(x == 0 || !GetBool(x-1,y,z)) {
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

          if(x == x_size_-1 || !GetBool(x+1,y,z)) {
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

          if(y == 0 || !GetBool(x,y-1,z)) {
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

          if(y == y_size_-1 || !GetBool(x,y+1,z)) {
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

          if(z == 0 || !GetBool(x,y,z-1)) {
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

          if(z == z_size_-1 || !GetBool(x,y,z+1)) {
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
  }

  return mesh;
}


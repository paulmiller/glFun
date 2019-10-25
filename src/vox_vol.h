#ifndef VOX_VOL_H
#define VOX_VOL_H

#include "math/util.h"
#include "math/vector.h"
#include "mesh.h"

#include <cassert>
#include <ostream>

/*
A volume of boolean voxels

A 2x1x1 volume (x_size_ = 2, y_size_ = 1, z_size_ = 1) is addressed like so:

       vertex >  * - - - - - * - - - - - *  < vertex
       0,1,1    /           /           /|    2,1,1
               /           /           / |
    vertex >  * - - - - - * - - - - - *  |
    0,0,1     |           |           |  |
              |      .    |      .    |  |
              |           |           |  *  < vertex
              |   voxel   |   voxel   | /     2,1,0
              |   0,0,0   |   1,0,0   |/
              * - - - - - * - - - - - *
              ^           ^           ^
            vertex      vertex      vertex
            0,0,0       1,0,0       2,0,0

The voxels evenly sample a volume defined by the x/y/z min/max member variables:

                 * - - - - - - - - - - - *
                /                       /|
               /                       / |
  z_max_  --  * - - - - - - - - - - - *  |
              |                       |  |        Z
              |                       |  |        |
              |                       |  *        |  Y
              |                       | /         | /
              |                       |/          |/
  z_min_  --  * - - - - - - - - - - - *           * - - - X

              |                       |

            x_min_                  x_max_

Thus the vertex addressed as (0,0,0) is at position (x_min_, y_min_, z_min_).

The opposite vertex (x_size_, y_size_, z_size_) is at (x_max_, y_max_, z_max_).

The vertex (1,0,0) is at (x_min_ + X, y_min_, z_min_), where X is the x-axis
length of a single voxel: X = (x_max_ - x_min_) / x_size_.

The voxels are packed into VoxelWords, 1 bit per voxel. To simplify the math,
x_size_ must be a multiple of the number of bits per VoxelWord. That way, each
row of x_size_ voxels fits neatly into a whole number of VoxelWords.

Suppose VoxelWord = uint8_t. Then VoxelsPerWord = 8; BitIndexBits = 3;
BitIndexMask = 0b111. voxels_[0] contains voxels 0-7; voxels_[1] contains
voxels 8-15; etc. Suppose we want voxel 20. 20 = 0b10100. The 3 least
significant bits, 0b100, select a bit within a given uint8_t word. The other
bits, 0b10, select a word within voxels_. So to get voxel 20, we do
(voxels_[20 >> BitIndexBits] >> (20 & BitIndexMask)) & 1.
*/
class VoxelVolume {
public:
  using VoxelWord = uint32_t;
  static constexpr int VoxelsPerWord = std::numeric_limits<VoxelWord>::digits;
  static constexpr int BitIndexBits = Log<2>(VoxelsPerWord);
  static constexpr VoxelWord BitIndexMask = VoxelsPerWord - 1;

  VoxelVolume(int x_size, int y_size, int z_size);

  int XSize() const { return x_size_; }
  int YSize() const { return y_size_; }
  int ZSize() const { return z_size_; }

  // get the voxel at the given x,y,z address
  bool Get(int x, int y, int z) const {
    int index = VoxelIndex(x, y, z);
    return (voxels_[index >> BitIndexBits] >> (index & BitIndexMask)) & 1;
  }

  // set a voxel to 1
  void Set(int x, int y, int z) {
    int index = VoxelIndex(x, y, z);
    VoxelWord &word = voxels_[index >> BitIndexBits];
    VoxelWord bit = VoxelWord(1) << (index & BitIndexMask);
    word = (word & ~bit) | bit;
  }

  // dimensions of an individual voxel
  float VoxXSize() const { return (x_max_ - x_min_) / x_size_; }
  float VoxYSize() const { return (y_max_ - y_min_) / y_size_; }
  float VoxZSize() const { return (z_max_ - z_min_) / z_size_; }

  // get the position of the center of the voxel at the given x,y,z address
  Vector3f CenterOf(int x, int y, int z) const;

  bool IsEmpty() const;

  const std::vector<VoxelWord>& GetVoxels() { return voxels_; }

  VoxelVolume SweepX() const;

  VoxelVolume RotateX() const; // quarter rotation around X-axis
  VoxelVolume RotateY() const;
  VoxelVolume RotateZ() const;

  VoxelVolume Union(const VoxelVolume&) const;
  VoxelVolume Intersect(const VoxelVolume&) const;
  VoxelVolume Subtract(const VoxelVolume&) const;

  TriMesh CreateBlockMesh();

private:
  int VoxelIndex(int x, int y, int z) const {
    assert(x >= 0); assert(x < x_size_);
    assert(y >= 0); assert(y < y_size_);
    assert(z >= 0); assert(z < z_size_);
    return (z * y_size_ + y) * x_size_ + x;
  }

  float x_min_, y_min_, z_min_, x_max_, y_max_, z_max_; // volume boundaries
  int x_size_, y_size_, z_size_; // number of voxels
  int x_words_; // size in VoxelWords of each row of x_size_ voxels

  // voxels, in z-major order. 1 voxel = 1 bit. not using vector<bool> because
  // it doesn't support data()
  std::vector<VoxelWord> voxels_;
};

std::ostream& operator<<(std::ostream&, const VoxelVolume&);

#endif

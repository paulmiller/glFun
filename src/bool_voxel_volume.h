#ifndef BOOL_VOXEL_VOLUME_H
#define BOOL_VOXEL_VOLUME_H

#include "math/util.h"
#include "math/vector.h"
#include "voxel_volume.h"

#include <cassert>
#include <ostream>

/*
A VoxelVolume where each voxel is a bool value.

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
class BoolVoxelVolume : public VoxelVolume {
public:
  using VoxelWord = uint32_t;
  static constexpr int VoxelsPerWord = std::numeric_limits<VoxelWord>::digits;
  static_assert(IsPowerOf2(VoxelsPerWord));
  static constexpr int BitIndexBits = Log<2>(VoxelsPerWord);
  static constexpr VoxelWord BitIndexMask = VoxelsPerWord - 1;

  BoolVoxelVolume(int x_size, int y_size, int z_size);
  virtual ~BoolVoxelVolume() {}

  // get the voxel at the given x,y,z address (prefer this non-virtual method
  // over GetBool, when possible, for performance)
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

  bool GetBool(int x, int y, int z) const override;
  Color GetColor(int x, int y, int z) const override;

  bool IsEmpty() const;

  const std::vector<VoxelWord>& GetVoxels() { return voxels_; }

  BoolVoxelVolume SweepX() const;

  BoolVoxelVolume RotateX() const; // quarter rotation around X-axis
  BoolVoxelVolume RotateY() const;
  BoolVoxelVolume RotateZ() const;

  BoolVoxelVolume Union(const BoolVoxelVolume&) const;
  BoolVoxelVolume Intersect(const BoolVoxelVolume&) const;
  BoolVoxelVolume Subtract(const BoolVoxelVolume&) const;

private:
  int x_words_; // size in VoxelWords of each row of x_size_ voxels

  // voxels, in z-major order. 1 voxel = 1 bit. not using vector<bool> because
  // it doesn't support data()
  std::vector<VoxelWord> voxels_;
};

std::ostream& operator<<(std::ostream&, const BoolVoxelVolume&);

#endif

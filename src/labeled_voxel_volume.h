#ifndef LABELED_VOXEL_VOLUME_H
#define LABELED_VOXEL_VOLUME_H

#include "voxel_volume.h"

#include <ostream>
#include <vector>

class LabeledVoxelVolume : public VoxelVolume {
public:
  using Voxel = uint16_t;
  using VoxelPairWord = uint32_t; // a word big enough to hold 2 Voxels
  using VoxelMaxWord = uint64_t; // a word for working on Voxels in parallel
  static constexpr int VoxelsPerMaxWord = sizeof(VoxelMaxWord) / sizeof(Voxel);

  LabeledVoxelVolume(int x_size, int y_size, int z_size);
  virtual ~LabeledVoxelVolume() {}

  Voxel Get(int x, int y, int z) const {
    return voxels_[VoxelIndex(x,y,z)];
  }

  void Set(int x, int y, int z, Voxel v) {
    voxels_[VoxelIndex(x,y,z)] = v;
  }

  bool GetBool(int x, int y, int z) const override;
  Color GetColor(int x, int y, int z) const override;

  bool IsEmpty() const;

  LabeledVoxelVolume RotateX() const;
  LabeledVoxelVolume RotateY() const;
  LabeledVoxelVolume RotateZ() const;

  // overlay another volume on this one, compare each pair of overlaid voxels,
  // and generate new labels representing each unique pairing
  void Merge(const LabeledVoxelVolume &overlay);

  void SweepXAndMerge();

private:
  const VoxelMaxWord *VoxelsAsMaxWords() const {
    return reinterpret_cast<const VoxelMaxWord*>(voxels_.data());
  }

  size_t VoxelMaxWordNum() const {
    return voxels_.size() / VoxelsPerMaxWord;
  }

  std::vector<Voxel> voxels_;
};

std::ostream& operator<<(std::ostream&, const LabeledVoxelVolume&);

#endif

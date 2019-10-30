#include "labeled_voxel_volume.h"

#include "math/util.h"

#include <cstring>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>

LabeledVoxelVolume::LabeledVoxelVolume(int x_size, int y_size, int z_size) :
  VoxelVolume(x_size, y_size, z_size),
  voxels_(x_size * y_size * z_size)
{
  assert(x_size % VoxelsPerMaxWord == 0);
}

bool LabeledVoxelVolume::GetBool(int x, int y, int z) const /*override*/ {
  return voxels_[VoxelIndex(x,y,z)];
}

Color LabeledVoxelVolume::GetColor(int x, int y, int z) const /*override*/ {
  Voxel voxel = voxels_[VoxelIndex(x,y,z)];

  // hash the value to pick a hue
  for(int i = 0; i < 3; i++) {
    voxel ^= voxel << 7;
    voxel ^= voxel >> 9;
    voxel ^= voxel << 8;
  }

  constexpr Voxel max_hash = std::numeric_limits<Voxel>::max();
  float hue = LinearMap_f(float(voxel), 0.0f, float(max_hash), 0.0f, 1.0f);

  bool tint = (x ^ y ^ z) & 1;
  float value = tint ? 0.875f : 1.0f;

  return ColorFromHsv(hue, 0.75f, value);
}

bool LabeledVoxelVolume::IsEmpty() const {
  size_t size = VoxelMaxWordNum();
  const VoxelMaxWord *data = VoxelsAsMaxWords();
  for(size_t i = 0; i < size; i++) {
    if(data[i])
      return false;
  }
  return true;
}

LabeledVoxelVolume LabeledVoxelVolume::RotateX() const {
  assert(y_size_ == z_size_);

  const int y_stride = x_size_;
  const int z_stride = x_size_ * y_size_;

  LabeledVoxelVolume rotated(x_size_, y_size_, z_size_);
  Voxel *dest_data = rotated.voxels_.data();
  const Voxel *source_row = voxels_.data();
  for(int z = 0; z < z_size_; z++) {
    Voxel *dest_row = dest_data + VoxelIndex(0, y_size_ - 1 - z, 0);
    for(int y = 0; y < y_size_; y++) {
      memcpy(dest_row, source_row, x_size_ * sizeof(Voxel));
      source_row += y_stride;
      dest_row += z_stride;
    }
  }
  return rotated;
}

LabeledVoxelVolume LabeledVoxelVolume::RotateY() const {
  assert(x_size_ == z_size_);

  const int z_stride = x_size_ * y_size_;

  LabeledVoxelVolume rotated(x_size_, y_size_, z_size_);
  Voxel *dest_data = rotated.voxels_.data();
  const Voxel *source_voxel = voxels_.data();
  for(int z = 0; z < z_size_; z++) {
    for(int y = 0; y < y_size_; y++) {
      Voxel *dest_voxel = dest_data + VoxelIndex(z, y, z_size_ - 1);
      for(int x = 0; x < x_size_; x++) {
        *dest_voxel = *source_voxel;
        source_voxel++;
        dest_voxel -= z_stride;
      }
    }
  }
  return rotated;
}

LabeledVoxelVolume LabeledVoxelVolume::RotateZ() const {
  assert(x_size_ == y_size_);

  const int y_stride = x_size_;

  LabeledVoxelVolume rotated(x_size_, y_size_, z_size_);
  Voxel *dest_data = rotated.voxels_.data();
  const Voxel *source_voxel = voxels_.data();
  for(int z = 0; z < z_size_; z++) {
    for(int y = 0; y < y_size_; y++) {
      Voxel *dest_voxel = dest_data + VoxelIndex(x_size_ - 1 - y, 0, z);
      for(int x = 0; x < x_size_; x++) {
        *dest_voxel = *source_voxel;
        source_voxel++;
        dest_voxel += y_stride;
      }
    }
  }
  return rotated;
}

void LabeledVoxelVolume::Merge(const LabeledVoxelVolume &overlay) {
  using VoxelPair = union {
    struct { Voxel a, b; };
    VoxelPairWord ab;
  };

  std::unordered_map<VoxelPairWord, Voxel> new_labels;
  new_labels.insert(std::pair<VoxelPairWord, Voxel>(0,0));
  Voxel next_label = 1;

  Voxel *a_voxel = voxels_.data();
  const Voxel *b_voxel = overlay.voxels_.data();
  for(int z = 0; z < z_size_; z++) {
    for(int y = 0; y < y_size_; y++) {
      for(int x = 0; x < x_size_; x++) {
        VoxelPair pair;
        pair.a = *a_voxel;
        pair.b = *b_voxel;
        auto new_label = new_labels.find(pair.ab);
        if(new_label == new_labels.end()) {
          new_label = new_labels.insert(std::pair(pair.ab, next_label)).first;
          next_label++;
        }
        *a_voxel = new_label->second;
        a_voxel++;
        b_voxel++;
      }
    }
  }

  std::cout << "LabeledVoxelVolume::Merge labels=" << next_label << '\n';
}

void LabeledVoxelVolume::SweepXAndMerge() {
  // map<set<int>, int> works but unordered_map<set<int>, int> doesn't
  std::map<std::set<Voxel>, std::map<Voxel,Voxel>> new_labels;

  // no need to relabel an empty row
  static const auto empty_row_set = std::set<Voxel>({0});
  new_labels.insert(std::make_pair(empty_row_set, std::map<Voxel,Voxel>()));

  Voxel next_label = 1;

  for(int z = 0; z < z_size_; z++) {
    for(int y = 0; y < y_size_; y++) {
      const auto row_start_iter = voxels_.begin() + VoxelIndex(0,y,z);
      const auto row_end_iter = row_start_iter + x_size_;
      std::set<Voxel> row_set(row_start_iter, row_end_iter);

      if(row_set == empty_row_set) continue;

      // get the mapping which says how to relabel this class of row
      auto row_remapping_iter = new_labels.find(row_set);
      if(row_remapping_iter == new_labels.end()) {
        // generate new labels for this class of row
        std::map<Voxel,Voxel> row_remapping;
        for(Voxel label: row_set) {
          row_remapping.insert(std::make_pair(label, next_label));
          next_label++;
        }

        row_remapping_iter = new_labels.insert(
          std::make_pair(row_set, std::move(row_remapping))
        ).first;
      }
      std::map<Voxel,Voxel> &row_remapping = row_remapping_iter->second;

      // relabel this row
      auto row_iter = row_start_iter;
      for(int x = 0; x < x_size_; x++) {
        *row_iter = row_remapping.at(*row_iter);
        row_iter++;
      }
    }
  }

  std::cout << "LabeledVoxelVolume::SweepXAndMerge labels=" << next_label
    << '\n';
}

std::ostream& operator<<(std::ostream &out, const LabeledVoxelVolume &v) {
  int max_width = 3; // TODO actually check labels for max(log10())
  const int z_size = v.XSize(), y_size = v.YSize(), x_size = v.XSize();
  out << "LabeledVoxelVolume("
    << x_size << ',' << y_size << ',' << z_size << ")\n";
  for(int z = 0; z < z_size; z++) {
    out << "  z=" << z << '\n';
    for(int y = 0; y < y_size; y++) {
      out << "    ";
      for(int x = 0; x < x_size; x++) {
        std::string str = std::to_string(unsigned(v.Get(x,y,z)));
        int padding = max_width - str.size();
        assert(0 <= padding); assert(padding < max_width);
        padding += (x > 0); // extra space between columns
        std::string padding_str(padding, ' ');
        std::cout << padding_str << str;
      }
      out << '\n';
    }
  }
  return out;
}

#include "bool_voxel_volume.h"

#include <cassert>
#include <cstring>

BoolVoxelVolume::BoolVoxelVolume(int x_size, int y_size, int z_size) :
  VoxelVolume(x_size, y_size, z_size),
  x_words_(x_size / VoxelsPerWord),
  voxels_(x_size * y_size * z_size / VoxelsPerWord)
{
  assert(x_size % VoxelsPerWord == 0);
}

bool BoolVoxelVolume::GetBool(int x, int y, int z) const /*override*/ {
  return Get(x,y,z);
}

Color BoolVoxelVolume::GetColor(int x, int y, int z) const /*override*/ {
  return Color::White;
}

bool BoolVoxelVolume::IsEmpty() const {
  size_t size = voxels_.size();
  const VoxelWord *data = voxels_.data();
  for(size_t i = 0; i < size; i++) {
    if(data[i])
      return false;
  }
  return true;
}

BoolVoxelVolume BoolVoxelVolume::SweepX() const {
  const int y_stride = x_words_;

  BoolVoxelVolume swept(x_size_, y_size_, z_size_);
  VoxelWord *dest_row = swept.voxels_.data();
  const VoxelWord *source_row = voxels_.data();

  for(int z = 0; z < z_size_; z++) {
    for(int y = 0; y < y_size_; y++) {
      const VoxelWord *word = source_row;
      for(int x = 0; x < x_words_; x++) {
        if(*word) {
          memset(dest_row, 0xff, x_words_ * sizeof(VoxelWord));
          break;
        }
        word++;
      }
      dest_row += y_stride;
      source_row += y_stride;
    }
  }

  return swept;
}

BoolVoxelVolume BoolVoxelVolume::RotateX() const {
  assert(y_size_ == z_size_);

  const int y_stride = x_words_;
  const int z_stride = x_words_ * y_size_;

  BoolVoxelVolume rotated(x_size_, y_size_, z_size_);
  VoxelWord *dest_data = rotated.voxels_.data();
  const VoxelWord *source_row = voxels_.data();

  for(int z = 0; z < z_size_; z++) {
    VoxelWord *dest_row = dest_data + (y_size_ - 1 - z) * y_stride;
    for(int y = 0; y < y_size_; y++) {
      memcpy(dest_row, source_row, x_words_ * sizeof(VoxelWord));
      dest_row += z_stride;
      source_row += y_stride;
    }
  }
  return rotated;
}

BoolVoxelVolume BoolVoxelVolume::RotateY() const {
  assert(x_size_ == z_size_);

  BoolVoxelVolume rotated(x_size_, y_size_, z_size_);
  for(int z = 0; z < z_size_; z++) {
    for(int y = 0; y < y_size_; y++) {
      for(int x = 0; x < x_size_; x++) {
        if(Get(x,y,z))
          rotated.Set(z, y, z_size_ - 1 - x);
      }
    }
  }
  return rotated;
}

BoolVoxelVolume BoolVoxelVolume::RotateZ() const {
  assert(x_size_ == y_size_);

  BoolVoxelVolume rotated(x_size_, y_size_, z_size_);
  for(int z = 0; z < z_size_; z++) {
    for(int y = 0; y < y_size_; y++) {
      for(int x = 0; x < x_size_; x++) {
        if(Get(x,y,z))
          rotated.Set(x_size_ - 1 - y, x, z);
      }
    }
  }
  return rotated;
}

// c = a | b
BoolVoxelVolume BoolVoxelVolume::Union(const BoolVoxelVolume &b) const {
  assert(x_size_ == b.x_size_);
  assert(y_size_ == b.y_size_);
  assert(z_size_ == b.z_size_);

  BoolVoxelVolume c(x_size_, y_size_, z_size_);

  const VoxelWord *a_words = voxels_.data();
  const VoxelWord *b_words = b.voxels_.data();
  VoxelWord *c_words = c.voxels_.data();

  size_t size = voxels_.size();
  assert(size == b.voxels_.size());
  assert(size == c.voxels_.size());

  for(size_t i = 0; i < size; i++)
    c_words[i] = a_words[i] | b_words[i];

  return c;
}

// c = a & b
BoolVoxelVolume BoolVoxelVolume::Intersect(const BoolVoxelVolume &b) const {
  assert(x_size_ == b.x_size_);
  assert(y_size_ == b.y_size_);
  assert(z_size_ == b.z_size_);

  BoolVoxelVolume c(x_size_, y_size_, z_size_);

  const VoxelWord *a_words = voxels_.data();
  const VoxelWord *b_words = b.voxels_.data();
  VoxelWord *c_words = c.voxels_.data();

  size_t size = voxels_.size();
  assert(size == b.voxels_.size());
  assert(size == c.voxels_.size());

  for(size_t i = 0; i < size; i++)
    c_words[i] = a_words[i] & b_words[i];

  return c;
}

// c = a & ~b
BoolVoxelVolume BoolVoxelVolume::Subtract(const BoolVoxelVolume &b) const {
  assert(x_size_ == b.x_size_);
  assert(y_size_ == b.y_size_);
  assert(z_size_ == b.z_size_);

  BoolVoxelVolume c(x_size_, y_size_, z_size_);

  const VoxelWord *a_words = voxels_.data();
  const VoxelWord *b_words = b.voxels_.data();
  VoxelWord *c_words = c.voxels_.data();

  size_t size = voxels_.size();
  assert(size == b.voxels_.size());
  assert(size == c.voxels_.size());

  for(size_t i = 0; i < size; i++)
    c_words[i] = a_words[i] & ~b_words[i];

  return c;
}

std::ostream& operator<<(std::ostream &out, const BoolVoxelVolume &v) {
  const int z_size = v.XSize(), y_size = v.YSize(), x_size = v.XSize();
  out << "BoolVoxelVolume("
    << x_size << ',' << y_size << ',' << z_size << ")\n";
  for(int z = 0; z < z_size; z++) {
    out << "  z=" << z << '\n';
    for(int y = 0; y < y_size; y++) {
      out << "    ";
      for(int x = 0; x < x_size; x++) {
        if(x) out << ' ';
        out << (v.Get(x,y,z) ? 'X' : '-');
      }
      out << '\n';
    }
  }
  return out;
}

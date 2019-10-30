#ifndef VOXEL_VOLUME_H
#define VOXEL_VOLUME_H

#include "color.h"
#include "mesh.h"

/*
A base class for a rectangular volume of voxels.

Subclasses define their own data type for each individual voxel, and how those
voxels are stored in memory. But all subclasses store voxels in z-major
order, so VoxelIndex() is applicable to all subclasses.

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

The vertex (1,0,0) is at (x_min_ + VoxelXSize(), y_min_, z_min_).
*/
class VoxelVolume {
public:
  VoxelVolume(int x_size, int y_size, int z_size);
  virtual ~VoxelVolume() {}

  // number of voxels
  int XSize() const { return x_size_; }
  int YSize() const { return y_size_; }
  int ZSize() const { return z_size_; }

  // dimensions of an individual voxel
  float VoxelXSize() const { return (x_max_ - x_min_) / x_size_; }
  float VoxelYSize() const { return (y_max_ - y_min_) / y_size_; }
  float VoxelZSize() const { return (z_max_ - z_min_) / z_size_; }

  // get the position of the center of the voxel at the given x,y,z address
  Vector3f CenterOf(int x, int y, int z) const;

  // Get bool and color representations of the voxel at the given x,y,z address.
  // How to represent a voxel as bool/color is up to each subclass.
  virtual bool GetBool(int x, int y, int z) const = 0;
  virtual Color GetColor(int x, int y, int z) const = 0;

  // Create a mesh according to GetBool and GetColor. Every voxel for which
  // GetBool returns true will be a solid block of color GetColor. Everywhere
  // that GetBool returns false will be empty space.
  TriMesh CreateBlockMesh();

protected:
  // given the x,y,z address of a voxel, return its index in a linear, z-major
  // array of all voxels.
  int VoxelIndex(int x, int y, int z) const {
    assert(x >= 0); assert(x < x_size_);
    assert(y >= 0); assert(y < y_size_);
    assert(z >= 0); assert(z < z_size_);
    return (z * y_size_ + y) * x_size_ + x;
  }

  float x_min_, y_min_, z_min_, x_max_, y_max_, z_max_; // volume boundaries
  int x_size_, y_size_, z_size_; // number of voxels
};

#endif

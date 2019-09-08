#ifndef VOX_VOL_H
#define VOX_VOL_H

#include "math3d.h"
#include "mesh.h"

#include <cassert>

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
*/
class BoolVoxVol {
public:
  BoolVoxVol(int x_size, int y_size, int z_size);

  // get the voxel at the given x,y,z address
  inline std::vector<bool>::reference at(int x, int y, int z) {
    assert(x >= 0); assert(x < x_size_);
    assert(y >= 0); assert(y < y_size_);
    assert(z >= 0); assert(z < z_size_);
    return vox_[(z * y_size_ + y) * x_size_ + x];
  }

  // dimensions of an individual voxel
  float voxSizeX();
  float voxSizeY();
  float voxSizeZ();

  // get the position of the center of the voxel at the given x,y,z address
  Vec3 centerOf(int x, int y, int z);

  TriMesh createBlockMesh();

private:
  float x_min_, x_max_, y_min_, y_max_, z_min_, z_max_; // volume boundaries
  int x_size_, y_size_, z_size_; // number of voxels
  std::vector<bool> vox_; // voxels, in z-major order
};

#endif

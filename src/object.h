#ifndef OBJECT_H
#define OBJECT_H

#include "math3d.h"

class Object {
public:
  Vec3 pos;
  Quat rot;

  Object();
  Object(const Object &src);

  Mat4 getTransform() const;
  void moveLocal(Vec3 move); // translate along local (rotated) axes
  void moveGlobal(Vec3 move); // translate along global axes
};

#endif

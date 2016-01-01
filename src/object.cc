#include "object.h"

Object::Object() : pos(Vec3::ZERO), rot(Quat::IDENTITY) {}
Object::Object(const Object &src) : pos(src.pos), rot(src.rot) {}

Mat4 Object::getTransform() const {
  return Mat4::translation(pos) * Mat4::rotation(rot);
}

void Object::moveLocal(Vec3 move) {
  pos += (Mat4::rotation(rot) * Vec4(move, 0)).dropW();
}

void Object::moveGlobal(Vec3 move) {
  pos += move;
}

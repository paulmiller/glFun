#include "camera.h"

#include "util.h"

#include <cassert>
#include <cmath>
#include <cstring> // for memset

Camera::Camera() {
  memset(this, 0, sizeof(Camera));
}

void Camera::setResolution(int widthPx, int heightPx) {
  assert(widthPx > 0);
  assert(heightPx > 0);
  mFrameWidthPx = widthPx;
  mFrameHeightPx = heightPx;
}

void Camera::setFrustum(
  float nearClip, float farClip, float horizFOV, float aspect
) {
  assert(nearClip > 0);
  assert(farClip > nearClip);
  assert(horizFOV > 0);
  assert(horizFOV < PI);
  assert(aspect > 0);

  float n = nearClip;
  float f = farClip;
  float r = n * tan(horizFOV / 2);
  float t = r / aspect;

  /*
  A matrix of the form:
    [ .  .  .  . ]
    [ .  .  .  . ]
    [ 0  0  a  b ]
    [ 0  0 -1  0 ]
  Will transform z like so:
    z' = (az+b)/(-z) = -a - b/z
  We want to choose a and b such that:
    z = -n => z' = 1 & z = -f => z' = -1
  So:
    -a - b/(-n) = 1  & -a - b/(-f) = -1
    -a + b/n = 1     & -a + b/f = -1
    -a = 1 - b/n     & -a = -1 - b/f

    1 - b/n = -1 - b/f
    nf - nfb/n = -nf - nfb/f
    nf - fb = -nf - nb
    2nf = fb - nb
    2nf = (f - n)b
    2nf / (f - n) = b

    -a - 2nf / (-n(f - n)) = 1
    -a + 2f / (f - n) = 1
    2f / (f - n) - 1 = a
  */

  mProj = Mat4(
    n/r,    0,           0,           0,
       0, n/t,           0,           0,
       0,   0, 2*f/(f-n)-1, 2*f*n/(f-n),
       0,   0,          -1,           0
  );

  mInvProj = Mat4(
    r/n,   0,             0,             0,
      0, t/n,             0,             0,
      0,   0,             0,            -1,
      0,   0, (f-n)/(2*f*n), (f+n)/(2*f*n)
  );
}

void Camera::look(const Vec3 &eye, const Vec3 &forward, const Vec3 &up) {
  // Camera-space basis vectors
  Vec3 b = -forward.unit(); // backwards
  Vec3 r = cross(up, b).unit(); // right
  Vec3 u = cross(b, r); // true up

  const Vec3 &e = eye;

  mView = Mat4(
    r.x, r.y, r.z, 0,
    u.x, u.y, u.z, 0,
    b.x, b.y, b.z, 0,
      0,   0,   0, 1
  ) * Mat4(
    1, 0, 0, -e.x,
    0, 1, 0, -e.y,
    0, 0, 1, -e.z,
    0, 0, 0,    1
  );

  mInvView = Mat4(
    r.x, u.x, b.x, e.x,
    r.y, u.y, b.y, e.y,
    r.z, u.z, b.z, e.z,
      0,   0,   0,   1
  );
}

void Camera::lookAt(const Vec3 &eye, const Vec3 &target, const Vec3 &up) {
  look(eye, target - eye, up);
  /*
  Vec3 e = eye;

  // Camera-space basis vectors
  Vec3 b = (e - target).unit(); // backwards
  Vec3 r = cross(up, b).unit(); // right
  Vec3 u = cross(b, r); // true up

  mView = Mat4(
    r.x, r.y, r.z, 0,
    u.x, u.y, u.z, 0,
    b.x, b.y, b.z, 0,
      0,   0,   0, 1
  ) * Mat4(
    1, 0, 0, -e.x,
    0, 1, 0, -e.y,
    0, 0, 1, -e.z,
    0, 0, 0,    1
  );

  mInvView = Mat4(
    r.x, u.x, b.x, e.x,
    r.y, u.y, b.y, e.y,
    r.z, u.z, b.z, e.z,
      0,   0,   0,   1
  );
  */
}

Mat4 Camera::getTransform() const {
  return mProj * mView;
}

Mat4 Camera::getInvTransform() const {
  return mInvView * mInvProj;
}

Ray Camera::pixelRay(int xPx, int yPx) const {
  float x = linearMap(xPx, -0.5f, (mFrameWidthPx  - 1) + 0.5f, -1.0f,  1.0f);
  float y = linearMap(yPx, -0.5f, (mFrameHeightPx - 1) + 0.5f,  1.0f, -1.0f);
  return getInvTransform() * Ray(Vec3(x, y, 1), -Vec3::UNIT_Z);
}

/*
void viewProjectArc(view_t *this, vect3_t *pt, int xClick, int yClick) {
  float x = map((float) xClick, 0, (float) this->mFrameWidthPx, -1, 1);
  float y = map((float) yClick, 0, (float) this->mFrameHeightPx, 1, -1);

  float xyLen = sqrt(x * x + y * y);

  if(xyLen > 1) {
    pt->x = x / xyLen;
    pt->y = y / xyLen;
    pt->z = 0;
  } else {
    pt->x = x;
    pt->y = y;
    pt->z = -sqrt(1 - xyLen * xyLen);
  }
}

void viewStartArc(view_t *this, int xClick, int yClick) {
  viewProjectArc(this, &this->arcStart, xClick, yClick);
  this->modelLast = this->modelCurrent;
  this->doingArc = 1;
}

void viewUpdateArc(view_t *this, int xClick, int yClick) {
  vect3_t arcEnd, axis;
  float theta;
  matrix4_t rotation;

  assert(this->doingArc);

  viewProjectArc(this, &arcEnd, xClick, yClick);

  cross(&axis, &this->arcStart, &arcEnd);
  mvMult(&axis, &this->modelLast, &axis, 0);
  normalize(&axis);

  theta = -acos(dot(&this->arcStart, &arcEnd)) * DRAG_FACTOR;
  setRotation(&rotation, theta, &axis);

  mult(&this->modelCurrent, &rotation, &this->modelLast);

  //this->modelCurrent.sq[0][0] *= this->scale;
  //this->modelCurrent.sq[1][1] *= this->scale;
  //this->modelCurrent.sq[2][2] *= this->scale;
}

void viewFinishArc(view_t *this) {
  this->doingArc = 0;
}

void viewCancelArc(view_t *this) {
  this->modelCurrent = this->modelLast;
  this->doingArc = 0;
}

int viewDoingArc(view_t *this) {
  return this->doingArc;
}
*/

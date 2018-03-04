#include "camera.h"

#include "util.h"

#include <cassert>
#include <cmath>

Camera::Camera() : frame_width_px_(0), frame_height_px_(0) {}

void Camera::setResolution(int width_px, int height_px) {
  assert(width_px > 0);
  assert(height_px > 0);
  frame_width_px_ = width_px;
  frame_height_px_ = height_px;
}

void Camera::setFrustum(
  float near_clip, float far_clip, float horiz_fov, float aspect
) {
  assert(near_clip > 0);
  assert(far_clip > near_clip);
  assert(horiz_fov > 0);
  assert(horiz_fov < PI_f);
  assert(aspect > 0);

  float n = near_clip;
  float f = far_clip;
  float r = n * tan(horiz_fov / 2);
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

  proj_ = Mat4(
    n/r,    0,           0,           0,
       0, n/t,           0,           0,
       0,   0, 2*f/(f-n)-1, 2*f*n/(f-n),
       0,   0,          -1,           0
  );

  proj_inverse_ = Mat4(
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

  view_ = Mat4(
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

  view_inverse_ = Mat4(
    r.x, u.x, b.x, e.x,
    r.y, u.y, b.y, e.y,
    r.z, u.z, b.z, e.z,
      0,   0,   0,   1
  );
}

void Camera::lookAt(const Vec3 &eye, const Vec3 &target, const Vec3 &up) {
  look(eye, target - eye, up);
}

Mat4 Camera::getTransform() const {
  return proj_ * view_;
}

Mat4 Camera::getInvTransform() const {
  return view_inverse_ * proj_inverse_;
}

void Camera::castPixel(int x_px, int y_px, Vec3 &near, Vec3 &far) const {
  Mat4 inv = getInvTransform();

  // Map pixel indices to screen coordinates
  float x = linearMap(x_px, -0.5f, frame_width_px_  - 0.5f, -1.0f,  1.0f);
  float y = linearMap(y_px, -0.5f, frame_height_px_ - 0.5f,  1.0f, -1.0f);

  // Points defining the ray, in viewing-volume-space
  Vec3 view_near(x, y,  1);
  Vec3 view_far (x, y, -1);

  // Points, in world-space
  near = (inv * Vec4(view_near, 1)).unHomogenize();
  far  = (inv * Vec4(view_far , 1)).unHomogenize();
}

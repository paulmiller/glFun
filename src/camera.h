#ifndef CAMERA_H
#define CAMERA_H

#include "math3d.h"

class Camera {
private:
  int frame_width_px_, frame_height_px_;
  Mat4 view_, view_inverse_, proj_, proj_inverse_;

public:
  Camera();
  void setResolution(int width_px, int height_px);
  void setFrustum(float near_clip, float far_clip, float horiz_fov, float aspect);
  void look(const Vec3 &eye, const Vec3 &forward, const Vec3 &up);
  void lookAt(const Vec3 &eye, const Vec3 &target, const Vec3 &up);
  // Get the matrix which transforms points from world-space into
  // viewing-volume-space
  Mat4 getTransform() const;
  // Get the matrix which transforms points from viewing-volume-space into
  // world-space
  Mat4 getInvTransform() const;
  // Cast a ray from a screen pixel, producing points on the near and far
  // clipping planes
  void castPixel(int x_px, int y_px, Vec3 &near, Vec3 &far) const;
};

#endif

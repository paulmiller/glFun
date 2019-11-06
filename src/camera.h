#ifndef CAMERA_H
#define CAMERA_H

#include "math/matrix.h"

class Camera {
private:
  int frame_width_px_, frame_height_px_;
  Matrix4x4f view_, view_inverse_, proj_, proj_inverse_;

public:
  Camera();

  int GetPxRows() const { return frame_height_px_; }
  int GetPxCols() const { return frame_width_px_; }

  void setResolution(int width_px, int height_px);
  void setFrustum(
    float near_clip, float far_clip, float horiz_fov, float aspect);
  void look(const Vector3f &eye, const Vector3f &forward, const Vector3f &up);
  void lookAt(const Vector3f &eye, const Vector3f &target, const Vector3f &up);
  // Get the matrix which transforms points from world-space into
  // viewing-volume-space
  Matrix4x4f getTransform() const;
  // Get the matrix which transforms points from viewing-volume-space into
  // world-space
  Matrix4x4f getInvTransform() const;
  // Cast a ray from a screen pixel, producing points on the near and far
  // clipping planes
  void castPixel(int x_px, int y_px, Vector3f &near, Vector3f &far) const;
};

#endif

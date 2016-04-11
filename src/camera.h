#ifndef CAMERA_H
#define CAMERA_H

#include "math3d.h"

class Camera {
private:
  int mFrameWidthPx, mFrameHeightPx;
  Mat4 mView, mInvView, mProj, mInvProj;

public:
  Camera();
  void setResolution(int widthPx, int heightPx);
  void setFrustum(float nearClip, float farClip, float horizFOV, float aspect);
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
  void castPixel(int xPx, int yPx, Vec3 &near, Vec3 &far) const;
};

#endif

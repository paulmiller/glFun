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
  Mat4 getTransform() const;
  Mat4 getInvTransform() const;
  Ray pixelRay(int xPx, int yPx) const;
};

#endif

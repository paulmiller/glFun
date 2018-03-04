#ifndef CAMERA_CONTROL_H
#define CAMERA_CONTROL_H

#include "camera.h"
#include "glfw_window.h"

class CameraControl : public GlfwWindow::Observer {
public:
  CameraControl(int width, int height);

  Camera *getCam();

  void onFramebufferSize(int width, int height) override;
  void onMouseButton(int button, int action, int mods) override;
  void onCursorPosition(double x, double y) override;

private:
  Camera cam_;

  double rotation_;
  double declination_;
  double distance_;

  bool dragging_;
  double drag_scale_;
  double drag_start_x_, drag_start_y_;
  double drag_end_x_, drag_end_y_;

  double draggedRotation();
  double draggedDeclination();
  void updateCamPos();
};

#endif

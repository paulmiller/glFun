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
  void onCursorPosition(double xpos, double ypos) override;

private:
  Camera cam;

  double rotation;
  double declination;
  double distance;

  bool dragging;
  double drag_scale;
  double drag_start_x, drag_start_y;
  double drag_end_x, drag_end_y;

  double draggedRotation();
  double draggedDeclination();
  void updateCamPos();
};

#endif

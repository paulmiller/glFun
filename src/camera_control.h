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
  bool dragging;
  double drag_start_x, drag_start_y;
};

#endif

#include "camera_control.h"

#include "util.h"

#include <iostream>

CameraControl::CameraControl(int width, int height) :
  dragging(false), drag_start_x(0), drag_start_y(0)
{
  onFramebufferSize(width, height);
}

Camera *CameraControl::getCam() {
  return &cam;
}

void CameraControl::onFramebufferSize(int width, int height) {
  float aspect = float(width) / float(height);
  float horizFov = PI_f / 2;
  cam.setResolution(width, height);
  cam.setFrustum(0.1f, 100.0f, horizFov, aspect);
}

void CameraControl::onMouseButton(int button, int action, int mods) {
  if(button != GLFW_MOUSE_BUTTON_RIGHT) return;
  if(action == GLFW_PRESS) {
    dragging = true;
    glfwGetCursorPos(glfwGetCurrentContext(), &drag_start_x, &drag_start_y);
    std::cout << "start drag " << drag_start_x << ' ' << drag_start_y << std::endl;
  } else {
    dragging = false;
    std::cout << "stop drag " << drag_start_x << ' ' << drag_start_y << std::endl;
    // TODO arcball controls
  }
}

void CameraControl::onCursorPosition(double xpos, double ypos) {
  std::cout << "cursor " << xpos << ' ' << ypos << std::endl;
}

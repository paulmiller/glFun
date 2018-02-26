#include "camera_control.h"

#include "util.h"

#include <algorithm>
#include <cmath>

CameraControl::CameraControl(int width, int height) :
  rotation(0), declination(0.0), distance(4.0),
  dragging(false), drag_scale(0.01),
  drag_start_x(0), drag_start_y(0), drag_end_x(0), drag_end_y(0)
{
  onFramebufferSize(width, height);
  updateCamPos();
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
  } else {
    dragging = false;
    rotation = draggedRotation();
    declination = draggedDeclination();
  }
}

void CameraControl::onCursorPosition(double x, double y) {
  if (dragging) {
    drag_end_x = x;
    drag_end_y = y;
    updateCamPos();
  }
}

double CameraControl::draggedRotation() {
  double dx = drag_end_x - drag_start_x;
  return rotation - dx * drag_scale;
}

double CameraControl::draggedDeclination() {
  double dy = drag_end_y - drag_start_y;
  return std::clamp(declination + dy * drag_scale, -PI_d/2, PI_d/2);
}

void CameraControl::updateCamPos() {
  double new_rotation    = dragging ? draggedRotation()    : rotation;
  double new_declination = dragging ? draggedDeclination() : declination;

  double cnd = cos(new_declination);
  double new_x = distance * sin(new_rotation) * cnd;
  double new_y = distance * sin(new_declination);
  double new_z = distance * cos(new_rotation) * cnd;

  cam.lookAt(Vec3(new_x, new_y, new_z), Vec3::ZERO, Vec3::UNIT_Y);
}

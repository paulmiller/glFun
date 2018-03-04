#include "camera_control.h"

#include "util.h"

#include <algorithm>
#include <cmath>

CameraControl::CameraControl(int width, int height) :
  rotation_(0), declination_(0.0), distance_(4.0),
  dragging_(false), drag_scale_(0.01),
  drag_start_x_(0), drag_start_y_(0), drag_end_x_(0), drag_end_y_(0)
{
  onFramebufferSize(width, height);
  updateCamPos();
}

Camera *CameraControl::getCam() {
  return &cam_;
}

void CameraControl::onFramebufferSize(int width, int height) {
  float aspect = float(width) / float(height);
  float horiz_fov = PI_f / 2;
  cam_.setResolution(width, height);
  cam_.setFrustum(0.1f, 100.0f, horiz_fov, aspect);
}

void CameraControl::onMouseButton(int button, int action, int mods) {
  if(button != GLFW_MOUSE_BUTTON_RIGHT) return;
  if(action == GLFW_PRESS) {
    dragging_ = true;
    glfwGetCursorPos(glfwGetCurrentContext(), &drag_start_x_, &drag_start_y_);
  } else {
    dragging_ = false;
    rotation_ = draggedRotation();
    declination_ = draggedDeclination();
  }
}

void CameraControl::onCursorPosition(double x, double y) {
  if (dragging_) {
    drag_end_x_ = x;
    drag_end_y_ = y;
    updateCamPos();
  }
}

double CameraControl::draggedRotation() {
  double dx = drag_end_x_ - drag_start_x_;
  return rotation_ - dx * drag_scale_;
}

double CameraControl::draggedDeclination() {
  double dy = drag_end_y_ - drag_start_y_;
  return std::clamp(declination_ + dy * drag_scale_, -PI_d/2, PI_d/2);
}

void CameraControl::updateCamPos() {
  double new_rotation    = dragging_ ? draggedRotation()    : rotation_;
  double new_declination = dragging_ ? draggedDeclination() : declination_;

  double cnd = cos(new_declination);
  double new_x = distance_ * sin(new_rotation) * cnd;
  double new_y = distance_ * sin(new_declination);
  double new_z = distance_ * cos(new_rotation) * cnd;

  cam_.lookAt(Vec3(new_x, new_y, new_z), Vec3::ZERO, Vec3::UNIT_Y);
}

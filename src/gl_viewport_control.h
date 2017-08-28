#ifndef VIEWPORT_CONTROL_H
#define VIEWPORT_CONTROL_H

#include "glfw_window.h"

class GlViewportControl : public GlfwWindow::Observer {
public:
  void onFramebufferSize(int width, int height) override;
};

#endif

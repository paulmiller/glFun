#include "gl_viewport_control.h"

void GlViewportControl::onFramebufferSize(int width, int height) {
  glViewport(0, 0, width, height);
}

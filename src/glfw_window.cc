#include "glfw_window.h"

#include "ohno.h"

#include <algorithm>
#include <cassert>

GlfwWindow::GlfwWindow() : window_(nullptr) {}

GlfwWindow::~GlfwWindow() {
  destroy();
}

GlfwWindow::operator GLFWwindow*() {
  return window_;
}

GLFWwindow* GlfwWindow::getWin() {
  return window_;
}

void GlfwWindow::create(int width, int height, const char *name) {
  window_ = glfwCreateWindow(width, height, name, nullptr, nullptr);
  if(!window_)
    throw OHNO("glfwCreateWindow failed");

  glfwSetWindowUserPointer(window_, this);
  glfwSetFramebufferSizeCallback(window_,
    static_cast<GLFWframebuffersizefun>(dispatchFramebufferSize));
  glfwSetKeyCallback(window_,
    static_cast<GLFWkeyfun>(dispatchKey));
  glfwSetMouseButtonCallback(window_,
    static_cast<GLFWmousebuttonfun>(dispatchMouseButton));
  glfwSetCursorPosCallback(window_,
    static_cast<GLFWcursorposfun>(dispatchCursorPosition));
}

void GlfwWindow::destroy() {
  if(window_) {
    glfwDestroyWindow(window_);
    window_ = nullptr;
  }
}

void GlfwWindow::addObserver(Observer *observer) {
  assert(std::find(observers_.begin(), observers_.end(), observer) ==
    observers_.end()); // assert not found
  observers_.push_back(observer);
}

void GlfwWindow::removeObserver(Observer *observer) {
  auto it = std::find(observers_.begin(), observers_.end(), observer);
  assert(it != observers_.end()); // assert found
  observers_.erase(it);
}

void GlfwWindow::Observer::onFramebufferSize(int width, int height) {}
void GlfwWindow::Observer::onKey(int key, int scancode, int action, int mods) {}
void GlfwWindow::Observer::onMouseButton(int button, int action, int mods) {}
void GlfwWindow::Observer::onCursorPosition(double x, double y) {}

/*static*/ void GlfwWindow::dispatchFramebufferSize(
  GLFWwindow *window, int width, int height
) {
  assert(glfwGetCurrentContext() == window);
  std::vector<Observer*> &observers_ =
    static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))->observers_;
  for(auto it = observers_.begin(); it != observers_.end(); ++it) {
    (*it)->onFramebufferSize(width, height);
  }
}

/*static*/ void GlfwWindow::dispatchKey(
  GLFWwindow* window, int key, int scancode, int action, int mods
) {
  assert(glfwGetCurrentContext() == window);
  std::vector<Observer*> &observers_ =
    static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))->observers_;
  for(auto it = observers_.begin(); it != observers_.end(); ++it) {
    (*it)->onKey(key, scancode, action, mods);
  }
}

/*static*/ void GlfwWindow::dispatchMouseButton(
  GLFWwindow *window, int button, int action, int mods
) {
  assert(glfwGetCurrentContext() == window);
  std::vector<Observer*> &observers_ =
    static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))->observers_;
  for(auto it = observers_.begin(); it != observers_.end(); ++it) {
    (*it)->onMouseButton(button, action, mods);
  }
}

/*static*/ void GlfwWindow::dispatchCursorPosition(
  GLFWwindow* window, double x, double y
) {
  assert(glfwGetCurrentContext() == window);
  std::vector<Observer*> &observers_ =
    static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))->observers_;
  for(auto it = observers_.begin(); it != observers_.end(); ++it) {
    (*it)->onCursorPosition(x, y);
  }
}

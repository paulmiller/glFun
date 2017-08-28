#include "glfw_window.h"

#include "ohno.h"

#include <algorithm>
#include <cassert>

GlfwWindow::GlfwWindow() : window(nullptr) {}

GlfwWindow::~GlfwWindow() {
  destroy();
}

GlfwWindow::operator GLFWwindow*() {
  return window;
}

GLFWwindow* GlfwWindow::getWin() {
  return window;
}

void GlfwWindow::create(int width, int height, const char *name) {
  window = glfwCreateWindow(width, height, name, nullptr, nullptr);
  if(!window)
    throw OHNO("glfwCreateWindow failed");

  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, static_cast<GLFWframebuffersizefun>(dispatchFramebufferSize));
  glfwSetKeyCallback(window, static_cast<GLFWkeyfun>(dispatchKey));
  glfwSetMouseButtonCallback(window, static_cast<GLFWmousebuttonfun>(dispatchMouseButton));
  glfwSetCursorPosCallback(window, static_cast<GLFWcursorposfun>(dispatchCursorPosition));
}

void GlfwWindow::destroy() {
  if(window) {
    glfwDestroyWindow(window);
    window = nullptr;
  }
}

void GlfwWindow::addObserver(Observer *observer) {
  assert(std::find(observers.begin(), observers.end(), observer) ==
    observers.end()); // assert not found
  observers.push_back(observer);
}

void GlfwWindow::removeObserver(Observer *observer) {
  auto it = std::find(observers.begin(), observers.end(), observer);
  assert(it != observers.end()); // assert found
  observers.erase(it);
}

void GlfwWindow::Observer::onFramebufferSize(int width, int height) {}
void GlfwWindow::Observer::onKey(int key, int scancode, int action, int mods) {}
void GlfwWindow::Observer::onMouseButton(int button, int action, int mods) {}
void GlfwWindow::Observer::onCursorPosition(double xpos, double ypos) {}

/*static*/ void GlfwWindow::dispatchFramebufferSize(
  GLFWwindow *window, int width, int height
) {
  assert(glfwGetCurrentContext() == window);
  std::vector<Observer*> &observers =
    static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))->observers;
  for(auto it = observers.begin(); it != observers.end(); ++it) {
    (*it)->onFramebufferSize(width, height);
  }
}

/*static*/ void GlfwWindow::dispatchKey(
  GLFWwindow* window, int key, int scancode, int action, int mods
) {
  assert(glfwGetCurrentContext() == window);
  std::vector<Observer*> &observers =
    static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))->observers;
  for(auto it = observers.begin(); it != observers.end(); ++it) {
    (*it)->onKey(key, scancode, action, mods);
  }
}

/*static*/ void GlfwWindow::dispatchMouseButton(
  GLFWwindow *window, int button, int action, int mods
) {
  assert(glfwGetCurrentContext() == window);
  std::vector<Observer*> &observers =
    static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))->observers;
  for(auto it = observers.begin(); it != observers.end(); ++it) {
    (*it)->onMouseButton(button, action, mods);
  }
}

/*static*/ void GlfwWindow::dispatchCursorPosition(
  GLFWwindow* window, double xpos, double ypos
) {
  assert(glfwGetCurrentContext() == window);
  std::vector<Observer*> &observers =
    static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))->observers;
  for(auto it = observers.begin(); it != observers.end(); ++it) {
    (*it)->onCursorPosition(xpos, ypos);
  }
}

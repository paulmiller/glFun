#ifndef GLFW_WINDOW_H
#define GLFW_WINDOW_H

#include <vector>

#include <GL/glew.h> // must precede gl, glfw
#include <GLFW/glfw3.h>

// object-oriented wrapper around GLFWwindow
class GlfwWindow {
public:
  class Observer {
  public:
    virtual void onFramebufferSize(int width, int height);
    virtual void onKey(int key, int scancode, int action, int mods);
    virtual void onMouseButton(int button, int action, int mods);
    virtual void onCursorPosition(double xpos, double ypos);
  };

  GlfwWindow();
  ~GlfwWindow();

  operator GLFWwindow*();
  GLFWwindow *getWin();

  void create(int width, int height, const char *name);
  void destroy();
  void addObserver(Observer *observer);
  void removeObserver(Observer *observer);

private:
  static void dispatchFramebufferSize(
    GLFWwindow *window, int width, int height);
  static void dispatchKey(
    GLFWwindow *window, int key, int scancode, int action, int mods);
  static void dispatchMouseButton(
    GLFWwindow *window, int button, int action, int mods);
  static void dispatchCursorPosition(
    GLFWwindow *window, double x, double y);

  GLFWwindow *window_;
  std::vector<Observer*> observers_;
};

#endif

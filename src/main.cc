#include "bytes.h"
#include "camera_control.h"
#include "gl_util.h"
#include "gl_viewport_control.h"
#include "image.h"
#include "image_hdr.h"
#include "image_png.h"
#include "mesh.h"
#include "object.h"
#include "ohno.h"
#include "util.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>

int submain();

const int defaultWidth = 512;
const int defaultHeight = 512;

/*
enum KeyCommand {
  FORWARD,
  BACKWARD,
  STRAFE_LEFT,
  STRAFE_RIGHT,
  TURN_LEFT,
  TURN_RIGHT,
  UP,
  DOWN,
  ESCAPE
};

class KeyInput {
public:
  const KeyCommand command;
  const int code;
  bool pressed;

  KeyInput(KeyCommand command_, int code_) :
    command(command_), code(code_), pressed(false) {};
};

KeyInput controls[] = {
  {FORWARD,      GLFW_KEY_COMMA},
  {BACKWARD,     GLFW_KEY_O},
  {STRAFE_LEFT,  GLFW_KEY_A},
  {STRAFE_RIGHT, GLFW_KEY_E},
  {TURN_LEFT,    GLFW_KEY_APOSTROPHE},
  {TURN_RIGHT,   GLFW_KEY_PERIOD},
  {UP,           GLFW_KEY_SPACE},
  {DOWN,         GLFW_KEY_SEMICOLON},
  {ESCAPE,       GLFW_KEY_ESCAPE}
};

const int controlNum = sizeof(controls) / sizeof(KeyInput);

bool getCommandState(KeyCommand command) {
  for(int i = 0; i < controlNum; i++) {
    if(command == controls[i].command)
      return controls[i].pressed;
  }
  assert(0);
  return false;
}

void onKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
  for(int i = 0; i < controlNum; i++) {
    if(key == controls[i].code) {
      if(action == GLFW_PRESS)
        controls[i].pressed = true;
      else if(action == GLFW_RELEASE)
        controls[i].pressed = false;
      break;
    }
  }
}

void onMouseButton(GLFWwindow *window, int button, int action, int mods) {
  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    std::cout << "click x=" << xpos << " y=" << ypos << std::endl;
    Vec3 near, far;
    cameraPtr->castPixel(int(xpos), int(ypos), near, far);
    std::cout << "cast near=" << near << " far=" << far << std::endl;
    std::cout << "hit=" << widgetPtr->intersects(near, far - near) << std::endl;
  }
}
*/

int main() {
  if(!glfwInit()) {
    std::cout << "glfwInit failed" << std::endl;
    return 1;
  }

  int ret;
  try {
    ret = submain();
  } catch(OhNo ohno) {
    std::cout << ohno << std::endl;
    ret = 1;
  }

  glfwTerminate();
  return ret;
}

#include <cmath>

int submain() {
  assert(checkGL());

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  assert(checkGL());

  GlViewportControl viewportControl;
  CameraControl cameraControl(defaultWidth, defaultHeight);
  cameraControl.getCam()->lookAt(Vec3(2,2,6), Vec3(0,0,0), Vec3::UNIT_Y);
  GlfwWindow window;
  window.addObserver(&viewportControl);
  window.addObserver(&cameraControl);
  window.create(defaultWidth, defaultHeight, "toy");

  assert(checkGL());

  assert(checkGL());

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  assert(checkGL());

  // https://www.opengl.org/wiki/OpenGL_Loading_Library
  glewExperimental = true;
  if(glewInit() != GLEW_OK) {
    std::cout << "glewInit failed" << std::endl;
    return 1;
  }
  GLenum glewError = glGetError();
  assert(glewError == GL_NO_ERROR || glewError == GL_INVALID_ENUM);

  //GLuint color = pngTex("res/suzanne-color.png");
  GLuint color = pngTex("res/axes.png");

  assert(checkGL());

  GLuint arrayId;
  glGenVertexArrays(1, &arrayId);
  glBindVertexArray(arrayId);

  //std::ifstream widgetInput("res/suzanne.obj", std::ifstream::in);
  std::ifstream widgetInput("res/axes.obj", std::ifstream::in);
  std::vector<Mesh> widgets = Mesh::parseObj(widgetInput);
  GLuint vertBufferId = vertVBO(widgets[0]);
  GLuint uvBufferId = uvVBO(widgets[0]);
  GLuint normBufferId = normVBO(widgets[0]);

  assert(checkGL());

  GLuint vertShaderId = loadShader("src/vert.glsl", GL_VERTEX_SHADER);
  GLuint fragShaderId = loadShader("src/frag.glsl", GL_FRAGMENT_SHADER);
  GLuint programId = linkProgram(vertShaderId, fragShaderId);
  glDeleteShader(vertShaderId);
  glDeleteShader(fragShaderId);

  assert(checkGL());

  GLuint MatrixID = glGetUniformLocation(programId, "MVP");
  GLuint samplerId = glGetUniformLocation(programId, "myTextureSampler");

  assert(checkGL());

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_GREATER);

  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  glClearDepth(-1.0f);

  int64_t frame = 0;
  while(!glfwWindowShouldClose(window)) {
    /*
    bool goingForward  = getCommandState(FORWARD);
    bool goingBackward = getCommandState(BACKWARD);
    bool goingLeft     = getCommandState(STRAFE_LEFT);
    bool goingRight    = getCommandState(STRAFE_RIGHT);
    bool goingUp       = getCommandState(UP);
    bool goingDown     = getCommandState(DOWN);

    Vec3 move = Vec3::ZERO;

    if(goingForward && !goingBackward)
      move += Vec3(0, 0, -1);
    else if(goingBackward && !goingForward)
      move += Vec3(0, 0, 1);

    if(goingLeft && !goingRight)
      move += Vec3(-1, 0, 0);
    else if(goingRight && !goingLeft)
      move += Vec3(1, 0, 0);

    if(goingUp && !goingDown)
      move += Vec3(0, 1, 0);
    else if(goingDown && !goingUp)
      move += Vec3(0, -1, 0);

    if(move != Vec3::ZERO) {
      move = move.unit() * 0.05;
      cameraObj.moveLocal(move);
    }

    bool turningLeft  = getCommandState(TURN_LEFT);
    bool turningRight = getCommandState(TURN_RIGHT);

    if(turningLeft && !turningRight)
      cameraObj.rot *= Quat::rotation(Vec3::UNIT_Y, 0.03);
    else if(turningRight && !turningLeft)
      cameraObj.rot *= Quat::rotation(Vec3::UNIT_Y, -0.03);

    Vec3 forward = (Mat4::rotation(cameraObj.rot) * -Vec4::UNIT_Z).dropW();
    camera.look(cameraObj.pos, forward, Vec3::UNIT_Y);
    */

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programId);

    glUniformMatrix4fv(MatrixID, 1, GL_FALSE,
      cameraControl.getCam()->getTransform().data());

    assert(checkGL());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color);
    glUniform1i(samplerId, 0);

    assert(checkGL());

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertBufferId);
    glVertexAttribPointer(
      0,        // index (attribute)
      3,        // size
      GL_FLOAT, // type
      GL_FALSE, // normalized
      0,        // stride
      (void*)0  // pointer (buffer offset)
    );

    assert(checkGL());

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferId);
    glVertexAttribPointer(
      1,        // index (attribute)
      2,        // size
      GL_FLOAT, // type
      GL_FALSE, // normalized
      0,        // stride
      (void*)0  // pointer (buffer offset)
    );

    assert(checkGL());

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normBufferId);
    glVertexAttribPointer(
      2,        // index (attribute)
      3,        // size
      GL_FLOAT, // type
      GL_FALSE, // normalized
      0,        // stride
      (void*)0  // pointer (buffer offset)
    );

    assert(checkGL());

    glDrawArrays(GL_TRIANGLES, 0, widgets[0].mTris.size() * 3);

    assert(checkGL());

    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    assert(checkGL());

    glfwSwapBuffers(window);
    glfwPollEvents();
    assert(checkGL());
    frame++;
  }

  glDeleteProgram(programId);
  glDeleteBuffers(1, &uvBufferId);
  glDeleteBuffers(1, &vertBufferId);
  glDeleteVertexArrays(1, &arrayId);
  glDeleteTextures(1, &color);

  assert(checkGL());

  return 0;
}

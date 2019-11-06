#include "ray.h"

#include "math/matrix_vector_product.h"
#include "math/util.h"

std::vector<Ray> MakeCameraRays(const Camera &camera) {
  int width = camera.GetPxCols(), height = camera.GetPxRows();
  Matrix4x4f transform = camera.getInvTransform();
  std::vector<Ray> rays;
  rays.reserve(width * height);
  for(int row = 0; row < height; row++) {
    float y = LinearMap_f(row, 0, height-1, 1, -1);
    for(int col = 0; col < width; col++) {
      float x = LinearMap_f(col, 0, width-1, -1, 1);
      Vector3f start = (transform * Vector4f{x, y, 1, 1}).divideByW();
      Vector3f end = (transform * Vector4f{x, y, -1, 1}).divideByW();
      Vector3f direction = (end - start).unit();
      rays.push_back(Ray{start, direction});
    }
  }
  return rays;
}

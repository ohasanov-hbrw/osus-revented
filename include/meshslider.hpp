#pragma once
#include "utils.hpp"
#include <cstddef>
#include <vector>

// Holds the simplified center line + arc lengths so that
// the path is simplified once, not every frame (per frame simplification of a
// snaking subpath made the vertices pop around),
// snaking cuts by true arc length.
// Yeah, we convert simplified to accurate length, and then back to simplified, but both is necessary so 
// im just gonna leave it like this. 
struct MeshSliderCache {
  std::vector<Vector2> points;   // simplified center line
  std::vector<float> arcLength;
  const Vector2 *srcData = nullptr;
  size_t srcCount = 0;
  float radius = -1.0f;
  // full-length mesh (start=0, end=1) in target space: built once, reused every
  // frame until the path, radius, screen size changes. Snaking rebuilds only
  // the partial mesh.
  std::vector<TexturedVertex> mesh;
  bool meshValid = false;
  int meshW = 0, meshH = 0;
  std::vector<Vector2> partialPoints;
  std::vector<Vector2> dirs;
  std::vector<TexturedVertex> partialMesh;
};

struct MeshSliderDrawData {
  const std::vector<Vector2> *points;
  float startProgress;  // snake out: 0 -> 1
  float endProgress;    // snake in : 0 -> 1
  float radius;
  float alpha;
  Color bodyColour;
  MeshSliderCache *cache = nullptr;
};

void MeshSliderInit();
void MeshSliderRender(const MeshSliderDrawData &data);
void MeshSliderFreeResources();
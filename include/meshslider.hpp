#pragma once
#include "utils.hpp"
#include <cstddef>
#include <vector>

constexpr float MESH_SLIDER_SHADOW_SCALE = 1.06f;

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

  // One contiguous mesh laid out in path order:
  //   [cap0][ridge0][join1][ridge1][join2]...[ridge(n-2)][cap1]
  // Built incrementally during snake-in (append as the cut advances), so no
  // full rebuild per frame and no partial-mesh buffer needed.
  std::vector<TexturedVertex> mesh;
  std::vector<Vector2> dirs;        // full-centerline segment directions
  std::vector<int> segStart;        // vertex index of ridge s (size n-1)
  std::vector<int> joinStart;       // vertex index of join at point k (size n)
  std::vector<int> joinCount;       // vertex count of join at point k (size n)
  int cap0Start = 0;
  int cap1Start = 0;
  int builtVerts = 0;               // vertices currently emitted into mesh
  int builtSeg = 0;                 // segments fully emitted (0..builtSeg-1)
  int builtJoin = 0;                // joins emitted for points 1..builtJoin-1
  bool cap0Done = false;
  bool cap1Done = false;
  bool meshValid = false;
  int meshW = 0, meshH = 0;
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
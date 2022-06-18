#include <utils.hpp>
#include <math.h>
#include <vector>
#include <algorithm>
#include <globals.hpp>

static std::vector<Vector2> MouseTrail(Global.MouseTrailSize, {-10,-10});
static float FrameTimeCounterTrail = 0;
static int indexTrail = 0;

void updateMouseTrail();

void renderMouse();
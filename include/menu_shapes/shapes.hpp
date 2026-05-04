#include <vector>
#include "raylib.h"

enum SHAPES_POSITION_ANCHORS {
    TOP_LEFT = 0,
    TOP_CENTER,
    TOP_RIGHT,
    CENTER_LEFT,
    CENTER,
    CENTER_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT,
};

struct MENU_BottomCard{
    float edgeWidth = 200;
    float edgeHeight = 60;
    float edgeRampWidth = 30;
    float height = 30;
    enum SHAPES_POSITION_ANCHORS anchor = BOTTOM_RIGHT;
    bool fillX = true;
    bool fillY = false;
    float width = 0;
    float x = 0;
    float y = 0;
};

struct MENU_TopCard{
    float edgeWidth = 260;
    float edgeHeight = 30;
    float edgeRampWidth = 20;
    float height = 10;
    enum SHAPES_POSITION_ANCHORS anchor = TOP_LEFT;
    bool fillX = true;
    bool fillY = false;
    float width = 0;
    float x = 0;
    float y = 0;
};

//rightMostButton
struct MENU_BottomCard_RMB{
    float edgeWidth = 260;
    float edgeHeight = 30;
    float edgeRampWidth = 20;
    float height = 10;
    enum SHAPES_POSITION_ANCHORS anchor = TOP_LEFT;
    bool fillX = true;
    bool fillY = false;
    float width = 0;
    float x = 0;
    float y = 0;
};


struct CommonMenuSizes {
    MENU_BottomCard bottomCard;
    MENU_TopCard topCard;
};

extern CommonMenuSizes MenuSizes;

//Create Triangle Strip Vertices for a edgy menu object, these vertices are not automatically scaled!
std::vector<Vector2> Create_Shape_Rectangle_With_Trapezoid(enum SHAPES_POSITION_ANCHORS anchor, bool fillX, bool fillY, float width, float height, float x, float y, float edgeHeight, float edgeWidth, float edgeRampWidth);

//Update already existing object to save on alloc calls 
void Update_Shape_Rectangle_With_Trapezoid(enum SHAPES_POSITION_ANCHORS anchor, bool fillX, bool fillY, float width, float height, float x, float y, float edgeHeight, float edgeWidth, float edgeRampWidth, std::vector<Vector2>* points);

//Create Triangle Strip Vertices for a edgy menu object, these vertices are not automatically scaled!
std::vector<Vector2> Create_Shape_Trapezoid(enum SHAPES_POSITION_ANCHORS anchor, bool fillX, bool fillY, float width, float height, float x, float y, float edgeHeight, float edgeWidth, float edgeRampWidth);

//Update already existing object to save on alloc calls 
void Update_Shape_Trapezoid(enum SHAPES_POSITION_ANCHORS anchor, bool fillX, bool fillY, float width, float height, float x, float y, float edgeHeight, float edgeWidth, float edgeRampWidth, std::vector<Vector2>* points);
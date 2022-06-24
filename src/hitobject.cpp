#include <hitobject.hpp>
#include <cmath>
#include <algorithm>
#include "gamemanager.hpp"
#include "utils.hpp"

float interpolate(float *p, float *time, float t) {
    float L01 = p[0] * (time[1] - t) / (time[1] - time[0]) + p[1] * (t - time[0]) / (time[1] - time[0]);
    float L12 = p[1] * (time[2] - t) / (time[2] - time[1]) + p[2] * (t - time[1]) / (time[2] - time[1]);
    float L23 = p[2] * (time[3] - t) / (time[3] - time[2]) + p[3] * (t - time[2]) / (time[3] - time[2]);
    float L012 = L01 * (time[2] - t) / (time[2] - time[0]) + L12 * (t - time[0]) / (time[2] - time[0]);
    float L123 = L12 * (time[3] - t) / (time[3] - time[1]) + L23 * (t - time[1]) / (time[3] - time[1]);
    float C12 = L012 * (time[2] - t) / (time[2] - time[1]) + L123 * (t - time[1]) / (time[2] - time[1]);
    return C12;
}   

std::vector<Vector2> interpolate(std::vector<Vector2> &points, int index, int pointsPerSegment) {
    std::vector<Vector2> result;
    float x[4];
    float y[4];
    float time[4];
    for (int i = 0; i < 4; i++) {
        x[i] = points[index + i].x;
        y[i] = points[index + i].y;
        time[i] = i;
    }
    float tstart = 1;
    float tend = 2;
    float total = 0;
    for (int i = 1; i < 4; i++) {
        float dx = x[i] - x[i - 1];
        float dy = y[i] - y[i - 1];
        total += std::pow(dx * dx + dy * dy, .25);
        time[i] = total;
    }
    tstart = time[1];
    tend = time[2];
    int segments = pointsPerSegment - 1;
    result.push_back(points[index + 1]);
    for (int i = 1; i < segments; i++) {
        float xi = interpolate(x, time, tstart + (i * (tend - tstart)) / segments);
        float yi = interpolate(y, time, tstart + (i * (tend - tstart)) / segments);
        result.push_back(Vector2{xi, yi});
    }
    result.push_back(points[index + 2]);
    return result;
}

std::vector<Vector2> interpolate(std::vector<Vector2> &coordinates, float length){
    std::vector<Vector2> vertices;
    std::vector<int> pointsPerSegment;
    for (size_t i = 0; i < coordinates.size(); i++){
        vertices.push_back(coordinates[i]);
        if(i > 0)
            pointsPerSegment.push_back(distance(vertices[i], vertices[i-1]));
    }
    float lengthAll = 0;
    for(size_t i = 0; i < pointsPerSegment.size(); i++) 
        lengthAll += pointsPerSegment[i];
    for(size_t i = 0; i < pointsPerSegment.size(); i++)
        pointsPerSegment[i] *= length/lengthAll;
    float dx = vertices[1].x - vertices[0].x;
    float dy = vertices[1].y - vertices[0].y;
    float x1 = vertices[0].x - dx;
    float y1 = vertices[0].y - dy;
    Vector2 start = {x1, y1};
    int n = vertices.size() - 1;
    dx = vertices[n].x - vertices[n-1].x;
    dy = vertices[n].y - vertices[n-1].y;
    float xn = vertices[n].x + dx;
    float yn = vertices[n].y + dy;
    Vector2 end = {xn, yn};
    vertices.insert(vertices.begin(), start);
    vertices.push_back(end);
    std::vector<Vector2> result;
    for (size_t i = 0; i < vertices.size() - 3; i++) {
        std::vector<Vector2> points = interpolate(vertices, i, pointsPerSegment[i]);
        for(size_t i = (result.size() > 0) ? 1 : 0; i < points.size(); i++)
            result.push_back(points[i]);
    }
    return result;
}

Vector2 getCatmullPoint(Vector2 &p0, Vector2 &p1, Vector2 &p2, Vector2 &p3, float alpha = 0.5f, float t = 0){
    float t01 = std::pow(distance(p0, p1), alpha);
    float t12 = std::pow(distance(p1, p2), alpha);
    float t23 = std::pow(distance(p2, p3), alpha);

    Vector2 m1 = (p2 - p1 + vectorize(t12) * ((p1 - p0) / vectorize(t01) - (p2 - p0) / vectorize((t01 + t12))));
    Vector2 m2 = (p2 - p1 + vectorize(t12) * ((p3 - p2) / vectorize(t23) - (p3 - p1) / vectorize((t12 + t23))));


    catmullRomSegment segment;
    segment.a = vectorize(2.0f) * (p1 - p2) + m1 + m2;
    segment.b = vectorize(-3.0f) * (p1 - p2) - m1 - m1 - m2;
    segment.c = m1;
    segment.d = p1;

    return segment.a * vectorize(t) * vectorize(t) * vectorize(t) + segment.b * vectorize(t) * vectorize(t) + segment.c * vectorize(t) + segment.d;
}

//checks the perfect circle slider's orientation
int orientation(Vector2 &p1, Vector2 &p2, Vector2 &p3){
    int val = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);
    return (val > 0)? false: true;
}

//gets the points needed for a bezier curve, we can also define a resolution for it
Vector2 getBezierPoint(std::vector<Vector2> &points, int numPoints, float t){
    Vector2* tmp = new Vector2[numPoints];
    for(size_t i = 0; i < points.size(); i++){
        tmp[i] = points[i];
    }
    int i = numPoints - 1;
    while (i > 0) {
        for (int k = 0; k < i; k++)
            tmp[k] = Vector2{tmp[k].x + t *(tmp[k+1].x - tmp[k].x),tmp[k].y + t *(tmp[k+1].y - tmp[k].y)};
        i--;
    }
    Vector2 answer = tmp[0];
    delete[] tmp;
    return answer;
}

//gets the center point and the size for the perfect circle sliders
std::pair<Vector2, int> getPerfectCircle(Vector2 &p1, Vector2 &p2, Vector2 &p3){
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    int x3 = p3.x;
    int y3 = p3.y;
    int a = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2;
    int b = (x1 * x1 + y1 * y1) * (y3 - y2) + (x2 * x2 + y2 * y2) * (y1 - y3) + (x3 * x3 + y3 * y3) * (y2 - y1);
    int c = (x1 * x1 + y1 * y1) * (x2 - x3) + (x2 * x2 + y2 * y2) * (x3 - x1) + (x3 * x3 + y3 * y3) * (x1 - x2);
    int x = -b / (2 * a);
    int y = -c / (2 * a);
    return std::make_pair(Vector2{(float)x,(float)y}, sqrt((x - x1) * (x - x1) + (y - y1) *(y - y1)));
}

//creates a circle
Circle::Circle(HitObjectData data){
    this->data = data;
    init();
}

//initilizes a circle
void Circle::init(){}

//the main code that runs for every circle on screen, the collision and point manager is in the GamerManager
void Circle::update(){
    GameManager* gm = GameManager::getInstance();
    //the circle is not clickable after some time so we check that
    if(gm->currentTime*1000 > data.time + gm->gameFile.p50Final){
        //this is needed for dead_update to work, maybe there is a smarter way to do that
        data.time = gm->currentTime*1000;
        data.point = 0;
        //resets the combo
        gm->clickCombo = 0;
        gm->destroyHitObject(data.index);
    }
}

//renders the Circle
void Circle::render(){
    GameManager* gm = GameManager::getInstance();
    float approachScale = 3*(1-(gm->currentTime*1000 - data.time + gm->gameFile.preempt)/gm->gameFile.preempt)+1;
    if (approachScale <= 1)
        approachScale = 1;
    float clampedFade = (gm->currentTime*1000 - data.time  + gm->gameFile.fade_in) / gm->gameFile.fade_in;
    Color renderColor;
    if(data.colour.size() > 2)
        renderColor =  Fade(Color{(unsigned char)data.colour[0],(unsigned char)data.colour[1],(unsigned char)data.colour[2]}, clampedFade);
    else
        renderColor =  Fade(Color{255,255,255}, clampedFade);
    DrawTextureCenter(gm->hitCircle, data.x, data.y, 1/2.0f , renderColor);
    DrawTextureCenter(gm->hitCircleOverlay, data.x, data.y, 1/2.0f , Fade(WHITE,clampedFade));
    DrawTextureCenter(gm->approachCircle, data.x, data.y, approachScale/2.0f , renderColor);
}

//renders the "dead" Circle
void Circle::dead_render(){
}

//just gives more time to render the "dead" Circle 
void Circle::dead_update(){
    GameManager* gm = GameManager::getInstance();
    //TODO: gives 400ms for the animation to play, MAKE IT DEPENDANT TO APPROACH RATE
    if (data.time+400 < gm->currentTime*1000)
        gm->destroyDeadHitObject(data.index);
}

//renders the combo number on top of the circle, NEED TO FIX THE DIGITS
void Circle::render_combo(){
}

//creates a Slider
Slider::Slider(HitObjectData data){
    this->data = data;
    init();
}

//initilizes a Slider, all the curve stuff and the texture creation happens here
void Slider::init(){
    GameManager* gm = GameManager::getInstance();
    //these is the points that we get from the beatmap file
    edgePoints.push_back(Vector2{(float)data.x, (float)data.y});
    //the resolution is the number of total points
    float resolution = data.length;
    float currentResolution = 0;
    float lengthScale, totalLength = 0;
    //add every point from the beatmap 
    for(size_t i = 0; i < data.curvePoints.size(); i++)
        edgePoints.push_back(Vector2{(float)data.curvePoints[i].first, (float)data.curvePoints[i].second});
    //if the "curve" is linear calculate the points needed to render the slider
    if(data.curveType == 'L'){
        //a linear "curve" consists of different sized lines so we calculate them here
        std::vector<float> lineLengths;
        for(size_t i = 0; i < edgePoints.size()-1; i++)
            lineLengths.push_back(std::sqrt(std::pow(std::abs(edgePoints[i].x - edgePoints[i+1].x),2)+std::pow(std::abs(edgePoints[i].y - edgePoints[i+1].y),2)));
        //gets the total length of the calculated lines
        for(size_t i = 0; i < lineLengths.size(); i++)
            totalLength+=lineLengths[i];
        //the calculated length is pretty different from the beatmap so we scale the calculations for that
        lengthScale = totalLength/data.length;
        //add the render points to a vector
        for(size_t i = 0; i < edgePoints.size()-1; i++)
            for(float j = 0; j < lineLengths[i]; j += lengthScale)
                renderPoints.push_back(Vector2{edgePoints[i].x + (edgePoints[i+1].x - edgePoints[i].x)*j/lineLengths[i], edgePoints[i].y + (edgePoints[i+1].y - edgePoints[i].y)*j/lineLengths[i]});
        renderPoints.push_back(edgePoints[edgePoints.size()-1]);
        //this is an inside joke, but yeah if we add more points than needed, we just delete them (doesn't happen that much)
        while(!false){
            if(renderPoints.size()-1 <= data.length) break;
            renderPoints.pop_back();
        }
    }
    else if(data.curveType == 'B'){
        //for the bezier curves we do the calculations in another function
        Vector2 edges[edgePoints.size()];
        for(size_t i = 0; i < edgePoints.size(); i++)
            edges[i] = edgePoints[i];
        std::vector<Vector2> tempEdges;
        std::vector<Vector2> tempRender;
        std::vector<float> curveLengths;
        float totalCalculatedLength = 0;
        for(size_t i = 0; i < edgePoints.size(); i++){
            tempEdges.push_back(edgePoints[i]);
            if((edgePoints[i].x == edgePoints[i+1].x && edgePoints[i].y == edgePoints[i+1].y) || i == edgePoints.size()-1){
                currentResolution = 0;
                for(float j = 0; j < 1; j += 1.0f / 100.0f){
                    if(currentResolution > 100) break;
                    currentResolution++;
                    Vector2 tmp = getBezierPoint(tempEdges, tempEdges.size(), j);
                    tempRender.push_back(tmp);
                }
                float tempLength = 0;
                for(size_t i = 1; i < tempRender.size(); i += 1)
                    tempLength += std::sqrt(std::pow(std::abs(tempRender[i-1].x - tempRender[i].x),2) + std::pow(std::abs(tempRender[i-1].y - tempRender[i].y),2));
                curveLengths.push_back(tempLength);
                totalCalculatedLength += tempLength;
                tempEdges.clear();
                tempRender.clear();
            }
        }
        tempEdges.clear();
        tempRender.clear();
        int curveIndex = 0;
        for(size_t i = 0; i < edgePoints.size(); i++){
            tempEdges.push_back(edgePoints[i]);
            if((edgePoints[i].x == edgePoints[i+1].x && edgePoints[i].y == edgePoints[i+1].y) || i == edgePoints.size()-1){
                std::vector<float> tValues;
                currentResolution = 0;
                float tempResolution = resolution / (totalCalculatedLength / curveLengths[curveIndex]);

                std::vector<Vector2> samples;
                std::vector<int> indices;
                std::vector<float> lengths;
                lengths.push_back(0);
                samples.push_back(getBezierPoint(tempEdges, tempEdges.size(), 0));
                for(int k = 1; k < tempResolution + 1; k++){
                    samples.push_back(getBezierPoint(tempEdges, tempEdges.size(), k/tempResolution));
                    lengths.push_back(distance(samples[k], samples[k-1]) + lengths[k-1]);
                }
                for(int k = 1; k < tempResolution + 1; k++)
                    lengths[k] /= lengths[tempResolution];
                indices.push_back(0);
                for(int k = 1; k < tempResolution + 1; k++){
                    float s = (float)k / tempResolution;
                    int j = Search(lengths,s,0,lengths.size()-1);
                    indices.push_back(j);
                }
                samples.push_back(getBezierPoint(tempEdges, tempEdges.size(), 1));
                for(float s = 0; s < 1; s += 1.0f / tempResolution){
                    if(currentResolution > tempResolution) break;
                    currentResolution++;
                    if(s == 1){
                        renderPoints.push_back(samples[samples.size() - 1]);
                        continue;
                    }
                    int i = (int)(s * (float)indices.size());
                    int t = indices[i];
                    renderPoints.push_back(lerp(samples[t], samples[t+1], s * ((float)indices.size()) - (float)i));
                }
                if(i != edgePoints.size()-1 && renderPoints.size() > 1)
                    renderPoints.pop_back();
                curveIndex++;
                tempEdges.clear();
            }
        }
        while(!false){
            if(renderPoints.size()-1 <= data.length) break;
            renderPoints.pop_back();
        }
    }
    else if(data.curveType == 'P'){
        std::pair<Vector2, int> circleData = getPerfectCircle(edgePoints[0], edgePoints[1], edgePoints[2]);
        Vector2 center = circleData.first;
        int radius = circleData.second;
        float degree1 = atan2(edgePoints[0].y - center.y , edgePoints[0].x - center.x) * RAD2DEG;
        float degree2 = atan2(edgePoints[1].y - center.y , edgePoints[1].x - center.x) * RAD2DEG;
        float degree3 = atan2(edgePoints[2].y - center.y , edgePoints[2].x - center.x) * RAD2DEG;
        degree1 = degree1 < 0 ? degree1 + 360 : degree1;
        degree2 = degree2 < 0 ? degree2 + 360 : degree2;
        degree3 = degree3 < 0 ? degree3 + 360 : degree3;
        bool clockwise = !orientation(edgePoints[0], edgePoints[1], edgePoints[2]);
        if(clockwise){
            degree1 = degree1 < degree3 ? degree1 + 360 : degree1;
            degree2 = degree2 < degree3 ? degree2 + 360 : degree2;
            for(float i = degree1; i > degree3 - (degree1-degree3)/resolution; i-= (degree1-degree3)/resolution){
                if(currentResolution > resolution) break;
                currentResolution++;
                Vector2 tempPoint = Vector2{center.x + cos(i / RAD2DEG) * radius, center.y + sin(i / RAD2DEG) * radius};
                renderPoints.push_back(tempPoint);
            }
        }
        else{
            degree2 = degree2 < degree1 ? degree2 + 360 : degree2;
            degree3 = degree3 < degree1 ? degree3 + 360 : degree3;
            for(float i = degree3; i > degree1 - (degree3-degree1)/resolution; i -= (degree3-degree1)/resolution){
                if(currentResolution > resolution) break;
                currentResolution++;
                Vector2 tempPoint = Vector2{center.x + cos(i / RAD2DEG) * radius, center.y + sin(i / RAD2DEG) * radius};
                renderPoints.push_back(tempPoint);
            }
            std::reverse(renderPoints.begin(), renderPoints.end());
        }
    }
    else if(data.curveType == 'C'){
        renderPoints = interpolate(edgePoints, data.length);
        while(!false){
            if(renderPoints.size()-1 <= data.length) break;
            renderPoints.pop_back();
        }
        
    }
    else{
        std::__throw_invalid_argument("Invalid Slider type!");
    }
    for(size_t i = 0; i < renderPoints.size(); i++){
        minX = std::min(minX, renderPoints[i].x);
        minY = std::min(minY, renderPoints[i].y);
        maxX = std::max(maxX, renderPoints[i].x);
        maxY = std::max(maxY, renderPoints[i].y);
    }
    
    sliderTexture = LoadRenderTexture((maxX-minX+(float)gm->hitCircle.height/2)+4, (maxY-minY+(float)gm->hitCircle.height/2)+4);
    //start to draw on the texture
    BeginTextureMode(sliderTexture);
    ClearBackground(BLANK);
    /*for(int i = 0; i < renderPoints.size(); i+=gm->skip)
        DrawCircle((renderPoints[i].x-minX+(float)gm->hitCircle.height/4), sliderTexture.texture.height - (renderPoints[i].y-minY+(float)gm->hitCircle.height/4), (gm->hitCircle.height/4-2),Color{170,170,170,255});
    DrawCircle((renderPoints[renderPoints.size()-1].x-minX+(float)gm->hitCircle.height/4), sliderTexture.texture.height - (renderPoints[renderPoints.size()-1].y-minY+(float)gm->hitCircle.height/4), (gm->hitCircle.height/4-2),Color{170,170,170,255});
    for(int i = 0; i < renderPoints.size(); i+=gm->skip)
        DrawCircle((renderPoints[i].x-minX+(float)gm->hitCircle.height/4), sliderTexture.texture.height - (renderPoints[i].y-minY+(float)gm->hitCircle.height/4), (gm->hitCircle.height/4-5),Color{12,12,12,255});
    DrawCircle((renderPoints[renderPoints.size()-1].x-minX+(float)gm->hitCircle.height/4), sliderTexture.texture.height - (renderPoints[renderPoints.size()-1].y-minY+(float)gm->hitCircle.height/4), (gm->hitCircle.height/4-5),Color{12,12,12,255});
    */
    BeginBlendMode(BLEND_ALPHA_PREMUL);
    for(int i = 0; i < renderPoints.size(); i+=gm->skip)
        DrawTextureEx(gm->sliderout, {renderPoints[i].x+1-minX, sliderTexture.texture.height - (renderPoints[i].y+1-minY+(float)gm->sliderout.height/2)}, 0, 1/2.0f, WHITE);
    DrawTextureEx(gm->sliderout, {renderPoints[renderPoints.size()-1].x+1-minX, sliderTexture.texture.height - (renderPoints[renderPoints.size()-1].y+1-minY+(float)gm->sliderout.height/2)}, 0, 1/2.0f, WHITE);
    for(int i = 0; i < renderPoints.size(); i+=gm->skip)
        DrawTextureEx(gm->sliderin, {renderPoints[i].x+1-minX, sliderTexture.texture.height - (renderPoints[i].y+1-minY+(float)gm->sliderin.height/2)}, 0, 1/2.0f, WHITE);
    DrawTextureEx(gm->sliderin, {renderPoints[renderPoints.size()-1].x+1-minX, sliderTexture.texture.height - (renderPoints[renderPoints.size()-1].y+1-minY+(float)gm->sliderin.height/2)}, 0, 1/2.0f, WHITE);
    EndBlendMode();
        
    EndTextureMode();

    GenTextureMipmaps(&sliderTexture.texture);
    SetTextureFilter(sliderTexture.texture, TEXTURE_FILTER_TRILINEAR);
}

void Slider::update(){
    GameManager* gm = GameManager::getInstance();
    position = (gm->currentTime * 1000.f - (float)data.time) / (data.timing.beatLength) * gm->sliderSpeed * data.timing.sliderSpeedOverride;
    position *= 100;
    curRepeat = std::max(0,(int)(position / data.length));
    if((int)(std::max(0.0f, position) + data.length) < (int)(data.length*data.slides)){
        repeat = true;
    }
    else{
        repeat = false;
    }
    if((int)(std::max(0.0f, position) + 2*data.length) < (int)(data.length*data.slides)){
        repeat2 = true;
    }
    else{
        repeat2 = false;
    }
    if(gm->currentTime*1000 - data.time > 0){
        if ((int)((gm->currentTime*1000 - data.time) /((data.length/100) * (data.timing.beatLength) / (gm->sliderSpeed* data.timing.sliderSpeedOverride))) % 2 == 1)
            position = (int)data.length - ((int)position % (int)data.length + 1); 
        else
            position = (int)position % (int)data.length + 1; 
    }
    position = std::max(0.f,position);
    if (is_hit_at_first || gm->currentTime*1000 > data.time + gm->gameFile.p100Final)
        state = false;
    if(gm->currentTime*1000 > data.time + (data.length/100) * (data.timing.beatLength) / (gm->sliderSpeed * data.timing.sliderSpeedOverride) * data.slides){
        data.time = gm->currentTime*1000;
        data.point = 0;
        gm->clickCombo = 0;
        gm->destroyHitObject(data.index);
    }
}

void Slider::render(){
    GameManager* gm = GameManager::getInstance();
    float approachScale = 3*(1-(gm->currentTime*1000 - data.time + gm->gameFile.preempt)/gm->gameFile.preempt)+1;
    if (approachScale <= 1)
        approachScale = 1;
    float clampedFade = (gm->currentTime*1000 - data.time  + gm->gameFile.fade_in) / gm->gameFile.fade_in;
    Color renderColor;

    DrawTextureSlider(sliderTexture.texture, minX-1, minY-1, Fade(WHITE,clampedFade), gm-> hitCircle.height/2.0f);

    if(data.colour.size() > 2)
            renderColor =  Fade(Color{(unsigned char)data.colour[0],(unsigned char)data.colour[1],(unsigned char)data.colour[2]}, clampedFade);
        else
            renderColor =  Fade(Color{255,255,255}, clampedFade);

    int calPos = position;
    calPos = std::min(calPos, static_cast<int>(renderPoints.size()-1));

    if(gm->currentTime*1000 - data.time > 0 or !state){
        DrawTextureCenter(gm->sliderb, renderPoints[calPos].x, renderPoints[calPos].y, 1/2.0f , renderColor);
    }

    if(state){
        DrawTextureCenter(gm->hitCircle, data.x, data.y, 1/2.0f , renderColor);
        DrawTextureCenter(gm->hitCircleOverlay, data.x, data.y, 1/2.0f , Fade(WHITE,clampedFade));
        DrawTextureCenter(gm->approachCircle, data.x, data.y, approachScale/2.0f , renderColor);
    }
}

void Slider::dead_render(){
}

void Slider::dead_update(){
    GameManager* gm = GameManager::getInstance();
    if (data.time+400 < gm->currentTime*1000){
        UnloadRenderTexture(sliderTexture);
        gm->destroyDeadHitObject(data.index);
    }
}

void Slider::render_combo(){
}

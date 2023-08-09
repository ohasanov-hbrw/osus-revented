#include <hitobject.hpp>
#include <cmath>
#include <algorithm>
#include "gamemanager.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <limits>

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
    float x = (float)-b / (2.0f * (float)a);
    float y = (float)-c / (2.0f * (float)a);
    //std::cout << "x1: " << x1 << " y1: " << y1 << " x2: " << x2 << " y2: " << y2 << " x3: " << x3 << " y3: " << y3 << " a: " << a << " b: " << b << " c: " << c << " x: " << x << " y: " << y << std::endl; 
    return std::make_pair(Vector2{x,y}, sqrt((x - x1) * (x - x1) + (y - y1) *(y - y1)));
}

//initilizes a Slider, all the curve stuff and the texture creation happens here
void Slider::init(){
    //std::cout << "Starting slider init at time " << data.time << "\n";
    GameManager* gm = GameManager::getInstance();

    data.textureReady = false;
    data.textureLoaded = false;
    bool durationNull = false;
    double templength = data.length;
    if(data.length < 1){
        data.length = 1;
        durationNull = true;
    }
    //these is the points that we get from the beatmap file
    double startTime = getTimer();
    edgePoints.push_back(Vector2{(float)data.x, (float)data.y});
    //the resolution is the number of total points
    float resolution = data.length;
    float currentResolution = 0;
    float lengthScale, totalLength = 0;
    //std::cout << "init: " << data.time << std::endl;
    //add every point from the beatmap 
    //renderPoints.push_back(edgePoints[0]);
    for(size_t i = 0; i < data.curvePoints.size(); i++)
        edgePoints.push_back(Vector2{(float)data.curvePoints[i].first, (float)data.curvePoints[i].second});
    //if the "curve" is linear calculate the points needed to render the slider
    if(edgePoints.size() == 1){
        for(int k = 0; k < data.length; k++){
            renderPoints.push_back(edgePoints[0]);
        }
    }
    else{
        if(data.curveType == 'L'){
            extraPosition = data.extraPos;
            edgePoints[edgePoints.size()-1] = extraPosition;
            data.totalLength-=data.lengths[data.lengths.size()-1];
            data.lengths[data.lengths.size()-1] = std::sqrt(std::pow(std::abs(edgePoints[edgePoints.size()-2].x - edgePoints[edgePoints.size()-1].x),2)+std::pow(std::abs(edgePoints[edgePoints.size()-2].y - edgePoints[edgePoints.size()-1].y),2));
            data.totalLength+=data.lengths[data.lengths.size()-1];
            lengthScale = data.totalLength/data.length;
            //lengthScale = 1;
            for(size_t i = 0; i < edgePoints.size()-1; i++)
                for(float j = 0; j < data.lengths[i]; j += lengthScale)
                    renderPoints.push_back(Vector2{edgePoints[i].x + (edgePoints[i+1].x - edgePoints[i].x)*j/data.lengths[i], edgePoints[i].y + (edgePoints[i+1].y - edgePoints[i].y)*j/data.lengths[i]});
            renderPoints.push_back(edgePoints[edgePoints.size()-1]);
            while(!false){
                if(renderPoints.size() <= data.length)
                    break;
                renderPoints.pop_back();
            }
        }
        else if(data.curveType == 'B'){
            bool old = false;
            if(old){
                //for the bezier curves we do the calculations in another function
                Vector2 edges[edgePoints.size()];
                for(size_t i = 0; i < edgePoints.size(); i++)
                    edges[i] = edgePoints[i];
                std::vector<Vector2> tempEdges;
                std::vector<Vector2> tempRender;
                std::vector<float> curveLengths;
                float totalCalculatedLength = 0;
                tempEdges.clear();
                tempRender.clear();
                int curveIndex = 0;
                double currentMax = 0;

                for(size_t i = 0; i < edgePoints.size(); i++){
                    tempEdges.push_back(edgePoints[i]);
                    if(i == edgePoints.size()-1 || (edgePoints[i].x == edgePoints[i+1].x && edgePoints[i].y == edgePoints[i+1].y)){
                        std::vector<float> tValues;
                        currentResolution = 0;
                        float tempResolution = data.lengths[curveIndex]; //clip(data.lengths[curveIndex], 0, 20000);
                        tempResolution = std::min(data.lengths[curveIndex], 400.0f);
                        std::vector<Vector2> samples;
                        std::vector<int> indices;
                        std::vector<float> lengths;
                        //if(tempResolution < 3000){
                        
                        if(tempResolution != 0){
                            samples.push_back(getBezierPoint(tempEdges, tempEdges.size(), 0));
                            lengths.push_back(0);
                            int lastk = 0;
                            for(int k = 1; k < tempResolution; k++){
                                samples.push_back(getBezierPoint(tempEdges, tempEdges.size(), k/tempResolution));
                                lengths.push_back(distance(samples[samples.size() - 1], samples[samples.size() - 2]) + lengths[lengths.size() - 1]);
                                lastk = k;
                            }

                            samples.push_back(getBezierPoint(tempEdges, tempEdges.size(), 1));
                            lengths.push_back(distance(samples[samples.size() - 1], samples[samples.size() - 2]) + lengths[lengths.size() - 1]);

                            float maxlen = 1;
                            for(int k = 0; k < lengths.size(); k++){
                                maxlen = std::max(lengths[k], maxlen);
                            }
                            for(int k = 0; k < lengths.size(); k++)
                                lengths[k] /= maxlen;
                            

                            tempResolution = data.lengths[curveIndex];

                            indices.push_back(0);
                            for(int k = 1; k < (tempResolution + 1.0f) / 2.0f; k++){
                                float s = (float)k / tempResolution * 2.0f;
                                s = clip(s, 0.0f, 1.0f);
                                int j = Search(lengths,s,0,lengths.size()-1);
                                indices.push_back(j);
                                //std::cout << "s " << s << std::endl;
                            }

                            float s = 1.0f;
                            int j = Search(lengths,s,0,lengths.size()-1);
                            if(indices[indices.size() - 1] != j)
                                indices.push_back(j);

                            samples.push_back(getBezierPoint(tempEdges, tempEdges.size(), 1));
                            //lengths.push_back(1);
                            int index = 0;
                            currentMax += data.lengths[curveIndex];
                            int add = currentMax - renderPoints.size();
                            for(index = 0; index < data.lengths[curveIndex]; index++){
                                double s = (double)index / (double)data.lengths[curveIndex];
                                
                                if(renderPoints.size() > data.length)
                                    break;
                                if(renderPoints.size() > currentMax)
                                    break;
                                currentResolution++;
                                if(currentResolution > data.lengths[curveIndex])
                                    break;
                                int i = (int)(s * (float)indices.size());
                                int t = indices[i];
                                float temporaryLerpPos;
                                if(t >= lengths.size() - 1){
                                    temporaryLerpPos = 0;
                                }
                                else{
                                    temporaryLerpPos = (s - lengths[t]) / (lengths[t + 1] - lengths[t]);
                                }
                                float otherLerp = s * ((float)indices.size()) - (float)i;
                                renderPoints.push_back(lerp(samples[t], samples[t+1], temporaryLerpPos));
                            }
                            /*if((i != edgePoints.size()-1 && renderPoints.size() > 1) and (renderPoints.size() > currentMax))
                                renderPoints.pop_back();*/
                            //std::cout << renderPoints.size() << " " << currentMax << std::endl;
                            if((i != edgePoints.size()-1 && renderPoints.size() > 1))
                            renderPoints.pop_back();
                        }
                        else{
                            std::cout << "ya wtf ya \n";
                            renderPoints.push_back(tempEdges[0]);
                        }
                        curveIndex++;
                        indices.clear();
                        tempEdges.clear();
                        //}
                        //else{
                            //just fuck it.
                        //}
                        //tempEdges.clear();

                    }
                }
                if(renderPoints.size() < data.length){
                    float angle = atan2(renderPoints[renderPoints.size()-1].y - renderPoints[renderPoints.size()-2].y, renderPoints[renderPoints.size()-1].x - renderPoints[renderPoints.size()-2].x) * 180 / 3.14159265;
                    float hipotenus = data.length - totalLength;
                    float xdiff = hipotenus * cos(-angle * 3.14159265 / 180.0f);
                    float ydiff = sqrt(hipotenus*hipotenus-xdiff*xdiff);
                    extraPosition = {renderPoints[renderPoints.size()-1].x + xdiff, renderPoints[renderPoints.size()-1].y - ydiff * (angle/abs(angle))};
                    int lerploc = renderPoints.size() - 1;
                    int res = 0;
                    for(float i = 1.0/hipotenus; i <= 1; i += 1.0/hipotenus) {
                        res++;
                        if(res > hipotenus) break;
                        renderPoints.push_back(lerp(renderPoints[lerploc], extraPosition, i));
                    }
                }
                while(!false){
                    if(renderPoints.size() <= data.length)
                        break;
                    renderPoints.pop_back();
                    //std::cout << data.time << " " << renderPoints.size() << " " << data.length << std::endl;
                }
                //std::cout << "Bdata: " << data.length << " calculated: " << renderPoints.size() << std::endl;
            }
            else{
                Vector2 edges[edgePoints.size()];
                for(size_t i = 0; i < edgePoints.size(); i++)
                    edges[i] = edgePoints[i];
                std::vector<Vector2> tempEdges;
                std::vector<Vector2> tempRender;
                std::vector<float> curveLengths;
                double totalCalculatedLength = 0;
                tempEdges.clear();
                tempRender.clear();
                int curveIndex = 0;
                double currentMax = 0;
                std::vector<Vector2> samples;
                std::vector<int> indices;
                std::vector<float> lengths;
                bool first = true;
                double tempResolution;
                for(size_t i = 0; i < edgePoints.size(); i++){
                    tempEdges.push_back(edgePoints[i]);
                    if(i == edgePoints.size()-1 || (edgePoints[i].x == edgePoints[i+1].x && edgePoints[i].y == edgePoints[i+1].y)){
                        tempResolution = data.lengths[curveIndex]; //clip(data.lengths[curveIndex], 0, 20000);
                        //std::cout << "tempResolution: " << tempResolution << std::endl;
                        tempResolution = std::min(data.lengths[curveIndex], 400.0f);
                        if(tempResolution > 0 and tempEdges.size() > 1){
                            if(first){
                                samples.push_back(getBezierPoint(tempEdges, tempEdges.size(), 0));
                                lengths.push_back(0);
                            }
                            int lastk = 0;
                            int k = 1;
                            if(!first)
                                k = 0;
                            for(; k < tempResolution; k++){
                                samples.push_back(getBezierPoint(tempEdges, tempEdges.size(), ((double)k)/tempResolution));
                                lengths.push_back(distance(samples[samples.size() - 1], samples[samples.size() - 2]) + lengths[lengths.size() - 1]);
                                lastk = k;
                            }

                            samples.push_back(getBezierPoint(tempEdges, tempEdges.size(), 1));
                            lengths.push_back(distance(samples[samples.size() - 1], samples[samples.size() - 2]) + lengths[lengths.size() - 1]);
                            if(first)
                                first = false;
                        }
                        curveIndex++;
                        tempEdges.clear();
                    }
                }

                totalCalculatedLength = lengths[lengths.size() - 1];
                tempResolution = data.length;
                if(totalCalculatedLength < data.length){
                    float angle = atan2(samples[samples.size()-1].y - samples[samples.size()-2].y, samples[samples.size()-1].x - samples[samples.size()-2].x) * 180 / 3.14159265;
                    float hipotenus = data.length - totalCalculatedLength;
                    float xdiff = hipotenus * cos(-angle * 3.14159265 / 180.0f);
                    float ydiff = sqrt(std::abs(hipotenus*hipotenus-xdiff*xdiff));
                    int ything = 1;
                    if(angle < 0.0f){
                        ything = -1;
                    }
                    else if(angle == 0.0f){
                        ything = 0;
                    }

                    Vector2 extraPosition = {samples[samples.size()-1].x + xdiff, samples[samples.size()-1].y - ydiff * (float)ything};
                    samples.push_back(extraPosition);
                    lengths.push_back(distance(samples[samples.size() - 1], samples[samples.size() - 2]) + lengths[lengths.size() - 1]);
                }
                renderPoints.clear();

                int SampleIndex = 1;

                for(int index = 0; index <= data.length; index++){
                    while(index > lengths[SampleIndex]){
                        if(SampleIndex == lengths.size() - 1)  
                            break;
                        else{
                            SampleIndex++;
                        }
                    }
                    double lerpPos = (index - lengths[SampleIndex - 1]) / (lengths[SampleIndex] - lengths[SampleIndex - 1]);
                    renderPoints.push_back(lerp(samples[SampleIndex], samples[SampleIndex - 1], lerpPos));
                }
            }   
        }
        else if(data.curveType == 'P'){
            std::pair<Vector2, float> circleData = getPerfectCircle(edgePoints[0], edgePoints[1], edgePoints[2]);
            float inf = std::numeric_limits<float>::infinity();
            if(circleData.first.x == -inf or circleData.first.x == inf or circleData.first.y == -inf or circleData.first.y == inf){
                extraPosition = data.extraPos;
                edgePoints[edgePoints.size()-1] = extraPosition;
                data.totalLength-=data.lengths[data.lengths.size()-1];
                data.lengths[data.lengths.size()-1] = std::sqrt(std::pow(std::abs(edgePoints[edgePoints.size()-2].x - edgePoints[edgePoints.size()-1].x),2)+std::pow(std::abs(edgePoints[edgePoints.size()-2].y - edgePoints[edgePoints.size()-1].y),2));
                data.totalLength+=data.lengths[data.lengths.size()-1];

                lengthScale = data.totalLength/data.length;

                for(size_t i = 0; i < edgePoints.size()-1; i++)
                    for(float j = 0; j < data.lengths[i]; j += lengthScale)
                        renderPoints.push_back(Vector2{edgePoints[i].x + (edgePoints[i+1].x - edgePoints[i].x)*j/data.lengths[i], edgePoints[i].y + (edgePoints[i+1].y - edgePoints[i].y)*j/data.lengths[i]});
                renderPoints.push_back(edgePoints[edgePoints.size()-1]);
                while(!false){
                    if(renderPoints.size() <= data.length) break;
                    renderPoints.pop_back();
                }
            }
            else{
                Vector2 center = circleData.first;
                int radius = circleData.second;
                float degree1 = atan2(edgePoints[0].y - center.y , edgePoints[0].x - center.x) * RAD2DEG;
                float degree2 = atan2(edgePoints[1].y - center.y , edgePoints[1].x - center.x) * RAD2DEG;
                float degree3 = atan2(edgePoints[2].y - center.y , edgePoints[2].x - center.x) * RAD2DEG;
                degree1 = degree1 < 0 ? degree1 + 360 : degree1;
                degree2 = degree2 < 0 ? degree2 + 360 : degree2;
                degree3 = degree3 < 0 ? degree3 + 360 : degree3;
                bool clockwise = !orientation(edgePoints[0], edgePoints[1], edgePoints[2]);
                float angle = (((data.length * 360) / radius ) / 3.14159265 ) / 2;
                int a = 0;
                if(clockwise){
                    degree1 = degree1 < degree3 ? degree1 + 360 : degree1;
                    degree2 = degree2 < degree3 ? degree2 + 360 : degree2;
                    for(float i = degree1; i > degree1 - angle; i-=angle/data.length){
                        if(a > data.length){
                            renderPoints.pop_back();
                            break;
                        }
                        Vector2 tempPoint = Vector2{center.x + cos(i / RAD2DEG) * radius, center.y + sin(i / RAD2DEG) * radius};
                        renderPoints.push_back(tempPoint);
                        a++;
                    }
                }
                else{
                    degree2 = degree2 < degree1 ? degree2 + 360 : degree2;
                    degree3 = degree3 < degree1 ? degree3 + 360 : degree3;
                    for(float i = degree1; i < degree1 + angle; i+=angle/data.length){
                        if(a > data.length){
                            renderPoints.pop_back();
                            break;
                        }
                        Vector2 tempPoint = Vector2{center.x + cos(i / RAD2DEG) * radius, center.y + sin(i / RAD2DEG) * radius};
                        renderPoints.push_back(tempPoint);
                        a++;
                    }
                    //std::reverse(renderPoints.begin(), renderPoints.end());
                }
                while(renderPoints.size() > data.length){
                    renderPoints.pop_back();
                }
                //std::cout << "Pdata: " << data.length << " size: " << renderPoints.size() << std::endl;
                
            }
            
        }
        else if(data.curveType == 'C'){
            renderPoints = interpolate(edgePoints, data.length);
            while(!false){
                if(renderPoints.size() <= data.length) break;
                renderPoints.pop_back();
            }
        }
        else{
            std::__throw_invalid_argument("Invalid Slider type!");
        }

    }
    for(size_t i = 0; i < renderPoints.size(); i++){
        if(renderPoints[i].x < -150){
            renderPoints[i].x = -150;
        }
        if(renderPoints[i].y < -150){
            renderPoints[i].y = -150;
        }
        if(renderPoints[i].x > 790){
            renderPoints[i].x = 790;
        }
        if(renderPoints[i].y > 630){
            renderPoints[i].y = 630;
        }
        minX = std::min(minX, renderPoints[i].x);
        minY = std::min(minY, renderPoints[i].y);
        maxX = std::max(maxX, renderPoints[i].x);
        maxY = std::max(maxY, renderPoints[i].y);
        
    }
    //std::cout << "slider:" << data.time << " minX:" << minX << " minY:" << minY << " maxX:" << maxX << " maxY:" << maxY << "\n";
    ticks = 0;
    sliderDuration = (double)(data.length/100) * (double)(data.timing.beatLength) / (double)((double)gm->sliderSpeed * (double)data.timing.sliderSpeedOverride);
    if(durationNull){
        sliderDuration = 1.0;
    }
    currentDuration = 0.0f;
    while(true){
        currentDuration += (double)data.timing.beatLength / (double)gm->slidertickrate;
        if(AreSame((double)currentDuration, (double)sliderDuration) || (double)currentDuration > (double)sliderDuration)
            break;
        else{
            ticks++;
        }
    }
    //ticks = (int)((sliderDuration - 1) / ((double)data.timing.beatLength / (double)gm->slidertickrate));
    if(ticks < 0)
        ticks = 0;
    std::vector<int> indices;
    if(true || data.timing.renderTicks){
        for(int i = 1; i <= ticks; i++){
            double absolutePosition = ((double)(i) * (double)((double)data.timing.beatLength / (double)gm->slidertickrate));
            bool add = true;
            if(AreSame((double)absolutePosition, (double)sliderDuration))
                add = false;
            absolutePosition = ((double)absolutePosition) / ((double)data.timing.beatLength) * (double)gm->sliderSpeed * (double)data.timing.sliderSpeedOverride;
            absolutePosition  *= (double)100;
            if(add)
                indices.push_back((int)absolutePosition);
        }
    }
    ticks = indices.size();
    for(int i = 0; i < data.slides; i++){
        if(i % 2 == 0){
            reverseclicked.push_back(-1);
            for(int j = 0; j < indices.size(); j++){
                tickPositions.push_back((int)indices[j] + (int)((double)i * data.length));
                tickclicked.push_back(-1);
            }
        }
        else{
            reverseclicked.push_back(-1);
            for(int j = indices.size() - 1; j >= 0; j--){
                tickPositions.push_back((data.length - (int)indices[j]) + (int)((double)i * data.length));
                tickclicked.push_back(-1);
            }
        }
    }
    reverseclicked.pop_back();
    //data.length = templength;
    double operationTime = getTimer() - startTime;
    //std::cout << "Init slider at time " << data.time << " with the size of " << maxX-minX << " and " << maxY-minY << " in " << operationTime << " miliseconds" << "\n";
    
    data.textureReady = true;
    if(data.slides % 2 == 0){
        data.ex = data.x;
        data.ey = data.y;
    }
    else{
        data.ex = renderPoints[renderPoints.size() - 1].x;
        data.ey = renderPoints[renderPoints.size() - 1].y;
    }
    
}

void Slider::update(){
    GameManager* gm = GameManager::getInstance();
    
    position = ((double)gm->currentTime * (double)(1000) - (double)data.time) / ((double)data.timing.beatLength) * (double)gm->sliderSpeed * (double)data.timing.sliderSpeedOverride;
    
    if(data.timing.beatLength < 0.1)
        position = 0;

    int volume = data.volume;
    if(volume == 0){
        data.volume = gm->currentTimingSettings.volume;
        volume = data.volume;
    }

    position *= (double)100;
    curRepeat = std::max(0,(int)(position / data.length));
    if(data.timing.beatLength < 0.1)
        curRepeat = 0;
    if((int)(std::max((double)0, position) + data.length) < (int)(data.length*data.slides)){
        repeat = true;
    }
    else{
        repeat = false;
    }
    if((int)(std::max((double)0, position) + 2*data.length) < (int)(data.length*data.slides)){
        repeat2 = true;
    }
    else{
        repeat2 = false;
    }

    time = position;

    double absolutePosition = position;
    while(absolutePosition > (double)data.length){
        absolutePosition -= (double)data.length;
    }
    
    if(gm->currentTime*1000.0f - data.time > 0){
        if(data.timing.beatLength > 0.1){
            if ((int)((gm->currentTime*1000.0f - data.time) /((data.length/100) * (data.timing.beatLength) / (gm->sliderSpeed* data.timing.sliderSpeedOverride))) % 2 == 1)
                position = (double)data.length - (absolutePosition + 1.0f); 
            else
                position = absolutePosition + 1.0f; 
        }
    }
    position = std::max((double)0,position);
    if (is_hit_at_first || gm->currentTime*1000.0f > data.time + gm->gameFile.p50Final)
        state = false;
    if(!state and !is_hit_at_first){
        if(gm->clickCombo > 30){
            SetSoundVolume(gm->SoundFilesAll.data["combobreak"], 1.0f);
            PlaySound(gm->SoundFilesAll.data["combobreak"]);
        }
        gm->clickCombo = 0;
    }
    calPos = position;
    calPos = std::min(calPos, static_cast<int>(renderPoints.size()-1));
    /*if(data.length == 7105)
        std::cout << renderPoints[calPos].x << " " << renderPoints[calPos].y << "\n";*/
    if(Global.useAuto and (gm->currentTime*1000.0f - data.time > 0 or !state))
        Global.AutoMousePosition = renderPoints[calPos];

    if((gm->currentTime*1000.0f - data.time > 0 or !state)){
        if(CheckCollisionPointCircle(Global.MousePosition,Vector2{renderPoints[calPos].x,renderPoints[calPos].y}, gm->circlesize))
            inSlider = true;
        else
            inSlider = false;
    }
    else{
        inSlider = false;
    }

    //DEBUG

    
    bool debugf = IsKeyDown(SDL_SCANCODE_LEFT);
    if(debugf){
        inSlider = true;
    }

    if(data.timing.beatLength < 0.1)
        inSlider = true;
    if(data.timing.beatLength < 0.1)
        is_hit_at_end = true;
    float templength = (data.length/100) * (data.timing.beatLength) / (gm->sliderSpeed * data.timing.sliderSpeedOverride) * data.slides;

    //templength = std::ceil(templength);

    if(gm->currentTime*1000.0f > data.time + templength - (36 - (18 * (templength <= 72.0f)))){
        if(inSlider && (Global.Key1D || Global.Key2D || debugf)){
            is_hit_at_end = true;
        }
    }

    if((gm->currentTime*1000.0f - data.time > 0 or !state) and renderPoints.size() > 0){
        int ticksrendered = 0;
        bool debugf = IsKeyDown(SDL_SCANCODE_LEFT);
        for(int i = 0; i < tickPositions.size(); i++){
            if(tickPositions[i] <= (int)time && (int) time > 0){
                if(tickclicked[i] == -1){
                    if((inSlider && (Global.Key1D || Global.Key2D)) || debugf){
                        tickclicked[i] = 1;
                        ticknumber++;
                        gm->clickCombo++;
                        playtick = true;
                    }
                    else{
                        tickclicked[i] = 0;
                        if(gm->clickCombo > 30){
							SetSoundVolume(gm->SoundFilesAll.data["combobreak"], 1.0f);
							PlaySound(gm->SoundFilesAll.data["combobreak"]);
						}
                        gm->clickCombo = 0;
                    }
                }
            }
        }
    }

    if(playtick){

        int defaultSampleSetForObject = 0;
        if(gm->currentTimingSettings.sampleSet == 1)
            defaultSampleSetForObject = 0;
        else if(gm->currentTimingSettings.sampleSet == 2)
            defaultSampleSetForObject = 1;
        else if(gm->currentTimingSettings.sampleSet == 3)
            defaultSampleSetForObject = 2;
        else
            defaultSampleSetForObject = gm->defaultSampleSet;

        int NormalSetForObject = 0;

		if(data.normalSet == 1)
			NormalSetForObject = 0;
		else if(data.normalSet == 2)
			NormalSetForObject = 1;
		else if(data.normalSet == 3)
			NormalSetForObject = 2;
		else
			NormalSetForObject = defaultSampleSetForObject;

        std::string NormalFileName;

        if(NormalSetForObject == 0)
            NormalFileName = "normal-slidertick";
        else if(NormalSetForObject == 1)
            NormalFileName = "soft-slidertick";
        else
            NormalFileName = "drum-slidertick";
        SetSoundPan(gm->SoundFilesAll.data[NormalFileName], 1 - (clip(renderPoints[calPos].x / 640.0, 0, 1)));
        SetSoundVolume(gm->SoundFilesAll.data[NormalFileName], ((float)volume/100.0f) * Global.hitVolume);
        PlaySound(gm->SoundFilesAll.data[NormalFileName]);

        
        playtick = false;
    }



    if(durationNull or gm->currentTime*1000.0f > data.time + (data.length/100) * (data.timing.beatLength) / (gm->sliderSpeed * data.timing.sliderSpeedOverride) * data.slides){
        
        //std::cout << "Slides: " << data.slides << " TickCount: " << ticks << " SliderDuration: " << sliderDuration << " Beatlength: " << data.timing.beatLength << std::endl;
        //std::cout << "Killed slider at time " << data.time << "\n";
        data.time = gm->currentTime*1000.0f;
        data.point = 0;
        //gm->clickCombo = 0;
        //std::cout << "Ticks: " << ticknumber << " Hit first:" << is_hit_at_first << " Hit end:" << is_hit_at_end << " Reverse:" << reversenumber << " Percentage: %" <<  ((float)(is_hit_at_end + is_hit_at_first + reversenumber + ticknumber) / (float)(tickclicked.size() + reverseclicked.size() + 2)) * 100.0f<< std::endl;

        if(durationNull or (data.length/100) * (data.timing.beatLength) / (gm->sliderSpeed * data.timing.sliderSpeedOverride) * data.slides <= 36.0f){
            if(is_hit_at_first){
                data.point = 3;
                gm->score += 300 + (300 * (std::max(gm->clickCombo-1,0) * gm->difficultyMultiplier * 1)/25);
                gm->clickCombo++;
            }
            else{
                data.point = 0;
                if(gm->clickCombo > 30){
                    SetSoundVolume(gm->SoundFilesAll.data["combobreak"], 1.0f);
                    PlaySound(gm->SoundFilesAll.data["combobreak"]);
                }
                gm->clickCombo = 0;
            }
        }
        else{
            float succ = ((float)(is_hit_at_end + is_hit_at_first + reversenumber + ticknumber) / (float)(tickclicked.size() + reverseclicked.size() + 2)) * 100.0f;
            if(AreSame(succ,0))
                data.point = 0;
            if(succ > 0.0f)
                data.point = 1;
            if(succ >= 50.0f)
                data.point = 2;
            if(AreSame(succ,100.0f))
                data.point = 3;
            if(data.point == 0){
                if(gm->clickCombo > 30){
                    SetSoundVolume(gm->SoundFilesAll.data["combobreak"], 1.0f);
                    PlaySound(gm->SoundFilesAll.data["combobreak"]);
                }
                gm->clickCombo = 0;
            }
            else if(data.point == 1){
                gm->score += 50 + (50 * (std::max(gm->clickCombo-1,0) * gm->difficultyMultiplier * 1)/25);
                gm->clickCombo++;
            }
            else if(data.point == 2){
                gm->score += 100 + (100 * (std::max(gm->clickCombo-1,0) * gm->difficultyMultiplier * 1)/25);
                gm->clickCombo++;
            }
            else if(data.point == 3){
                gm->score += 300 + (300 * (std::max(gm->clickCombo-1,0) * gm->difficultyMultiplier * 1)/25);
                gm->clickCombo++;
            }
        }

        bool debugf = IsKeyDown(SDL_SCANCODE_LEFT);
        if(is_hit_at_end || debugf){
            std::vector<std::string> sounds = getAudioFilenames(gm->currentTimingSettings.sampleSet, gm->currentTimingSettings.sampleIndex, gm->defaultSampleSet, data.edgeSets[data.edgeSets.size() - 1].first, data.edgeSets[data.edgeSets.size() - 1].second, data.edgeSounds[data.edgeSounds.size() - 1], data.hindex, data.filename);
            //std::cout << gm->currentTimingSettings.sampleSet << "  -  " << gm->currentTimingSettings.sampleIndex << "  -  " << gm->defaultSampleSet << "  -  " << data.edgeSets[data.edgeSets.size() - 1].first << "  -  " << data.edgeSets[data.edgeSets.size() - 1].second << "  -  " << data.edgeSounds[data.edgeSounds.size() - 1] << "  -  " << data.hindex << " - end at " << gm->currentTime*1000.0f << std::endl;
            
            for(int i = 0; i < sounds.size(); i+=2){
                if(gm->SoundFilesAll.data.count(sounds[i]) == 1 and gm->SoundFilesAll.loaded[sounds[i]].value){
                    SetSoundPan(gm->SoundFilesAll.data[sounds[i]], 1-clip(renderPoints[calPos].x / 640.0, 0, 1));
                    SetSoundVolume(gm->SoundFilesAll.data[sounds[i]], ((float)volume/100.0f) * Global.hitVolume);
                    PlaySound(gm->SoundFilesAll.data[sounds[i]]);
                    //std::cout << sounds[0] << " played \n";
                }
                else if(gm->SoundFilesAll.data.count(sounds[i+1]) == 1 and gm->SoundFilesAll.loaded[sounds[i+1]].value){
                    SetSoundPan(gm->SoundFilesAll.data[sounds[i+1]], 1-clip(renderPoints[calPos].x / 640.0, 0, 1));
                    SetSoundVolume(gm->SoundFilesAll.data[sounds[i+1]], ((float)volume/100.0f) * Global.hitVolume);
                    PlaySound(gm->SoundFilesAll.data[sounds[i+1]]);
                    //std::cout << sounds[1] << " played \n";
                }
            }
        }
        lastPosition = renderPoints[calPos];
        Global.AutoMousePositionStart = renderPoints[calPos];
		Global.AutoMouseStartTime = gm->currentTime*1000.0f;

        gm->destroyHitObject(data.index);
    }
    else{
        if(curRepeat > 0){
            if(reverseclicked[curRepeat-1] == -1){
                if(inSlider || debugf){
                    reverseclicked[curRepeat-1] = 1;
                    reversenumber++;
                    gm->clickCombo++;
                    std::vector<std::string> sounds = getAudioFilenames(gm->currentTimingSettings.sampleSet, gm->currentTimingSettings.sampleIndex, gm->defaultSampleSet, data.edgeSets[curRepeat].first, data.edgeSets[curRepeat].second, data.edgeSounds[curRepeat], data.hindex, data.filename);
                    //std::cout << gm->currentTimingSettings.sampleSet << " " << gm->currentTimingSettings.sampleIndex << " " << gm->defaultSampleSet << std::endl;
                    for(int i = 0; i < sounds.size(); i+=2){
                        if(gm->SoundFilesAll.data.count(sounds[i]) == 1 and gm->SoundFilesAll.loaded[sounds[i]].value){
                            SetSoundPan(gm->SoundFilesAll.data[sounds[i]], 1-clip(renderPoints[calPos].x / 640.0, 0, 1));
                            SetSoundVolume(gm->SoundFilesAll.data[sounds[i]], ((float)volume/100.0f) * Global.hitVolume);
                            PlaySound(gm->SoundFilesAll.data[sounds[i]]);
                            //std::cout << sounds[0] << " played \n";
                        }
                        else if(gm->SoundFilesAll.data.count(sounds[i+1]) == 1 and gm->SoundFilesAll.loaded[sounds[i+1]].value){
                            SetSoundPan(gm->SoundFilesAll.data[sounds[i+1]], 1-clip(renderPoints[calPos].x / 640.0, 0, 1));
                            SetSoundVolume(gm->SoundFilesAll.data[sounds[i+1]], ((float)volume/100.0f) * Global.hitVolume);
                            PlaySound(gm->SoundFilesAll.data[sounds[i+1]]);
                            //std::cout << sounds[1] << " played \n";
                        }
                    }
                }
                else{
                    reverseclicked[curRepeat-1] = 0;
                    if(gm->clickCombo > 30){
                        SetSoundVolume(gm->SoundFilesAll.data["combobreak"], 1.0f);
                        PlaySound(gm->SoundFilesAll.data["combobreak"]);
                    }
                    gm->clickCombo = 0;
                }
            }
        }
    }
}

void Slider::render(){
    bool legacyRender = Global.legacyRender;

    if(!legacyRender){
        Global.sliderTexSize = 1.0;
    }
    GameManager* gm = GameManager::getInstance();
    bool changeTex = false;
    if(data.textureReady == true and data.textureLoaded == false){
        if(legacyRender)
            sliderTexture = LoadRenderTexture((int)(((std::max(maxX-minX, 1.0f)+(float)gm->sliderout.width*(gm->circlesize/gm->sliderout.width))+16)*Global.sliderTexSize),
                                          (int)(((std::max(maxY-minY, 1.0f)+(float)gm->sliderout.width*(gm->circlesize/gm->sliderout.height))+16)*Global.sliderTexSize));
        else
            sliderTexture = LoadRenderTexture((int)(((std::max(maxX-minX, 1.0f)+(float)gm->sliderout.width*(gm->circlesize/gm->sliderout.width))+16)),
                                          (int)(((std::max(maxY-minY, 1.0f)+(float)gm->sliderout.width*(gm->circlesize/gm->sliderout.height))+16)));
        //rlFramebufferAttach(sliderTexture.id, sliderTexture.depth.id, RL_ATTACHMENT_STENCIL, RL_ATTACHMENT_TEXTURE2D, 0);
        
        SetTextureFilter(sliderTexture.texture, TEXTURE_FILTER_BILINEAR);
        BeginTextureMode(sliderTexture);
        //rlClearScreenBuffers();
        //BeginBlendMode(BLEND_ALPHA_PREMUL);
        /*if(legacyRender){
            rlEnableDepthTest(); 
            rlCustomDepthFunc(false);
            rlEnableDepthMask();
            rlClearDepth(0.0f);
            rlDisableDepthTest();
        }*/

        ClearBackground({0,0,0,0});
        if(legacyRender){
            rlEnableDepthMask();
            rlClearScreenBuffers();
        }
        //EndBlendMode();
        EndTextureMode();
        changeTex = true;
        
    }


    float approachScale = 3.5*easeInOutCubic((1-(gm->currentTime*1000.0f - data.time + gm->gameFile.preempt)/gm->gameFile.preempt))+1;
    if (approachScale <= 1)
        approachScale = 1;
    float clampedFade = clip(((gm->currentTime*1000.0f - data.time  + gm->gameFile.preempt) / gm->gameFile.fade_in) / 1.0f, 0, 1.0f);
    float clampedBigFade = clip(((gm->currentTime*1000.0f - data.time  + gm->gameFile.preempt) / gm->gameFile.fade_in) / 1.0f, 0, 100.0f);
    Color renderColor;
    data.textureLoaded = IsRenderTextureReady(sliderTexture);
    //rlCustomDepthFunc(true);
    if(data.textureLoaded and data.textureReady){
        
        if(legacyRender){   
            if(clampedBigFade <= 0.7f and renderPoints.size() > 0 and last != renderPoints.size() - 1){
                
                BeginTextureMode(sliderTexture);
                rlEnableDepthTest(); 
                //BeginBlendMode(BLEND_ALPHA_PREMUL);
                bool draw = true;
                if(renderPoints.size() > 0){
                    for(int i = last; i < std::min(((float)renderPoints.size() * (clampedFade * 2.0f)), (float)(renderPoints.size())); i+=gm->skip){
                        draw = false;
                        if(i < renderPoints.size() and renderPoints[i].x > -150 and renderPoints[i].x < 790 and renderPoints[i].y > -150 and renderPoints[i].y < 630){
                            if(!renderedLocations[(int)renderPoints[i].x + 151][(int)renderPoints[i].y + 151]){
                                /*rlSetBlendFactorsSeparate(RL_ONE, RL_ONE_MINUS_SRC_ALPHA, RL_ONE, RL_ONE_MINUS_SRC_ALPHA, RL_MIN, RL_MIN);
                                rlSetBlendMode(RL_BLEND_CUSTOM_SEPARATE);
                                DrawTextureEx(gm->sliderout, {(renderPoints[i].x+4*Global.sliderTexSize-minX)*Global.sliderTexSize,
                                (sliderTexture.texture.height - (renderPoints[i].y+4*Global.sliderTexSize-minY+(float)gm->sliderout.width*(gm->circlesize/gm->sliderout.width))*Global.sliderTexSize)},0,(gm->circlesize/gm->sliderout.width)*Global.sliderTexSize,WHITE);
                                */

                                /*
                                rlSetBlendFactorsSeparate(RL_ONE, RL_ONE_MINUS_SRC_ALPHA, RL_ONE, RL_ONE_MINUS_SRC_ALPHA, RL_MAX, RL_MAX);
                                rlSetBlendMode(RL_BLEND_CUSTOM_SEPARATE);
                                
                                DrawTextureEx(gm->sliderout, {(renderPoints[i].x+4*Global.sliderTexSize-minX)*Global.sliderTexSize,
                                (sliderTexture.texture.height - (renderPoints[i].y+4*Global.sliderTexSize-minY+(float)gm->sliderout.width*(gm->circlesize/gm->sliderout.width))*Global.sliderTexSize)},0,(gm->circlesize/gm->sliderout.width)*Global.sliderTexSize,WHITE);
                                */

                                //DrawCircle((renderPoints[i].x+4-minX + gm->circlesize/2.0f), ((sliderTexture.texture.height - (renderPoints[i].y+4-minY + gm->circlesize/2.0f))), (gm->circlesize/4.0f), RED);
                                

                                Vector2 centerCoord = {(renderPoints[i].x+4 * Global.sliderTexSize-minX + gm->circlesize/2.0f) * Global.sliderTexSize, ((sliderTexture.texture.height - (renderPoints[i].y+4 * Global.sliderTexSize-minY + gm->circlesize/2.0f) * Global.sliderTexSize))};
                                
                                //BeginBlendMode(BLEND_ALPHA_PREMUL);


                                DrawCircleWithDepth(centerCoord, ((gm->circlesize/2.0f) * 0.98)  * Global.sliderTexSize, Global.circleSector, -0.9f, {255, 255, 255, 255});

                                rlSetBlendFactorsSeparate(RL_SRC_ALPHA, RL_ONE_MINUS_SRC_ALPHA, RL_SRC_ALPHA, RL_ONE_MINUS_SRC_ALPHA, RL_MIN, RL_MIN);
                                rlSetBlendMode(RL_BLEND_CUSTOM_SEPARATE);

                                DrawCircleWithDepth(centerCoord, ((gm->circlesize/2.0f) * 0.79)  * Global.sliderTexSize, Global.circleSector, -0.5f, {2, 0, 4, 128});
                                EndBlendMode();
                                
                            
                                /*DrawLineEx({(renderPoints[last].x+4*Global.sliderTexSize-minX+gm->circlesize/2.0)*Global.sliderTexSize, sliderTexture.texture.height-(renderPoints[last].y+4*Global.sliderTexSize-minY+gm->circlesize/2.0)*Global.sliderTexSize},
                                {(renderPoints[i].x+4*Global.sliderTexSize-minX+gm->circlesize/2.0)*Global.sliderTexSize, sliderTexture.texture.height-(renderPoints[i].y+4*Global.sliderTexSize-minY+gm->circlesize/2.0)*Global.sliderTexSize},
                                (312.0) * (gm->circlesize/gm->sliderin.width)*Global.sliderTexSize , Color{28,28,28,255});*/
                                last = std::max(i, 0);
                                if(last == renderPoints.size() - 1){
                                    last = renderPoints.size();
                                }
                                renderedLocations[(int)renderPoints[i].x + 151][(int)renderPoints[i].y + 151] = true;
                                
                            }
                        }
                        
                    }
                }
                rlDisableDepthTest();
                EndTextureMode();
                
            }

            rlDisableDepthTest();
            EndBlendMode();
            //EndShaderMode();
            float outlineSize = ((17.5f * Global.sliderTexSize) * gm->circlesize/gm->sliderin.width);
            float outlineColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };     // Normalized RED color 
            float textureSize[2] = { (float)sliderTexture.texture.width, (float)sliderTexture.texture.height};
            //int transparency = GetShaderLocation(Global.shdrTest, "transparency");
            //SetShaderValue(Global.shdrTest, transparency, &clampedFade, SHADER_UNIFORM_FLOAT);


            //BeginShaderMode(Global.shdrTest);

            float amog = easeInOutCubic(clampedFade) * 0.7;
            DrawTextureSlider(sliderTexture.texture, minX, minY, Fade(WHITE,amog), gm->circlesize);
            //EndShaderMode();
            //Vector2 tempPos2 = renderPoints[(int)std::min(((float)renderPoints.size() * (clampedFade * 2.0f)), (float)(renderPoints.size()))];
            //DrawTextureCenter(gm->sliderin, tempPos.x, tempPos.y, gm->circlesize/gm->sliderin.width, RED);
        }
        else{
            if(clampedBigFade <= 0.7f and renderPoints.size() > 0 and last != renderPoints.size() - 1){
                
                BeginTextureMode(sliderTexture);
                //BeginBlendMode(BLEND_ALPHA_PREMUL);
                bool draw = true;
                if(renderPoints.size() > 0){
                    for(int i = last; i < std::min(((float)renderPoints.size() * (clampedFade * 2.5f)), (float)(renderPoints.size())); i+=gm->skip){
                        draw = false;
                        if(i < renderPoints.size() and renderPoints[i].x > -150 and renderPoints[i].x < 790 and renderPoints[i].y > -150 and renderPoints[i].y < 630){
                            if(!renderedLocations[(int)renderPoints[i].x + 151][(int)renderPoints[i].y + 151]){
                                rlSetBlendFactorsSeparate(RL_ONE, RL_ONE_MINUS_SRC_ALPHA, RL_ONE, RL_ONE_MINUS_SRC_ALPHA, RL_MAX, RL_MAX);
                                rlSetBlendMode(RL_BLEND_CUSTOM_SEPARATE);
                                DrawTextureEx(gm->sliderout, {(renderPoints[i].x+4*Global.sliderTexSize-minX)*Global.sliderTexSize,
                                (sliderTexture.texture.height - (renderPoints[i].y+4*Global.sliderTexSize-minY+(float)gm->sliderout.width*(gm->circlesize/gm->sliderout.width))*Global.sliderTexSize)},0,(gm->circlesize/gm->sliderout.width)*Global.sliderTexSize,WHITE);
                                last = std::max(i, 0);
                                if(last == renderPoints.size() - 1){
                                    last = renderPoints.size();
                                }
                                renderedLocations[(int)renderPoints[i].x + 151][(int)renderPoints[i].y + 151] = true;
                            }
                        }
                        
                    }
                }
                EndTextureMode();
            }
            EndBlendMode();
            
            
            float amog = easeInOutCubic(clampedFade) * 0.7;
            int transparency = GetShaderLocation(Global.shdrTest, "transparency");
            SetShaderValue(Global.shdrTest, transparency, &amog, SHADER_UNIFORM_FLOAT);
            BeginShaderMode(Global.shdrTest);
            DrawTextureSlider(sliderTexture.texture, minX, minY, Fade(WHITE,1), gm->circlesize);
            EndShaderMode();
        }
        
    }
    rlDisableDepthTest();
    
    clampedFade = clip(clampedFade, 0.0f, 1.0f);
    clampedFade = easeInOutCubic(clampedFade);

    int index = 0;
    int topla = 1;
    float angle = 0;
    if(curRepeat%2 == 0){
        index = renderPoints.size()-1;
        topla = -2;
    }
    if(renderPoints.size() > 2){
        angle = atan2(renderPoints[index].y- renderPoints[index+topla].y, renderPoints[index].x - renderPoints[index+topla].x);
        angle = angle * 180 / PI + 180;
    }
    if(repeat && !(clampedBigFade < 0.7f))
        DrawTextureRotate(gm->reverseArrow, renderPoints[index].x, renderPoints[index].y, (gm->circlesize/128.0f), angle, Fade(WHITE, clampedFade));
    
    index = renderPoints.size()-1;
    topla = -2;
    angle = 0;
    if(curRepeat%2 == 0){
        index = 0; 
        topla = 2; 
    }
    if(renderPoints.size() > 2){
        angle = atan2(renderPoints[index].y- renderPoints[index+topla].y, renderPoints[index].x - renderPoints[index+topla].x);
        angle = angle * 180 / PI + 180;
    }
    if(data.colour.size() > 2)
        renderColor =  Fade(Color{(unsigned char)data.colour[0],(unsigned char)data.colour[1],(unsigned char)data.colour[2]}, clampedFade);
    else
        renderColor =  Fade(Color{255,255,255}, clampedFade);
    if(repeat2 && position > 0 && !(clampedBigFade < 0.7f))
        DrawTextureRotate(gm->reverseArrow, renderPoints[index].x, renderPoints[index].y, (gm->circlesize/128.0f), angle, Fade(WHITE, clampedFade));

    if((gm->currentTime*1000.0f - data.time > 0 or !state) and renderPoints.size() > 0){
        if(calPos == 0){
            angle = atan2(renderPoints[calPos].y- renderPoints[calPos+1].y, renderPoints[calPos].x - renderPoints[calPos+1].x);
            angle = angle * 180 / PI + 180; 
        }
        else{
            angle = atan2(renderPoints[calPos].y- renderPoints[calPos-1].y, renderPoints[calPos].x - renderPoints[calPos-1].x);
            angle = angle * 180 / PI; 
        }
        if(curRepeat%2 == 0)
            angle+=180;
        int ticksrendered = 0;

        bool debugf = IsKeyDown(SDL_SCANCODE_LEFT);

        for(int i = 0; i < tickPositions.size(); i++){
            if(tickPositions[i] > (int)time && (int) time > 0 && ticksrendered < 10){
                double absolutePosition = tickPositions[i];
                int k = 0;
                while(absolutePosition > (double)data.length){
                    absolutePosition -= (double)data.length;
                    k++;
                }
                if(k % 2 == 1){
                    absolutePosition = data.length - absolutePosition;
                }
                absolutePosition = std::max((double)0,absolutePosition);
                absolutePosition = std::min((int)absolutePosition, static_cast<int>(renderPoints.size()-1));
                Vector2 pos = renderPoints[(int)absolutePosition];
                DrawTextureCenter(gm->sliderscorepoint, pos.x, pos.y, (gm->circlesize/gm->sliderscorepoint.width)/3.5f , Fade(WHITE,clip((((10 - (float)ticksrendered) / 10)),0,1)));
                ticksrendered++;
            }
        }
        
        DrawTextureRotate(gm->sliderb, renderPoints[calPos].x, renderPoints[calPos].y, (gm->circlesize/gm->sliderb.width)*(gm->sliderb.width/128.0f), angle, Fade(renderColor,clampedFade));
        if(inSlider)
            DrawTextureRotate(gm->sliderfollow, renderPoints[calPos].x, renderPoints[calPos].y, (gm->circlesize/gm->sliderfollow.width)*2*(gm->sliderfollow.width/256.0f) , angle, Fade(WHITE,clampedFade));
    }

    clampedFade = (gm->currentTime*1000.0f - data.time  + gm->gameFile.preempt) / gm->gameFile.fade_in;
    clampedFade = clip(clampedFade, 0.0f, 1.0f);
    clampedFade = easeInOutCubic(clampedFade);

    if(state){
        DrawTextureCenter(gm->hitCircle, data.x, data.y, gm->circlesize/gm->hitCircle.width*(gm->hitCircle.width/128.0f) , renderColor);
        DrawCNumbersCenter(data.comboNumber, data.x, data.y, gm->circlesize/gm->hitCircle.width*(gm->hitCircle.width/128.0f), Fade(WHITE,clampedFade));
        DrawTextureCenter(gm->hitCircleOverlay, data.x, data.y, gm->circlesize/gm->hitCircleOverlay.width*(gm->hitCircleOverlay.width/128.0f) , Fade(WHITE,clampedFade));
        DrawTextureCenter(gm->approachCircle, data.x, data.y, approachScale*gm->circlesize/gm->approachCircle.width*(gm->approachCircle.width/128.0f) , renderColor);
    }
    /*if(changeTex == true)
        data.textureLoaded = true;*/
}

void Slider::dead_render(){
    bool legacyRender = Global.legacyRender;

    if(!legacyRender){
        Global.sliderTexSize = 1.0;
    }
    
    GameManager* gm = GameManager::getInstance();
    float clampedFade = (gm->gameFile.fade_in/1.0f + data.time - gm->currentTime*1000.0f) / (gm->gameFile.fade_in/1.0f);
    float clampedFade2 = (gm->gameFile.fade_in/2.0f + data.time - gm->currentTime*1000.0f) / (gm->gameFile.fade_in/2.0f);
    if(readyToDelete){
        if(IsRenderTextureReady(sliderTexture))
            UnloadRenderTexture(sliderTexture);
        data.textureReady = false;
    }

    clampedFade = clip(clampedFade, 0.0f, 1.0f);
    clampedFade = easeInOutCubic(clampedFade);

    clampedFade2 = clip(clampedFade2, 0.0f, 1.0f);
    clampedFade2 = easeInOutCubic(clampedFade2);
    clampedFade2 = clampedFade2 * 0.7;
    
    if(data.textureReady and data.textureLoaded){
        float outlineSize = 4.0 * gm->circlesize/gm->sliderin.width;
        float outlineColor[4] = { 1.0f, 1.0f, 1.0f, clampedFade2 };     // Normalized RED color 
        float textureSize[2] = { (float)sliderTexture.texture.width, (float)sliderTexture.texture.height };
        
        // Get shader locations
        /*int outlineSizeLoc = GetShaderLocation(Global.shdrOutline, "outlineSize");
        int outlineColorLoc = GetShaderLocation(Global.shdrOutline, "outlineColor");
        int textureSizeLoc = GetShaderLocation(Global.shdrOutline, "textureSize");
        int transparency = GetShaderLocation(Global.shdrOutline, "transparency");
        
        // Set shader values (they can be changed later)
        SetShaderValue(Global.shdrOutline, outlineSizeLoc, &outlineSize, SHADER_UNIFORM_FLOAT);
        SetShaderValue(Global.shdrOutline, outlineColorLoc, outlineColor, SHADER_UNIFORM_VEC4);
        SetShaderValue(Global.shdrOutline, textureSizeLoc, textureSize, SHADER_UNIFORM_VEC2);
        SetShaderValue(Global.shdrOutline, transparency, &clampedFade2, SHADER_UNIFORM_FLOAT);


        BeginShaderMode(Global.shdrOutline);*/

        if(!legacyRender){
            int transparency = GetShaderLocation(Global.shdrTest, "transparency");
            SetShaderValue(Global.shdrTest, transparency, &clampedFade2, SHADER_UNIFORM_FLOAT);
            BeginShaderMode(Global.shdrTest);
            DrawTextureSlider(sliderTexture.texture, minX, minY, Fade(WHITE,1), gm->circlesize);
            EndShaderMode();
        }
        else{
            DrawTextureSlider(sliderTexture.texture, minX, minY, Fade(WHITE,clampedFade2), gm->circlesize);
        }
        


        float scale = (gm->currentTime*1000.0f + gm->gameFile.fade_in/2.0f - data.time) / (gm->gameFile.fade_in/2.0f);
        scale = clip(scale,1,2);
        scale -= 1.0;
        scale = easeInOutCubic(scale);
        scale += 1.0;

        Color renderColor;
        float x = lastPosition.x;
        float y = lastPosition.y;
        if(data.colour.size() > 2)
            renderColor =  Fade(Color{(unsigned char)data.colour[0],(unsigned char)data.colour[1],(unsigned char)data.colour[2]}, clampedFade2);
        else
            renderColor =  Fade(Color{255,255,255}, clampedFade2);
        DrawTextureCenter(gm->hitCircle, x, y, clip(scale/1.5f,1,2)*gm->circlesize/gm->hitCircle.width*(gm->hitCircle.width/128.0f) , renderColor);
        //DrawCNumbersCenter(data.comboNumber, data.x, data.y, gm->circlesize/gm->hitCircle.width*(gm->hitCircle.width/128.0f), Fade(WHITE,clampedFade2));
        //DrawTextureRotate(gm->sliderb, x, y, (gm->circlesize/gm->sliderb.width)*(gm->sliderb.width/128.0f), angle, Fade(renderColor,clampedFade));
        DrawTextureCenter(gm->hitCircleOverlay, x, y, clip(scale/1.5f,1,2)*gm->circlesize/gm->hitCircleOverlay.width*(gm->approachCircle.width/128.0f) , Fade(WHITE,clampedFade2));
        if(data.point != 0)
            DrawTextureCenter(gm->selectCircle, x, y, scale*gm->circlesize/gm->selectCircle.width*(gm->selectCircle.width/128.0f) , renderColor);


        if(data.point == 0)
            DrawTextureCenter(gm->hit0, renderPoints[position].x, renderPoints[position].y, (gm->circlesize/gm->hit0.width)*0.5f , Fade(WHITE,clampedFade));
        else if(data.point == 1)
            DrawTextureCenter(gm->hit50, renderPoints[position].x, renderPoints[position].y, (gm->circlesize/gm->hit50.width)*0.5f , Fade(WHITE,clampedFade));
        else if(data.point == 2)
            DrawTextureCenter(gm->hit100, renderPoints[position].x, renderPoints[position].y, (gm->circlesize/gm->hit100.width)*0.5f , Fade(WHITE,clampedFade));
        else if(data.point == 3)
            DrawTextureCenter(gm->hit300, renderPoints[position].x, renderPoints[position].y, (gm->circlesize/gm->hit300.width)*0.5f , Fade(WHITE,clampedFade));
    }
}

void Slider::dead_update(){
    GameManager* gm = GameManager::getInstance();
    if (data.time+gm->gameFile.fade_in/1.0f < gm->currentTime*1000.0f){
        readyToDelete = true;
        if(!data.textureReady){
            data.expired = true;
            renderPoints.clear();
        }
    }
}


Slider::Slider(HitObjectData data){
    this->data = data;
    //init();
}
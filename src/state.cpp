#include "state.hpp"
#include "fs.hpp"
#include <iostream>
#include "utils.hpp"
#include "gamemanager.hpp"
#include "zip.h"
#include "fastrender.hpp"
#include "rlgl.h"
#include "raylib.h"



PlayMenu::PlayMenu() {
    description = TextBox({320,140}, {520,40}, {240,98,161,255}, "Select a Beatmap\n    to play!", WHITE, 15, 50);
    bg = TextBox({320,240}, {530,290}, {240,98,161,255}, "", {240,98,161,255}, 15, 10);
    back = Button({395,360}, {120,40}, {255,135,198,255}, "Back", BLACK, 15);
    select = Button({520,360}, {120,40}, {255,135,198,255}, "Select", BLACK, 15);
    close = Button({70, 110}, {20,20}, {255,135,198,255}, "x", BLACK, 15);
    skin = Switch({310,350}, {40,20}, RED, GREEN, {255,135,198,255}, BLACK);
    sound = Switch({310,370}, {40,20}, RED, GREEN, {255,135,198,255}, BLACK);
    usedskin = TextBox({200,350}, {100,20}, {240,98,161,255}, "Use default skin", WHITE, 10, 50);
    usedsound = TextBox({200,370}, {100,20}, {240,98,161,255}, "Use default sound", WHITE, 10, 50);
    skin.state = Global.settings.useDefaultSkin;
    sound.state = Global.settings.useDefaultSounds;
}

void PlayMenu::init() {
    Global.NeedForBackgroundClear = true;
    Global.useAuto = false;
    Global.LastFrameTime = getTimer();
    temp = Global.Path;
    Global.Path = Global.BeatmapLocation;
    auto dir = ls(".osu");
    dir_list = SelectableList({320, 260}, {520, 150}, {255,135,198,255}, dir, BLACK, 10, 15, 65);
}
void PlayMenu::render() {
    //Global.mutex.lock();
    bg.render();
    description.render();
    back.render();
    select.render();
    dir_list.render();
    close.render();
    skin.render();
    sound.render();
    usedskin.render();
    usedsound.render();
    //Global.mutex.unlock();
}
void PlayMenu::update() {
    Global.enableMouse = true;
    dir_list.update();
    select.update();
    back.update();
    close.update();
    skin.update();
    sound.update();

    if(skin.state != Global.settings.useDefaultSkin)
        Global.settings.useDefaultSkin = skin.state;
    
    if(sound.state != Global.settings.useDefaultSounds)
        Global.settings.useDefaultSounds = sound.state;
    
    if(close.action){
        Global.Path = temp;
        Global.CurrentState->unload();
        Global.CurrentState.reset(new MainMenu());
    }

    if(select.action or dir_list.action){
        if(dir_list.objects.size() > 0 and dir_list.objects[dir_list.selectedindex].text.size() > 0){
            if(dir_list.objects[dir_list.selectedindex].text[dir_list.objects[dir_list.selectedindex].text.size()-1] == '/'){
                dir_list.objects[dir_list.selectedindex].text.pop_back();
                if(Global.Path.size() == 1) Global.Path.pop_back();
                Global.Path += '/' + dir_list.objects[dir_list.selectedindex].text;
                lastPos = dir_list.objects[dir_list.selectedindex].text;
                auto dir = ls(".osu");
                dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color, dir, dir_list.textcolor, dir_list.textsize, dir_list.objectsize, dir_list.maxlength);
                dir_list.init();
            }
            else{
                Global.selectedPath = Global.Path + '/' + dir_list.objects[dir_list.selectedindex].text;
                Global.CurrentLocation = "beatmaps/" + lastPos + "/";
                Global.CurrentState->unload();
                Global.CurrentState.reset(new Game());
                Global.CurrentState->init();
            }
        }
    }
    else if(back.action){
        Global.Path = Global.BeatmapLocation;
        auto dir = ls(".osu");
        dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color, dir, dir_list.textcolor, dir_list.textsize, dir_list.objectsize, dir_list.maxlength);
        dir_list.init();
    }
}
void PlayMenu::unload() {

}

LoadMenu::LoadMenu() {
    description = TextBox({320,140}, {520,40}, {240,98,161,255}, "Select a .OSZ Beatmap file\n    to extract!", WHITE, 15, 50);
    bg = TextBox({320,240}, {530,290}, {240,98,161,255}, "", {240,98,161,255}, 15, 10);
    back = Button({395,360}, {120,40}, {255,135,198,255}, "Back", BLACK, 15);
    select = Button({520,360}, {120,40}, {255,135,198,255}, "Select", BLACK, 15);
    close = Button({70, 110}, {20,20}, {255,135,198,255}, "x", BLACK, 15);
    auto dir = ls(".osz");
    dir_list = SelectableList({320, 260}, {520, 150}, {255,135,198,255}, dir, BLACK, 10, 15, 60);
    path = TextBox({195,360}, {270,40}, {240,98,161,255}, Global.Path, WHITE, 8, 40);
}

void LoadMenu::init() {
    Global.NeedForBackgroundClear = true;
    Global.useAuto = false;
    Global.LastFrameTime = getTimer();
    Global.FrameTime = 0.5;
}

void LoadMenu::render() {
    //Global.mutex.lock();
    bg.render();
    description.render();
    back.render();
    path.render();
    select.render();
    dir_list.render();
    close.render();
    //Global.mutex.unlock();
}
void LoadMenu::update() {
    Global.enableMouse = true;
    dir_list.update();
    select.update();
    back.update();
    close.update();
    
    if(close.action){
        Global.CurrentState->unload();
        Global.CurrentState.reset(new MainMenu());
    }

    if(select.action or dir_list.action){
        if(dir_list.objects.size() > 0 and dir_list.objects[dir_list.selectedindex].text.size() > 0){
            if(dir_list.objects[dir_list.selectedindex].text[dir_list.objects[dir_list.selectedindex].text.size()-1] == '/'){
                dir_list.objects[dir_list.selectedindex].text.pop_back();
                if(Global.Path.size() == 1) Global.Path.pop_back();
                Global.Path += '/' + dir_list.objects[dir_list.selectedindex].text;
                auto dir = ls(".osz");
                dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color, dir, dir_list.textcolor, dir_list.textsize, dir_list.objectsize, dir_list.maxlength);
                dir_list.init();
                path = TextBox(path.position, path.size, path.color, Global.Path, path.textcolor, path.textsize, path.maxlength);
            }
            else{
                Global.selectedPath = Global.Path + '/' + dir_list.objects[dir_list.selectedindex].text;
                std::string base_file = get_without_ext(Global.selectedPath);
                std::string final_path = Global.GamePath + "/beatmaps/" + base_file;
                create_dir(final_path);
                int arg = 2;
                std::cout << Global.selectedPath.c_str() << std::endl;
                zip_extract(Global.selectedPath.c_str(), final_path.c_str(), on_extract_entry, &arg);
            }
        }
    }
    else if(back.action){
        while(Global.Path.size() > 0){
            if(Global.Path[Global.Path.size()-1] == '/' || Global.Path[Global.Path.size() - 1] == ':'){
                if(Global.Path.size() > 1  && Global.Path.size() != 2)
                    Global.Path.pop_back();
                break;
            }
            Global.Path.pop_back();
        }
        auto dir = ls(".osz");
        dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color, dir, dir_list.textcolor, dir_list.textsize, dir_list.objectsize, dir_list.maxlength);
        dir_list.init();
        path = TextBox(path.position, path.size, path.color, Global.Path, path.textcolor, path.textsize, path.maxlength);
    }
}
void LoadMenu::unload() {

}

MainMenu::MainMenu() {
    play = Button({250,420}, {120,60}, {255,135,198,255}, "Play", BLACK, 20);
    wip = Button({320,320}, {120,60}, {255,135,198,0}, "WIP", BLACK, 20);
    load = Button({390,420}, {120,60}, {255,135,198,255}, "Load", BLACK, 20);
    volume = TestSlider({510,460}, {240,20}, BLACK, PURPLE, WHITE, WHITE);
}
void MainMenu::init() {
    Global.NeedForBackgroundClear = true;
    Global.LastFrameTime = getTimer();
    Global.FrameTime = 0.5;
    Global.useAuto = false;
    volume.location = Global.volume * 100.0f;
}
void MainMenu::update() {
    Global.enableMouse = true;
    play.update();
    wip.update();
    load.update();
    //test.update();
    if(wip.action){
        Global.CurrentState->unload();
        Global.CurrentState.reset(new WIPMenu());
        Global.CurrentState->init();
    }
    else if(play.action){
        Global.CurrentState->unload();
        Global.CurrentState.reset(new PlayMenu());
        Global.CurrentState->init();
    }
    else if(load.action){
        Global.CurrentState->unload();
        Global.CurrentState.reset(new LoadMenu());
        Global.CurrentState->init();
    }

    if(IsKeyDown(SDL_SCANCODE_LALT ))
        volume.update();
    Global.volume = volume.location / 100.0f;
}
void MainMenu::render() {
    //Global.mutex.lock();
    DrawTextureCenter(Global.OsusLogo, 320, 200, 1/3.f, WHITE);
    play.render();
    wip.render();
    load.render();
    if(IsKeyDown(SDL_SCANCODE_LALT ))
        volume.render();
    //test.render();
    //Global.mutex.unlock();
}

void MainMenu::unload() {

}

Game::Game() {
    volume = TestSlider({320,240}, {240,60}, BLUE, PURPLE, BLACK, BLACK);
}
void Game::init() {
    Global.NeedForBackgroundClear = true;
    Global.useAuto = false;
    initDone = 0;
    Global.LastFrameTime = getTimer();
    std::cout << Global.selectedPath << std::endl;
    Global.numberLines = 0;
    Global.parsedLines = 0;
    Global.loadingState = 0;
    initDone = -2;
    Global.mutex.unlock();
    Global.gameManager->loadGame(Global.selectedPath);
    Global.gameManager->timingSettingsForHitObject.clear();
    Global.mutex.lock();
    Global.startTime = -2000.0f;
    Global.errorSum = 0;
    Global.errorLast = 0;
    Global.errorDiv = 0;
    std::cout << "done init" << std::endl;
}
void Game::update() {
    if(initDone == 1){
        Global.enableMouse = false;
        Global.gameManager->run();
        if(IsKeyPressed(SDL_SCANCODE_BACKSPACE )){
            Global.gameManager->unloadGame();
            Global.CurrentState->unload();
            Global.CurrentState.reset(new PlayMenu());
            Global.CurrentState->init();
        }
    }
    else{
        if(initDone == 0 or Global.GameTextures != 0){
            initStartTime = getTimer();
            initDone = -1;
        }
        if(getTimer() - initStartTime > 4000.0f){
            std::cout << "init done in " << getTimer() - initStartTime << " secs\n";
            initDone = true;
        }
    }
    if(IsKeyDown(SDL_SCANCODE_LALT ))
        volume.update();
}
void Game::render() {
    
    if(initDone == 1){
        Global.enableMouse = false;
        Global.gameManager->render();
        //Global.mutex.lock();
        if(IsMusicStreamPlaying(Global.gameManager->backgroundMusic)){
            DrawTextEx(Global.DefaultFont, TextFormat("Playing: %.3f/%.3f", (Global.currentOsuTime/1000.0), GetMusicTimeLength(Global.gameManager->backgroundMusic)), {ScaleCordX(5), ScaleCordY(20)}, Scale(15) , Scale(1), WHITE);
            //DrawTextEx(Global.DefaultFont, TextFormat("Update rate: %.3f ms", Global.amogus3), {ScaleCordX(5), ScaleCordY(40)}, Scale(15) , Scale(1), WHITE);
            //DrawTextEx(Global.DefaultFont, TextFormat("Timer: %.3f ms", getTimer()), {ScaleCordX(5), ScaleCordY(55)}, Scale(10) , Scale(1), WHITE);
            //DrawTextEx(Global.DefaultFont, TextFormat("Last Error: %.3f ms", Global.errorLast/1000.0f), {ScaleCordX(5), ScaleCordY(65)}, Scale(10) , Scale(1), WHITE);
            //DrawTextEx(Global.DefaultFont, TextFormat("Avg Time Difference in the First Second: %.3f ms", Global.avgTime), {ScaleCordX(5), ScaleCordY(75)}, Scale(10) , Scale(1), WHITE);
        }
        else{
            DrawTextEx(Global.DefaultFont, TextFormat("Paused: %.3f/%.3f", GetMusicTimePlayed(Global.gameManager->backgroundMusic) * 1000000.0f, GetMusicTimeLength(Global.gameManager->backgroundMusic)), {ScaleCordX(5), ScaleCordY(20)}, Scale(15) , Scale(1), WHITE);
            if(Global.errorDiv != 0)
                DrawTextEx(Global.DefaultFont, TextFormat("Error Avg: %ld ms", (Global.errorSum/Global.errorDiv)/1000), {ScaleCordX(5), ScaleCordY(40)}, Scale(15) , Scale(1), WHITE);
        }
        if(GetMusicTimeLength(Global.gameManager->backgroundMusic) != 0){
            DrawLineEx({0, GetScreenHeight() - Scale(2)}, {GetScreenWidth() * ((Global.currentOsuTime/1000.0) / GetMusicTimeLength(Global.gameManager->backgroundMusic)), GetScreenHeight() - Scale(2)}, Scale(3), Fade(WHITE, 0.8));
        }
        //Global.mutex.unlock();
    }
    else if(initDone == -1){
        std::string message;
        if(getTimer() - initStartTime < 2000.0f)
            message = "Loaded Game!";
        else if(getTimer() - initStartTime < 2500.0f)
            message = "3...";
        else if(getTimer() - initStartTime < 3000.0f)
            message = "2...";
        else if(getTimer() - initStartTime < 3500.0f)
            message = "1...";
        else if(getTimer() - initStartTime < 4000.0f)
            message = "GO!";
        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
        //Global.mutex.lock();
        DrawTextEx(Global.DefaultFont, message.c_str(), {ScaleCordX(320 - message.size() * 7.5f), ScaleCordY(220)}, Scale(20), Scale(1), WHITE);
        //Global.mutex.unlock();
    }
    else if(initDone == -2){
        //Global.mutex.lock();
        std::string message;
        message = "Loading Game...";
        
        if(Global.loadingState == 1){
            //std::cout << "Precalculating HitObjects" << std::endl;
            message = "Precalculating HitObjects";
        }
        else if(Global.loadingState == 2){
            //std::cout << "Loading Background Music" << std::endl;
            message = "Loading Background Music";
        }
        else if(Global.loadingState == 3){
            //std::cout << "Loading ComboBreak Sound" << std::endl;
            message = "Loading ComboBreak Sound";
        }
        else if(Global.loadingState == 4){
            //std::cout << "Loading Hit Sounds" << std::endl;
            message = "Loading Hitsound " + std::to_string(Global.parsedLines) + " of " + std::to_string(Global.numberLines);
        }
        else if(Global.loadingState == 5){
            message = "Parsing line " + std::to_string(Global.parsedLines) + " of " + std::to_string(Global.numberLines);
        }
        else if(Global.loadingState == 6){
            message = "Parsing Timing Points";
        }
        else if(Global.loadingState == 7){
            message = "Loading Textures";
        }
        DrawTextEx(Global.DefaultFont, message.c_str(), {ScaleCordX(320 - message.size() * 7.5f), ScaleCordY(220)}, Scale(20), Scale(1), WHITE);
        //Global.mutex.unlock();
    }
    if(IsKeyDown(SDL_SCANCODE_LALT ))
        volume.render();
}

void Game::unload(){
    Global.NeedForBackgroundClear = true;
}

WIPMenu::WIPMenu() {
    
}

void WIPMenu::init(){
    //index = 0;
    Global.NeedForBackgroundClear = true;
    applyMouse = false;
    std::string temp = Global.Path;
    Global.Path = Path;
    dir.clear();
    dir = ls(".osu");
    //std::cout << "lsdone" << std::endl;
    std::sort(dir.begin(), dir.end());
    CanGoBack = false;
    if(Path != Global.BeatmapLocation + "/"){
        dir.insert(dir.begin(), "Back");
        CanGoBack = true;
    }
    Global.Path = temp;
    logo = LoadTexture("resources/osus.png");
    menu = LoadTexture("resources/menu.png");
    back = LoadTexture("resources/metadata.png");
	SetTextureFilter(logo, TEXTURE_FILTER_BILINEAR );
    SetTextureFilter(back, TEXTURE_FILTER_BILINEAR );
    float time = 0.2f;
    while(time <= 1.0f){
        time += Global.FrameTime / 1000.0f;
        //float weirdSmooth = -(std::cos(M_PI * time) - 1.0f) / 2.0f;
        float weirdSmooth = easeInOutCubic(time);
        angle = -250 + 255 * weirdSmooth;
        GetScale();
        GetMouse();
        GetKeys();
        updateMouseTrail();
        updateUpDown();
        BeginDrawing();
        ClearBackground(Global.Background);
        int index = 0;
        float tempangle = angle;
        if(tempangle < 0)
            tempangle -= 10;
        if(tempangle > 0)
            tempangle += 10;
        index = (tempangle) / 20;
        for(int i = (tempangle) / 20 - 9; i < (tempangle) / 20 + 9; i++){
            float tempAngle = angle - i * 20.0f;
            while(tempAngle > 0.0f)
                tempAngle -= 360.0f;
            while(tempAngle < 0.0f)
                tempAngle += 360.0f;
            if(tempAngle < 0)
                tempAngle -= 10;
            if(tempAngle > 0)
                tempAngle += 10;
            int tempindex = (int)(tempAngle / 20) % 18;
            if(tempindex > 9)
                tempindex = 18 - tempindex;
            else
                tempindex = -tempindex;
            index = (tempangle) / 20;
            index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();
            
            float tempAngle2 = angle - i * 20.0f;
            
            int offset = 0;
            Vector2 textpos = getPointOnCircle(610, 220, 300*weirdSmooth, tempAngle2 - 180.0f);
            int dirNum = (tempindex + index + dir.size() + offset) % dir.size();
            Color temp = WHITE;
            if(dir[dirNum] != "Back" and dir[dirNum][dir[dirNum].size() - 1] != '/')
                temp = {255,135,198,255};
            DrawTextureOnCircle(menu, 800, 240, 300*weirdSmooth, 0.4f, 0, tempAngle2 - 180.0f, temp);
            DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
            DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15*weirdSmooth, 7, WHITE);
        }
        DrawTextureRotate(logo, 800, 240, 0.5f, angle, WHITE);

        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
        renderMouse();
        DrawTextEx(Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {ScaleCordX(5), ScaleCordY(5)}, Scale(15), Scale(1), GREEN);
        EndDrawing();
    }
    applyMouse = true;
	//SetTextureFilter(menu, TEXTURE_FILTER_BILINEAR );

}
void WIPMenu::render(){
    //Global.mutex.lock();
    int index = 0;
    float tempangle = angle;
    if(tempangle < 0)
        tempangle -= 10;
    if(tempangle > 0)
        tempangle += 10;
    index = (tempangle) / 20;
    for(int i = (tempangle) / 20 - 9; i < (tempangle) / 20 + 9; i++){
        float tempAngle = angle - i * 20.0f;
        while(tempAngle > 0.0f)
            tempAngle -= 360.0f;
        while(tempAngle < 0.0f)
            tempAngle += 360.0f;
        if(tempAngle < 0)
            tempAngle -= 10;
        if(tempAngle > 0)
            tempAngle += 10;
        int tempindex = (int)(tempAngle / 20) % 18;
        if(tempindex > 9)
            tempindex = 18 - tempindex;
        else
            tempindex = -tempindex;
        index = (tempangle) / 20;
        index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();
        
        float tempAngle2 = angle - i * 20.0f;
        
        int offset = 0;
        Vector2 textpos = getPointOnCircle(610, 220, 300, tempAngle2 - 180.0f);
        int dirNum = (tempindex + index + dir.size() + offset) % dir.size();
        Color temp = WHITE;
        if(dir[dirNum] != "Back" and dir[dirNum][dir[dirNum].size() - 1] != '/')
            temp = {255,135,198,255};
        DrawTextureOnCircle(menu, 800, 240, 300, 0.4f, 0, tempAngle2 - 180.0f, temp);
        DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
        DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15, 7, WHITE);
    }
    
    DrawTextureCenter(back, 145, 240, 0.45f, Color{255,255,255,255 * easeInOutCubic(animtime)});
    if(TempMeta.size() == 5){
        DrawTextLeft((TempMeta[0]).c_str(), 25, 55, 9, Color{255,255,255,255 * easeInOutCubic(animtime)});
        DrawTextLeft((TempMeta[1]).c_str(), 25, 70, 9, Color{255,255,255,255 * easeInOutCubic(animtime)});
        DrawTextLeft((TempMeta[2]).c_str(), 25, 85, 9, Color{255,255,255,255 * easeInOutCubic(animtime)});
        DrawTextLeft((TempMeta[3]).c_str(), 25, 100, 9, Color{255,255,255,255 * easeInOutCubic(animtime)});
        DrawTextLeft((TempMeta[4]).c_str(), 25, 115, 9, Color{255,255,255,255 * easeInOutCubic(animtime)});
    }

    DrawTextureRotate(logo, 800, 240, 0.5f, angle, WHITE);
    //Global.mutex.unlock();
}
void WIPMenu::update(){
    float clampaccel = 0;
    if(applyMouse){
        accel += (float)(Global.FrameTime / 1000.0f) * (float)(100.0f * -Global.Wheel);
        if(accel > 60.0f)
            accel = 60.0f;
        if(accel < -60.0f)
            accel = -60.0f;
        if(accel < 0.01f and accel > -0.01f)
            accel = 0.0f;
    }
    float floatangle = ((int)posangle) % 20;
    if(floatangle >= 20.0f)
        floatangle -= 20.0f;
    if(floatangle >= 10.0f)
        clampaccel = (float)(Global.FrameTime / 1000.0f) * (float)(2.5f * (20.0f - floatangle));
    else
        clampaccel = -(float)(Global.FrameTime / 1000.0f) * (float)(2.5f * (floatangle));
    
    posangle = angle;
    while(posangle < 0.0f)
        posangle += 360.0f;
    
    int index = 0;
    float tempangle = angle;
    if(tempangle < 0)
        tempangle -= 10;
    if(tempangle > 0)
        tempangle += 10;
    index = (tempangle) / 20;
    index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();
    index = index % dir.size();
    
    if(Global.Key1D and Global.MouseInFocus and CheckCollisionPointRec(Global.MousePosition, Rectangle{320,-2000,320,6000})){
        if(Global.MouseInFocus)
            mouseMovement += Global.MousePosition.y - lastMouse;
        if(Global.MouseInFocus)
            absMouseMovement += std::abs(Global.MousePosition.y - lastMouse);
        if(absMouseMovement > 0.5f){
            if(applyMouse)
                angle += (Global.MousePosition.y - lastMouse) / -5.0f;
            moving = true;
        }   
    }
    if(!moving and Global.Key1R and CheckCollisionPointRec(Global.MousePosition, Rectangle{305,198,210,81})){
        if(selectedIndex == index){
            selectedIndex = -1;
            subObjects.clear();
        }
        else
            selectedIndex = index;
        index = 0;
        float tempangle = angle;
        if(tempangle < 0)
            tempangle -= 10;
        if(tempangle > 0)
            tempangle += 10;
        index = (tempangle) / 20;
        selectedAngleIndex = index;
        if(selectedIndex != -1){
            Global.Key1R = false;
            int size = dir[selectedIndex].size();
            if(dir[selectedIndex].size() > 0 and dir[selectedIndex][dir[selectedIndex].size() - 1] == '/'){
                Path += dir[selectedIndex];
                ParseNameFolder(dir[selectedIndex]);
                applyMouse = false;
                float time = 0.2f;
                while(time <= 1.0f){
                    time += (Global.FrameTime / 1000.0f) * 1.0f;
                    //float weirdSmooth = -(std::cos(M_PI * time) - 1.0f) / 2.0f;
                    float weirdSmooth = easeInOutCubic(1.0f-time);
                    angle = -250 + 250 * weirdSmooth;
                    GetScale();
                    GetMouse();
                    GetKeys();
                    updateMouseTrail();
                    updateUpDown();
                    BeginDrawing();
                    ClearBackground(Global.Background);
                    int index = 0;
                    float tempangle = angle;
                    if(tempangle < 0)
                        tempangle -= 10;
                    if(tempangle > 0)
                        tempangle += 10;
                    index = (tempangle) / 20;
                    for(int i = (tempangle) / 20 - 9; i < (tempangle) / 20 + 9; i++){
                        float tempAngle = angle - i * 20.0f;
                        while(tempAngle > 0.0f)
                            tempAngle -= 360.0f;
                        while(tempAngle < 0.0f)
                            tempAngle += 360.0f;
                        if(tempAngle < 0)
                            tempAngle -= 10;
                        if(tempAngle > 0)
                            tempAngle += 10;
                        int tempindex = (int)(tempAngle / 20) % 18;
                        if(tempindex > 9)
                            tempindex = 18 - tempindex;
                        else
                            tempindex = -tempindex;
                        index = (tempangle) / 20;
                        index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();
                        
                        float tempAngle2 = angle - i * 20.0f;
                        
                        int offset = 0;
                        Vector2 textpos = getPointOnCircle(610, 220, 300*weirdSmooth, tempAngle2 - 180.0f);
                        
                        int dirNum = (tempindex + index + dir.size() + offset) % dir.size();
                        Color temp = WHITE;
                        if(dir[dirNum] != "Back" and dir[dirNum][dir[dirNum].size() - 1] != '/')
                            temp = {255,135,198,255};
                        DrawTextureOnCircle(menu, 800, 240, 300*weirdSmooth, 0.4f, 0, tempAngle2 - 180.0f, temp);
                        DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
                        DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15*weirdSmooth, 7, WHITE);
                    }
                    DrawTextureRotate(logo, 800, 240, 0.5f, angle, WHITE);

                    DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
                    DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
                    renderMouse();
                    DrawTextEx(Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {ScaleCordX(5), ScaleCordY(5)}, Scale(15), Scale(1), GREEN);
                    EndDrawing();
                }
                applyMouse = true;
                init();
                index = 0;
            }
            else if(dir[selectedIndex] == "Back" and selectedIndex == 0 and CanGoBack){
                Path.pop_back();
                while(Path[Path.size()-1] != '/'){
                    Path.pop_back();
                }
                if(Path.size() <= (Global.BeatmapLocation + "/").size()){
                    Path = Global.BeatmapLocation + "/";
                }
                selectedIndex = -1;
                std::cout << Path << std::endl;
                applyMouse = false;
                float time = 0.2f;
                while(time <= 1.0f){
                    time += (Global.FrameTime / 1000.0f) * 1.0f;
                    //float weirdSmooth = -(std::cos(M_PI * time) - 1.0f) / 2.0f;
                    float weirdSmooth = easeInOutCubic(1.0f-time);
                    angle = -250 + 250 * weirdSmooth;
                    GetScale();
                    GetMouse();
                    GetKeys();
                    updateMouseTrail();
                    updateUpDown();
                    BeginDrawing();
                    ClearBackground(Global.Background);
                    int index = 0;
                    float tempangle = angle;
                    if(tempangle < 0)
                        tempangle -= 10;
                    if(tempangle > 0)
                        tempangle += 10;
                    index = (tempangle) / 20;
                    for(int i = (tempangle) / 20 - 9; i < (tempangle) / 20 + 9; i++){
                        float tempAngle = angle - i * 20.0f;
                        while(tempAngle > 0.0f)
                            tempAngle -= 360.0f;
                        while(tempAngle < 0.0f)
                            tempAngle += 360.0f;
                        if(tempAngle < 0)
                            tempAngle -= 10;
                        if(tempAngle > 0)
                            tempAngle += 10;
                        int tempindex = (int)(tempAngle / 20) % 18;
                        if(tempindex > 9)
                            tempindex = 18 - tempindex;
                        else
                            tempindex = -tempindex;
                        index = (tempangle) / 20;
                        index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();
                        
                        float tempAngle2 = angle - i * 20.0f;
                        
                        int offset = 0;
                        Vector2 textpos = getPointOnCircle(610, 220, 300*weirdSmooth, tempAngle2 - 180.0f);
                        int dirNum = (tempindex + index + dir.size() + offset) % dir.size();
                        Color temp = WHITE;
                        if(dir[dirNum] != "Back" and dir[dirNum][dir[dirNum].size() - 1] != '/')
                            temp = {255,135,198,255};
                        DrawTextureOnCircle(menu, 800, 240, 300*weirdSmooth, 0.4f, 0, tempAngle2 - 180.0f, temp);
                        DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
                        DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15*weirdSmooth, 7, WHITE);
                    }
                    DrawTextureRotate(logo, 800, 240, 0.5f, angle, WHITE);

                    DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
                    DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
                    renderMouse();
                    DrawTextEx(Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {ScaleCordX(5), ScaleCordY(5)}, Scale(15), Scale(1), GREEN);
                    EndDrawing();
                }
                applyMouse = true;
                init();
                index = 0;
            }
            else if(size >= 4 and dir[selectedIndex][size - 1] == 'u' and dir[selectedIndex][size - 2] == 's' and dir[selectedIndex][size - 3] == 'o' and dir[selectedIndex][size - 4] == '.'){
                ParseNameFile(Path + dir[selectedIndex]);
            }
            selectedIndex = -1;
        }
    }
    if(Global.Key1R and Global.MouseInFocus and CheckCollisionPointRec(Global.MousePosition, Rectangle{320,-2000,320,6000})){
        moving = false;
        if(applyMouse)
            accel += (Global.MousePosition.y - lastMouse) / -10.0f;
        mouseMovement = 0;
        absMouseMovement = 0;
    }
    if(Global.MouseInFocus and CheckCollisionPointRec(Global.MousePosition, Rectangle{320,-2000,320,6000}))
        lastMouse = Global.MousePosition.y;
    //std::cout << accel << std::endl;
    if(applyMouse)
        accel += ((-accel) / 2.0f) * ((float)(Global.FrameTime / 1000.0f) * 8.0f);
    angle += accel;
    if(!moving)
        angle += clampaccel;


    if(AreSame(accel,0.0f) and AreSame(clampaccel,0.0f)){
        if(dir[index] != "Back" and dir[index][dir[index].size() - 1] != '/'){
            renderMetadata = true;
            if(Metadata.size() == 0){
                TempMeta.clear();
                int size = dir[index].size();
                std::vector<std::string> output;
                if(size >= 4 and dir[index][size - 1] == 'u' and dir[index][size - 2] == 's' and dir[index][size - 3] == 'o' and dir[index][size - 4] == '.'){
                    output = ParseNameFile(Path + dir[index]);
                }
                if(output.size() == 5){
                    Metadata.push_back("Title: " + output[0]);
                    Metadata.push_back("Artist: " + output[1]);
                    Metadata.push_back("Creator: " + output[2]);
                    Metadata.push_back("Ver. " + output[3]);
                    Metadata.push_back("ID: " + output[4]);
                    TempMeta = Metadata;
                }
            }
            animtime += (Global.FrameTime / 1000.0f) * 2.0f;
            if(animtime > 1.0f)
                animtime = 1.0f;
        }
    }
    else{
        renderMetadata = false;
        if(Metadata.size() > 0)
            Metadata.clear();
        animtime -= (Global.FrameTime / 1000.0f) * 2.0f;
        if(animtime < 0.0f)
            animtime = 0.0f;
    }

    if(IsKeyPressed(SDL_SCANCODE_BACKSPACE) and CanGoBack){
        Path.pop_back();
        while(Path[Path.size()-1] != '/'){
            Path.pop_back();
        }
        if(Path.size() <= (Global.BeatmapLocation + "/").size()){
            Path = Global.BeatmapLocation + "/";
        }
        selectedIndex = -1;
        std::cout << Path << std::endl;
        applyMouse = false;
        float time = 0.2f;
        while(time <= 1.0f){
            time += (Global.FrameTime / 1000.0f) * 1.0f;
            //float weirdSmooth = -(std::cos(M_PI * time) - 1.0f) / 2.0f;
            float weirdSmooth = easeInOutCubic(1.0f-time);
            angle = -250 + 250 * weirdSmooth;
            GetScale();
            GetMouse();
            GetKeys();
            updateMouseTrail();
            updateUpDown();
            BeginDrawing();
            ClearBackground(Global.Background);
            int index = 0;
            float tempangle = angle;
            if(tempangle < 0)
                tempangle -= 10;
            if(tempangle > 0)
                tempangle += 10;
            index = (tempangle) / 20;
            for(int i = (tempangle) / 20 - 9; i < (tempangle) / 20 + 9; i++){
                float tempAngle = angle - i * 20.0f;
                while(tempAngle > 0.0f)
                    tempAngle -= 360.0f;
                while(tempAngle < 0.0f)
                    tempAngle += 360.0f;
                if(tempAngle < 0)
                    tempAngle -= 10;
                if(tempAngle > 0)
                    tempAngle += 10;
                int tempindex = (int)(tempAngle / 20) % 18;
                if(tempindex > 9)
                    tempindex = 18 - tempindex;
                else
                    tempindex = -tempindex;
                index = (tempangle) / 20;
                index += (std::abs(index / (int)dir.size()) + 1) * (int)dir.size();
                
                float tempAngle2 = angle - i * 20.0f;
                
                int offset = 0;
                Vector2 textpos = getPointOnCircle(610, 220, 300*weirdSmooth, tempAngle2 - 180.0f);
                int dirNum = (tempindex + index + dir.size() + offset) % dir.size();
                Color temp = WHITE;
                if(dir[dirNum] != "Back" and dir[dirNum][dir[dirNum].size() - 1] != '/')
                    temp = {255,135,198,255};
                DrawTextureOnCircle(menu, 800, 240, 300*weirdSmooth, 0.4f, 0, tempAngle2 - 180.0f, temp);
                DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
                DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15*weirdSmooth, 7, WHITE);
            }
            DrawTextureRotate(logo, 800, 240, 0.5f, angle, WHITE);

            DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
            DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
            renderMouse();
            DrawTextEx(Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {ScaleCordX(5), ScaleCordY(5)}, Scale(15), Scale(1), GREEN);
            EndDrawing();
        }
        applyMouse = true;
        init();
    }
    if(IsKeyPressed(SDL_SCANCODE_BACKSPACE ) and !CanGoBack){
        Global.CurrentState->unload();
        Global.CurrentState.reset(new MainMenu());
    }

}
void WIPMenu::unload(){
    dir.clear();
    subObjects.clear();
    UnloadTexture(logo);
    UnloadTexture(back);
    UnloadTexture(menu);
}

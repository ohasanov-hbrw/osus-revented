#include "state.hpp"
#include "fs.hpp"
#include <iostream>
#include "globals.hpp"
#include "utils.hpp"
#include "gamemanager.hpp"
#include "zip.h"
#include "fastrender.hpp"

PlayMenu::PlayMenu() {
    description = TextBox({320,140}, {520,40}, {240,98,161,255}, "Select a Beatmap\n    to play!", WHITE, 15, 50);
    bg = TextBox({320,240}, {530,290}, {240,98,161,255}, "", {240,98,161,255}, 15, 10);
    back = Button({395,360}, {120,40}, {255,135,198,255}, "Back", BLACK, 15);
    select = Button({520,360}, {120,40}, {255,135,198,255}, "Select", BLACK, 15);
    close = Button({70, 110}, {20,20}, {255,135,198,255}, "x", BLACK, 15);
    
}

void PlayMenu::init() {
    temp = Global.Path;
    Global.Path = Global.BeatmapLocation;
    auto dir = ls(".osu");
    dir_list = SelectableList({320, 260}, {520, 150}, {255,135,198,255}, dir, BLACK, 10, 15, 65);
}
void PlayMenu::render() {
    bg.render();
    description.render();
    back.render();
    select.render();
    dir_list.render();
    close.render();
}
void PlayMenu::update() {
    Global.enableMouse = true;
    dir_list.update();
    select.update();
    back.update();
    close.update();

    if(close.action){
        Global.Path = temp;
        Global.CurrentState->unload();
        Global.CurrentState.reset(new MainMenu());
    }

    if(select.action){
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

void LoadMenu::init() {}

void LoadMenu::render() {
    bg.render();
    description.render();
    back.render();
    path.render();
    select.render();
    dir_list.render();
    close.render();
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

    if(select.action){
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
}
void MainMenu::init() {}
void MainMenu::update() {
    Global.enableMouse = true;
    play.update();
    wip.update();
    load.update();
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
}
void MainMenu::render() {
    DrawTextureCenter(Global.OsusLogo, 320, 200, 1/3.f, WHITE);
    play.render();
    wip.render();
    load.render();
}

void MainMenu::unload() {

}

Game::Game() {
    
}
void Game::init() {
    std::cout << Global.selectedPath << std::endl;
    Global.gameManager->loadGame(Global.selectedPath);
    Global.gameManager->timingSettingsForHitObject.clear();

    float timer = 0;
    while(timer < 4.0f){
        timer += GetFrameTime();

        GetScale();
        GetMouse();
        GetKeys();

        updateMouseTrail();
        updateUpDown();

        std::string message;
        if(timer < 2.0f)
            message = "Loaded Game";
        else if(timer < 2.5f)
            message = "3!";
        else if(timer < 3.0f)
            message = "2!";
        else if(timer < 3.5f)
            message = "1!";
        else if(timer < 4.0f)
            message = "GO!";
        
        BeginDrawing();
        ClearBackground(Global.Background);
        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
        
        DrawTextEx(Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {ScaleCordX(5), ScaleCordY(5)}, Scale(15), Scale(1), GREEN);
        DrawTextEx(Global.DefaultFont, message.c_str(), {ScaleCordX(320 - message.size() * 15), ScaleCordY(220)}, Scale(40), Scale(1), WHITE);
        renderMouse();
        EndDrawing();
    }
}
void Game::update() {
    Global.enableMouse = false;
    Global.gameManager->run();
    if(IsKeyPressed(KEY_BACKSPACE)){
        Global.gameManager->unloadGame();
        Global.CurrentState->unload();
        Global.CurrentState.reset(new PlayMenu());
        Global.CurrentState->init();
    }
    if(IsKeyPressed(KEY_SPACE)){
        if(IsMusicStreamPlaying(Global.gameManager->backgroundMusic))
            PauseMusicStream(Global.gameManager->backgroundMusic);
        else
            ResumeMusicStream(Global.gameManager->backgroundMusic);
    }
}
void Game::render() {
    Global.enableMouse = false;
    Global.gameManager->render();
    if(IsMusicStreamPlaying(Global.gameManager->backgroundMusic))
        DrawTextEx(Global.DefaultFont, TextFormat("Playing: %.3f/%.3f", GetMusicTimePlayed(Global.gameManager->backgroundMusic), GetMusicTimeLength(Global.gameManager->backgroundMusic)), {ScaleCordX(5), ScaleCordY(20)}, Scale(15) , Scale(1), WHITE);
    else
        DrawTextEx(Global.DefaultFont, TextFormat("Paused: %.3f/%.3f", GetMusicTimePlayed(Global.gameManager->backgroundMusic), GetMusicTimeLength(Global.gameManager->backgroundMusic)), {ScaleCordX(5), ScaleCordY(20)}, Scale(15) , Scale(1), WHITE);
}

void Game::unload(){

}

WIPMenu::WIPMenu() {
    
}

void WIPMenu::init(){
    std::string temp = Global.Path;
    Global.Path = Global.BeatmapLocation + "/Songs/";
    dir.clear();
    dir = ls(".");
    std::sort(dir.begin(), dir.end());
    Global.Path = temp;
    logo = LoadTexture("resources/osus.png");
    menu = LoadTexture("resources/menu.png");
	SetTextureFilter(logo, TEXTURE_FILTER_BILINEAR );
	//SetTextureFilter(menu, TEXTURE_FILTER_BILINEAR );
}
void WIPMenu::render(){
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
        DrawTextureOnCircle(menu, 800, 240, 300, 0.4f, 0, tempAngle2 - 180.0f, WHITE);
        DrawTextLeft((dir[(tempindex + index + dir.size() + offset) % dir.size()]).c_str(), textpos.x, textpos.y, 9, WHITE);
        DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15, 7, WHITE);
        DrawTextLeft((std::to_string(subObjects.size())).c_str(), textpos.x, textpos.y + 37, 7, WHITE);
        if(selectedIndex == (tempindex + index + dir.size() + offset) % dir.size()){
            DrawTextLeft(("Selected index: " + std::to_string(selectedIndex + offset)).c_str(), textpos.x, textpos.y + 49, 7, GREEN);
        }
    }
    DrawTextureRotate(logo, 800, 240, 0.5f, angle, WHITE);
}
void WIPMenu::update(){
    float clampaccel = 0;
    accel += (float)GetFrameTime() * (float)(100.0f * -Global.Wheel);
    if(accel > 60.0f)
        accel = 60.0f;
    if(accel < -60.0f)
        accel = -60.0f;
    accel += ((-accel) / 2.0f) * ((float)GetFrameTime() * 8.0f);
    if(accel < 0.01f and accel > -0.01f)
        accel = 0.0f;
    float floatangle = ((int)posangle) % 20;
    if(floatangle >= 20.0f)
        floatangle -= 20.0f;
    if(floatangle >= 10.0f)
        clampaccel = (float)GetFrameTime() * (float)(2.5f * (20.0f - floatangle));
    else
        clampaccel = -(float)GetFrameTime() * (float)(2.5f * (floatangle));
    
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
            angle += (Global.MousePosition.y - lastMouse) / -5.0f;
            moving = true;
        }   
    }
    if(!moving and Global.Key1R and CheckCollisionPointRec(Global.MousePosition, Rectangle{305,198,210,81})){
        if(selectedIndex == index){
            selectedIndex = -1;
            subObjects.clear();
            subDir.clear();
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
        if(selectedIndex != -1 and dir[selectedIndex][dir[selectedIndex].size() - 1] == '/'){
            subObjects.clear();
            std::string temp = Global.Path;
            Global.Path = Global.BeatmapLocation + "/Songs/" + dir[selectedIndex];
            std::string loc = dir[selectedIndex];
            subDir.clear();
            subDir = ls(".osu");
            std::cout << "ls?" << std::endl;
            std::sort(subDir.begin(), subDir.end());
            for(int i = 0; i < subDir.size(); i++){
                std::cout << subDir.size() << " " << subDir[i].size() << " " << i << std::endl;
                if(subDir[i][subDir[i].size() - 1] != '/'){
                    subObjects.push_back(std::make_pair(subDir[i], Global.Path + subDir[i]));
                    std::cout << subDir[i] << std::endl;
                }
            }
            std::cout << "done?" << std::endl;
            Global.Path = temp;
        }
    }
    if(Global.Key1R){
        moving = false;
        accel += (Global.MousePosition.y - lastMouse) / -10.0f;
        mouseMovement = 0;
        absMouseMovement = 0;
    }
    if(Global.MouseInFocus and CheckCollisionPointRec(Global.MousePosition, Rectangle{320,-2000,320,6000}))
        lastMouse = Global.MousePosition.y;
    angle += accel;
    if(!moving)
        angle += clampaccel;

}
void WIPMenu::unload(){
    dir.clear();
    subObjects.clear();
    subDir.clear();
    UnloadTexture(logo);
    UnloadTexture(menu);
}

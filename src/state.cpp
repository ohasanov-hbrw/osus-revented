#include "state.hpp"
#include "fs.hpp"
#include <iostream>
#include "globals.hpp"
#include "utils.hpp"
#include "gamemanager.hpp"
#include "zip.h"

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
    dir_list.update();
    select.update();
    back.update();
    close.update();

    if(close.action){
        Global.Path = temp;
        Global.CurrentState.reset(new MainMenu());
    }

    if(select.action){
        if(dir_list.objects.size() > 0 and dir_list.objects[dir_list.selectedindex].text.size() > 0){
            if(dir_list.objects[dir_list.selectedindex].text[dir_list.objects[dir_list.selectedindex].text.size()-1] == '/'){
                dir_list.objects[dir_list.selectedindex].text.pop_back();
                if(Global.Path.size() == 1) Global.Path.pop_back();
                Global.Path += '/' + dir_list.objects[dir_list.selectedindex].text;
                auto dir = ls(".osu");
                dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color, dir, dir_list.textcolor, dir_list.textsize, dir_list.objectsize, dir_list.maxlength);
                dir_list.init();
            }
            else{
                Global.selectedPath = Global.Path + '/' + dir_list.objects[dir_list.selectedindex].text;
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
    dir_list.update();
    select.update();
    back.update();
    close.update();
    
    if(close.action){
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

MainMenu::MainMenu() {
    play = Button({250,420}, {120,60}, {255,135,198,255}, "Play", BLACK, 20);
    load = Button({390,420}, {120,60}, {255,135,198,255}, "Load", BLACK, 20);
}
void MainMenu::init() {}
void MainMenu::update() {
    play.update();
    load.update();
    if(play.action){
        Global.CurrentState.reset(new PlayMenu());
        Global.CurrentState->init();
    }
    else if(load.action){
        Global.CurrentState.reset(new LoadMenu());
    }
}
void MainMenu::render() {
    DrawTextureCenter(Global.OsusLogo, 320, 200, 1/3.f, WHITE);
    play.render();
    load.render();
}

Game::Game() {
    
}
void Game::init() {
    std::cout << Global.selectedPath << std::endl;
    Global.gameManager->loadGame(Global.selectedPath);
    Global.gameManager->timingSettingsForHitObject.sliderSpeedOverride = 1;

}
void Game::update() {
    Global.gameManager->run();
    if(IsKeyPressed(KEY_BACKSPACE)){
        Global.gameManager->unloadGame();
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
    Global.gameManager->render();
    if(IsMusicStreamPlaying(Global.gameManager->backgroundMusic))
        DrawTextEx(Global.DefaultFont, TextFormat("Playing: %.3f/%.3f", GetMusicTimePlayed(Global.gameManager->backgroundMusic), GetMusicTimeLength(Global.gameManager->backgroundMusic)), {ScaleCordX(5), ScaleCordY(20)}, Scale(15) , Scale(1), WHITE);
    else
        DrawTextEx(Global.DefaultFont, TextFormat("Paused: %.3f/%.3f", GetMusicTimePlayed(Global.gameManager->backgroundMusic), GetMusicTimeLength(Global.gameManager->backgroundMusic)), {ScaleCordX(5), ScaleCordY(20)}, Scale(15) , Scale(1), WHITE);
}

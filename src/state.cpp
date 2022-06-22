#include "state.hpp"
#include "fs.hpp"
#include <iostream>
#include "globals.hpp"
#include "utils.hpp"

PlayMenu::PlayMenu() {
    description = TextBox({320,140}, {520,40}, BLUE, "Select a Beatmap\n    to play!", WHITE, 15, 50);
    bg = TextBox({320,240}, {530,290}, BLUE, "", BLUE, 15, 10);
    back = Button({395,360}, {120,40}, PURPLE, "Back", BLACK, 15);
    select = Button({520,360}, {120,40}, PURPLE, "Select", BLACK, 15);
    close = Button({70, 110}, {20,20}, PURPLE, "x", BLACK, 15);
    auto dir = ls();
    dir_list = SelectableList({320, 260}, {520, 150}, PURPLE, dir, BLACK, 10, 15, 50);
    path = TextBox({195,360}, {270,40}, BLUE, Global.Path, WHITE, 8, 40);
}

void PlayMenu::init() {}
void PlayMenu::render() {
    bg.render();
    description.render();
    back.render();
    path.render();
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
        Global.CurrentState.reset(new MainMenu());
    }

    if(select.action){
        if(dir_list.objects.size() > 0 and dir_list.objects[dir_list.selectedindex].text.size() > 0){

            if(dir_list.objects[dir_list.selectedindex].text[dir_list.objects[dir_list.selectedindex].text.size()-1] == '/'){
                dir_list.objects[dir_list.selectedindex].text.pop_back();
                if(Global.Path.size() == 1) Global.Path.pop_back();
                Global.Path += '/' + dir_list.objects[dir_list.selectedindex].text;
                auto dir = ls();
                dir_list = SelectableList({320, 260}, {520, 150}, PURPLE, dir, BLACK, 10, 15, 50);
                dir_list.init();
                path = TextBox({195,360}, {270,40}, BLUE, Global.Path, WHITE, 8, 40);
            }
        }
    }
    if(back.action){
        while(Global.Path.size() > 0){
            if(Global.Path[Global.Path.size()-1] == '/'){
                if(Global.Path.size() > 1)
                    Global.Path.pop_back();
                break;
            }
            Global.Path.pop_back();
        }
        auto dir = ls();
        dir_list = SelectableList({320, 260}, {520, 150}, PURPLE, dir, BLACK, 10, 15, 50);
        dir_list.init();
        path = TextBox({195,360}, {270,40}, BLUE, Global.Path, WHITE, 8, 40);
    }
}

MainMenu::MainMenu() {
    play = Button({250,420}, {120,60}, PURPLE, "Play", BLACK, 20);
    load = Button({390,420}, {120,60}, PURPLE, "Load", BLACK, 20);
}
void MainMenu::init() {}
void MainMenu::update() {
    play.update();
    load.update();
    if(play.action){
        Global.CurrentState.reset(new PlayMenu());
    }
}
void MainMenu::render() {
    DrawTextureCenter(Global.OsusLogo, 320, 200, 1/3.f, WHITE);
    play.render();
    load.render();
}
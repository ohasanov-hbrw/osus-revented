#include "state.hpp"
#include "fs.hpp"
#include <iostream>

PlayMenu::PlayMenu() {
    description = TextBox({320,100}, {540,40}, BLUE, "Selectable List Demo", WHITE, 15, 50);
    bg = TextBox({320,240}, {540,420}, BLUE, "", BLUE, 15, 10);
    back = Button({360,420}, {120,40}, PURPLE, "Back", BLACK, 15);
    select = Button({520,420}, {120,40}, PURPLE, "Select", BLACK, 15);
    close = Button({240, 420}, {120,40}, PURPLE, "Close", BLACK, 15);
    auto dir = ls();
    dir_list = SelectableList({320, 300}, {520, 120}, PURPLE, dir, BLACK, 10, 15, 50);
}

void PlayMenu::init() {}
void PlayMenu::render() {
    bg.render();
    description.render();
    back.render();
    select.render();
    dir_list.render();
}
void PlayMenu::update() {
    dir_list.update();
    select.update();
    back.update();

    if(select.action){
        if(dir_list.objects.size() > 0 and dir_list.objects[dir_list.selectedindex].text.size() > 0){

            if(dir_list.objects[dir_list.selectedindex].text[dir_list.objects[dir_list.selectedindex].text.size()-1] == '/'){
                dir_list.objects[dir_list.selectedindex].text.pop_back();
                Global.Path += '/' + dir_list.objects[dir_list.selectedindex].text;
                auto dir = ls();
                dir_list = SelectableList({320, 300}, {520, 120}, PURPLE, dir, BLACK, 10, 15, 50);
                dir_list.init();
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
        dir_list = SelectableList({320, 300}, {520, 120}, PURPLE, dir, BLACK, 10, 15, 50);
        dir_list.init();
    }
}

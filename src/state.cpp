#include "state.hpp"
#include "fs.hpp"
#include <iostream>
#include "utils.hpp"
#include "gamemanager.hpp"
#include "zip.h"
#include "fastrender.hpp"
#include "rlgl.h"
#include "raylib.h"
#include <dirent.h>
#include <gamefile.hpp>
#include <ctype.h>
#include <cstring>
#include <clocale>
#include "utils.hpp"
#include "time_util.hpp"
#include "settingsParser.hpp"

PlayMenu::PlayMenu() {
    name = TextBox({320,440}, {520,40}, {0,0,0,0}, "BETA VERSION!", WHITE, 20, 50);
    description = TextBox({320,140}, {520,40}, {240,98,161,255}, "Select a Beatmap to play!", WHITE, 20, 50);
    bg = TextBox({320,240}, {530,290}, {240,98,161,255}, "", {240,98,161,255}, 20, 10);
    back = Button({395,360}, {120,40}, {255,135,198,255}, "Back", BLACK, 20);
    select = Button({520,360}, {120,40}, {255,135,198,255}, "Select", BLACK, 20);
    close = Button({70, 110}, {20,20}, {255,135,198,255}, "x", BLACK, 20);
    skin = Switch({310,350}, {40,20}, RED, GREEN, {255,135,198,255}, BLACK);
    sound = Switch({310,370}, {40,20}, RED, GREEN, {255,135,198,255}, BLACK);
    usedskin = TextBox({180,350}, {190,20}, {240,98,161,255}, "Use default skin", WHITE, 20, 50);
    usedsound = TextBox({180,370}, {190,20}, {240,98,161,255}, "Use default sound", WHITE, 20, 50);
    skin.state = Global.settings.useDefaultSkin;
    sound.state = Global.settings.useDefaultSounds;
}

void PlayMenu::init() {
    //MutexLock(SWITCHING_STATE);
    //std::cout << "loading the playmenu/n";
    Global.NeedForBackgroundClear = true;
    Global.useAuto = false;
    Global.LastFrameTime = getTimer();
    temp = Global.Path;
    Global.Path = Global.BeatmapLocation;

    std::vector<std::string> dir = ls(".osu");
    dir_list = SelectableList({320, 250}, {520, 160}, {255,135,198,255}, dir, BLACK, 20, 20, 65);
    //MutexUnlock(SWITCHING_STATE);
}
void PlayMenu::render() {
    //Global.mutex.lock();
    //MutexLock(SWITCHING_STATE);
    //MutexLock(ACCESSING_OBJECTS);
    MutexLock(ACCESSING_OBJECTS);
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
    name.render();
    MutexUnlock(ACCESSING_OBJECTS);
    //MutexUnlock(ACCESSING_OBJECTS);
    //MutexUnlock(SWITCHING_STATE);
    //Global.mutex.unlock();
}
void PlayMenu::update() {
    //MutexLock(SWITCHING_STATE);
    //MutexLock(ACCESSING_OBJECTS);
    MutexLock(ACCESSING_OBJECTS);
    Global.enableMouse = true;
    dir_list.update();
    select.update();
    back.update();
    close.update();
    skin.update();
    sound.update();
    /*if(lastIndex != dir_list.selectedindex){
        name.text = dir_list.objects[dir_list.selectedindex].text;
        name.update();
        lastIndex = dir_list.selectedindex;
    }*/
    if(skin.state != Global.settings.useDefaultSkin)
        Global.settings.useDefaultSkin = skin.state;
    
    if(sound.state != Global.settings.useDefaultSounds)
        Global.settings.useDefaultSounds = sound.state;
    MutexUnlock(ACCESSING_OBJECTS);

    if(close.action){
        Global.Path = temp;
        MutexLock(SWITCHING_STATE);
        Global.CurrentState->unload();
        Global.CurrentState.reset(new MainMenu());
        Global.CurrentState->init();
        MutexUnlock(SWITCHING_STATE);
    }

    if(select.action or dir_list.action){
        if(dir_list.objects.size() > 0 and dir_list.objects[dir_list.selectedindex].text.size() > 0){
            if(dir_list.objects[dir_list.selectedindex].text[dir_list.objects[dir_list.selectedindex].text.size()-1] == '/'){
                MutexLock(ACCESSING_OBJECTS);
                dir_list.objects[dir_list.selectedindex].text.pop_back();
                if(Global.Path.size() == 1) Global.Path.pop_back();
                Global.Path += '/' + dir_list.objects[dir_list.selectedindex].text;
                lastPos = dir_list.objects[dir_list.selectedindex].text;
                auto dir = ls(".osu");
                dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color, dir, dir_list.textcolor, dir_list.textsize, dir_list.objectsize, dir_list.maxlength);
                dir_list.init();
                lastIndex = -3;
                MutexUnlock(ACCESSING_OBJECTS);
            }
            else{
                MutexLock(ACCESSING_OBJECTS);
                Global.selectedPath = Global.Path + '/' + dir_list.objects[dir_list.selectedindex].text;
                Global.CurrentLocation = "beatmaps/" + lastPos + "/";
                MutexUnlock(ACCESSING_OBJECTS);
                MutexLock(SWITCHING_STATE);
                Global.CurrentState->unload();
                Global.CurrentState.reset(new Game());
                Global.CurrentState->init();
                MutexUnlock(SWITCHING_STATE);
            }
        }
    }
    else if(back.action){
        MutexLock(ACCESSING_OBJECTS);
        Global.Path = Global.BeatmapLocation;
        auto dir = ls(".osu");
        dir_list = SelectableList(dir_list.position, dir_list.size, dir_list.color, dir, dir_list.textcolor, dir_list.textsize, dir_list.objectsize, dir_list.maxlength);
        dir_list.init();
        MutexUnlock(ACCESSING_OBJECTS);
    }
    //MutexUnlock(ACCESSING_OBJECTS);
    //MutexUnlock(SWITCHING_STATE);
}
void PlayMenu::unload() {
    //MutexLock(SWITCHING_STATE);
    //MutexUnlock(SWITCHING_STATE);
}
void PlayMenu::textureOps() {

}

LoadMenu::LoadMenu() {
    description = TextBox({320,140}, {520,40}, {240,98,161,255}, "Select a .OSZ Beatmap file\n    to extract!", WHITE, 20, 50);
    bg = TextBox({320,240}, {530,290}, {240,98,161,255}, "", {240,98,161,255}, 20, 10);
    back = Button({395,360}, {120,40}, {255,135,198,255}, "Back", BLACK, 20);
    select = Button({520,360}, {120,40}, {255,135,198,255}, "Select", BLACK, 20);
    close = Button({70, 110}, {20,20}, {255,135,198,255}, "x", BLACK, 20);
    auto dir = ls(".osz");
    dir_list = SelectableList({320, 260}, {520, 150}, {255,135,198,255}, dir, BLACK, 20, 20, 60);
    path = TextBox({195,360}, {270,40}, {240,98,161,255}, Global.Path, WHITE, 20, 40);
}

void LoadMenu::init() {
    //MutexLock(SWITCHING_STATE);
    Global.NeedForBackgroundClear = true;
    Global.useAuto = false;
    Global.LastFrameTime = getTimer();
    Global.FrameTime = 0.5;
    //MutexUnlock(SWITCHING_STATE);
}
void LoadMenu::render() {
    //Global.mutex.lock();
    //MutexLock(SWITCHING_STATE);
    //MutexLock(ACCESSING_OBJECTS);
    MutexLock(ACCESSING_OBJECTS);
    bg.render();
    description.render();
    back.render();
    path.render();
    select.render();
    dir_list.render();
    close.render();
    MutexUnlock(ACCESSING_OBJECTS);
    //MutexUnlock(SWITCHING_STATE);
    //MutexUnlock(ACCESSING_OBJECTS);
    //Global.mutex.unlock();
}
void LoadMenu::update() {
    //MutexLock(SWITCHING_STATE);
    //MutexLock(ACCESSING_OBJECTS);
    MutexLock(ACCESSING_OBJECTS);
    Global.enableMouse = true;
    dir_list.update();
    select.update();
    back.update();
    close.update();
    MutexUnlock(ACCESSING_OBJECTS);
    
    if(close.action){
        MutexLock(SWITCHING_STATE);
        Global.CurrentState->unload();
        Global.CurrentState.reset(new MainMenu());
        Global.CurrentState->init();
        MutexUnlock(SWITCHING_STATE);
    }
    MutexLock(ACCESSING_OBJECTS);
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
                //std::cout << Global.selectedPath.c_str() << std::endl;
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
    MutexUnlock(ACCESSING_OBJECTS);
    //MutexUnlock(SWITCHING_STATE);
    //MutexUnlock(ACCESSING_OBJECTS);
}
void LoadMenu::unload() {
    //MutexLock(SWITCHING_STATE);
    //MutexUnlock(SWITCHING_STATE);
}
void LoadMenu::textureOps() {

}

MainMenu::MainMenu() {
    play = Button({250,420}, {120,60}, {255,135,198,255}, "Play", BLACK, 20);
    wip = Button({320,340}, {120,40}, {255,135,198,0}, "WIP", BLACK, 20);
    wip2 = Button({320,300}, {120,40}, {255,135,198,0}, "WIP2", BLACK, 20);
    load = Button({390,420}, {120,60}, {255,135,198,255}, "Load", BLACK, 20);
    volume = TestSlider({510,460}, {240,20}, BLACK, PURPLE, WHITE, WHITE);
}

void MainMenu::init() {
    //MutexLock(SWITCHING_STATE);
    Global.NeedForBackgroundClear = true;
    Global.LastFrameTime = getTimer();
    Global.FrameTime = 0.5;
    Global.useAuto = false;
    volume.location = Global.volume * 100.0f;
    setlocale(LC_ALL, "en_US.utf8");
    //MutexUnlock(SWITCHING_STATE);
}

void toupper(std::string &s) {
  for (char &c : s)
    c = std::toupper(c);
}
bool strcasecmp2(std::string lhs, std::string rhs) {
  toupper(lhs); toupper(rhs);
  return lhs < rhs;
}

void MainMenu::update() {

    //MutexLock(SWITCHING_STATE);
    //MutexLock(ACCESSING_OBJECTS);
    MutexLock(ACCESSING_OBJECTS);
    Global.enableMouse = true;
    play.update();
    wip.update();
    wip2.update();
    load.update();
    MutexUnlock(ACCESSING_OBJECTS);
    //test.update();
    if(wip.action){
        //Global.CurrentState->unload();
        //Global.CurrentState.reset(new WIPMenu());
        //Global.CurrentState->init();
        MutexLock(ACCESSING_OBJECTS);
        std::string temp = Global.Path;
        Global.Path = Global.GamePath + "/database";

        struct dirent *de;
        DIR *dr = opendir(Global.Path.c_str()); 
        if (dr == NULL){ // opendir returns NULL if couldn't open directory { 
            printf("Could not open current directory: database" ); 
        }
        else{
            while ((de = readdir(dr)) != NULL) {
                 std::remove((Global.Path + "/" + de->d_name).c_str());
            }
            closedir(dr);
        }

        Global.Path = temp;
        Parser parser = Parser();
        for(const auto& p: std::filesystem::recursive_directory_iterator(Global.BeatmapLocation)){
            if(!std::filesystem::is_directory(p)){
                if(p.path().extension().string() == ".osu"){
                    //std::cout << p.path().string() << '\n';
                    GameFile geym;
                    geym = parser.parseMetadata(p.path().string());
                    std::string GameTitle;
                    std::string GameSetId;
                    std::string GameVersion;
                    std::string GameAuthor;

                    if(geym.configMetadata.find("BeatmapSetID") == geym.configMetadata.end()){
                        std::cout << "didnt find setid bro, tryin to improvise\n";
                        std::string doublecheck = p.path().parent_path().filename().string();
                        int index = 0;
                        bool works = false;
                        bool startswithnumber = isdigit(doublecheck.at(0));
                        int number = 0;
                        
                        while(true){
                            if(index >= doublecheck.length()){
                                break;
                            }
                            if(!isdigit(doublecheck.at(index))){
                                if(doublecheck[index] == ' '){
                                    if(startswithnumber){
                                        works = true;
                                        break;
                                    }
                                }
                            }
                            else{
                                number *= 10;
                                number += int(doublecheck[index] - '0');
                            }
                            index++;
                            //std::cout << "number: " << number << std::endl;
                        }
                        GameSetId = std::to_string(number);
                    }
                    else{
                        GameSetId = geym.configMetadata["BeatmapSetID"];
                    }

                    if(geym.configMetadata.find("Title") == geym.configMetadata.end()){
                        std::cout << "didnt title bro, setting as Unknown\n";
                        GameTitle = "Unknown";
                    }
                    else{
                        GameTitle = geym.configMetadata["Title"];
                    }

                    if(geym.configMetadata.find("Artist") == geym.configMetadata.end()){
                        std::cout << "didnt find artist bro, setting as Unknown\n";
                        GameAuthor = "Unknown";
                    }
                    else{
                        GameAuthor = geym.configMetadata["Artist"];
                    }

                    std::string filename = Global.GamePath + "/database/" + GameTitle + " {" + GameSetId + "}.db";
                    bool firstLine = !checkIfExists((filename).c_str());
                    FILE * pFile;
                    pFile = fopen((filename).c_str()  ,"a");
                    if(pFile != NULL){
                        if(firstLine){
                            fprintf(pFile, "---[METADATA]---\n");
                            fprintf(pFile, (GameTitle + "\n").c_str());
                            fprintf(pFile, (GameAuthor + "\n").c_str());
                            fprintf(pFile, (GameSetId + "\n").c_str());
                            fprintf(pFile, (p.path().parent_path().string() + "/" + "\n").c_str());
                            fprintf(pFile, (parser.parseBackground(p.path().string()) + "\n").c_str());
                            fprintf(pFile, "---[FILES]---\n");
                        }
                        fprintf(pFile, (p.path().string() + "\n").c_str());
                        fclose(pFile);
                    }
                }
            }
        }

        Global.Path = Global.GamePath + "/database";

        std::vector<std::string> files;
        files = ls(".db");
        std::sort(files.begin(), files.end(), strcasecmp2);
        std::string filename = Global.GamePath + "/database/mainFolder.db";
        FILE * pFile;
        pFile = fopen((filename).c_str()  ,"a");
        if(pFile != NULL){
            for(int i = 0; i < files.size(); i++){
                fprintf(pFile, (files[i] + "\n").c_str());
            }
            fclose(pFile);
        }
        Global.Path = temp;
        MutexUnlock(ACCESSING_OBJECTS);
        return;
    }
    else if(play.action){
        
        MutexLock(SWITCHING_STATE);
        //std::cout << "play.action unload" << std::endl;
        Global.CurrentState->unload();
        //std::cout << "play.action reset" << std::endl;
        Global.CurrentState.reset(new PlayMenu());
        //std::cout << "play.action init" << std::endl;
        Global.CurrentState->init();
        //std::cout << "play.action done" << std::endl;
        MutexUnlock(SWITCHING_STATE);
        return;
    }
    else if(load.action){
        
        MutexLock(SWITCHING_STATE);
        Global.CurrentState->unload();
        Global.CurrentState.reset(new LoadMenu());
        Global.CurrentState->init();
        MutexUnlock(SWITCHING_STATE);
        return;
    }
    else if(wip2.action){
        
        MutexLock(SWITCHING_STATE);
        Global.CurrentState->unload();
        Global.CurrentState.reset(new WipMenu2());
        Global.CurrentState->init();
        MutexUnlock(SWITCHING_STATE);
        return;
    }

    if(IsKeyDown(Global.AUDIO_SETUP_KEY))
        volume.update();
    float lastVolume = Global.volume;
    Global.volume = volume.location / 100.0f;
    if(!AreSame(lastVolume, Global.volume)){
        //std::cout << "Volume: " << Global.volume << std::endl;
        Global.volumeChanged = true;
    }
    //MutexUnlock(SWITCHING_STATE);
    //MutexUnlock(ACCESSING_OBJECTS);
}
void MainMenu::render() {
    //Global.mutex.lock();
    //MutexLock(SWITCHING_STATE);
    //MutexLock(ACCESSING_OBJECTS);
    MutexLock(ACCESSING_OBJECTS);
    DrawTextureCenter(&Global.OsusLogo, 320, 200, 400.0 / (float)Global.OsusLogo.width, WHITE);
    play.render();
    wip.render();
    wip2.render();
    load.render();
    MutexUnlock(ACCESSING_OBJECTS);
    if(IsKeyDown(Global.AUDIO_SETUP_KEY ))
        volume.render();
    //MutexUnlock(SWITCHING_STATE);
    //MutexUnlock(ACCESSING_OBJECTS);
    //test.render();
    //Global.mutex.unlock();
}
void MainMenu::unload() {
    //MutexLock(SWITCHING_STATE);
    //MutexUnlock(SWITCHING_STATE);
}
void MainMenu::textureOps() {
    //NEED FIXING>>> UNLOADING DOESNT STOP RENDEWRING
}

Game::Game() {
    volume = TestSlider({510,460}, {240,20}, BLACK, PURPLE, WHITE, WHITE);
}

void Game::init() {
    //MutexLock(SWITCHING_STATE);
    Global.NeedForBackgroundClear = true;
    Global.useAuto = false;
    initDone = 0;
    Global.LastFrameTime = getTimer();
    //std::cout << Global.selectedPath << std::endl;

    Global.GameTextures = 0;
    Global.numberLines = 0;
    Global.parsedLines = 0;
    Global.loadingState = 0;
    initDone = -2;
    initStartTime = getTimer();
    //Global.mutex.unlock();
    //LightLock_Unlock(&Global.lightlock);
    //MutexUnlock(SWITCHING_STATE); 
    //While loading the game chaos can happen, no problem
    
    MutexUnlock(SWITCHING_STATE);
    parseSettings();
    Global.gameManager->loadGame(Global.selectedPath);
    MutexLock(ACCESSING_OBJECTS);
    MutexUnlock(ACCESSING_OBJECTS);
    MutexLock(SWITCHING_STATE);
    //MutexLock(SWITCHING_STATE);
    Global.gameManager->timingSettingsForHitObject.clear();
    //Global.mutex.lock();
    //LightLock_Lock(&Global.lightlock);
    Global.startTime = -5000.0f;
    Global.errorSum = 0;
    Global.errorLast = 0;
    Global.errorDiv = 0;
    
    volume.location = Global.volume * 100.0f;
    //MutexLock(SWITCHING_STATE);
    
}
void Game::update() {

    if(IsKeyDown(Global.AUDIO_SETUP_KEY ))
        volume.update();
    float lastVolume = Global.volume;
    Global.volume = volume.location / 100.0f;
    if(!AreSame(lastVolume, Global.volume)){
        //std::cout << "Volume: " << Global.volume << std::endl;
        Global.volumeChanged = true;
    }

    if(initDone == 1){
        //Global.enableMouse = false;
        MutexLock(ACCESSING_OBJECTS);
        if(IsKeyPressed(Global.GO_BACK_KEY) || !(!WindowShouldClose() and _os_should_program_run())){
            Global.CurrentState->initDone = 3;
            MutexUnlock(ACCESSING_OBJECTS);

			MutexLock(RENDER_BLOCK);
            MutexLock(ACCESSING_OBJECTS);
            MutexUnlock(RENDER_BLOCK);

			MutexLock(SWITCHING_STATE);
            std::cout << "locked the switching state\n";
            Global.CurrentState->unload();
            MutexUnlock(ACCESSING_OBJECTS);
            Global.CurrentState.reset(new PlayMenu());
            Global.CurrentState->init();
            MutexUnlock(SWITCHING_STATE);
            return;
        }
        Global.gameManager->run();
        MutexUnlock(ACCESSING_OBJECTS);
    }
    else{
        if(initDone == 0 or Global.GameTextures == 0){
            initDone = -1;
        }
        if(initDone == -1 and getTimer() - initStartTime > 0.0f){
            std::cout << "init done in " << getTimer() - initStartTime << " secs\n";
            initDone = true;
        }
    }

}
void Game::render() {
    
    if(initDone == 1){
        //Global.enableMouse = false;
        MutexLock(ACCESSING_OBJECTS);
        Global.gameManager->render();
        //Global.mutex.lock();
        if(IsMusicStreamPlaying(&Global.gameManager->backgroundMusic)){
            DrawTextEx(&Global.DefaultFont, TextFormat("Playing: %.3f/%.3f", (Global.currentOsuTime/1000.0), GetMusicTimeLength(&Global.gameManager->backgroundMusic)), {(int)ScaleCordX(5), (int)ScaleCordY(25)}, Scale(20.05) , Scale(2), WHITE);
            //DrawTextEx(Global.DefaultFont, TextFormat("Timer: %.3f ms", getTimer()), {ScaleCordX(5), ScaleCordY(55)}, Scale(10) , Scale(1), WHITE);
            //DrawTextEx(Global.DefaultFont, TextFormat("Last Error: %.3f ms", Global.errorLast/1000.0f), {ScaleCordX(5), ScaleCordY(65)}, Scale(10) , Scale(1), WHITE);
            //DrawTextEx(Global.DefaultFont, TextFormat("Avg Time Difference in the First Second: %.3f ms", Global.avgTime), {ScaleCordX(5), ScaleCordY(75)}, Scale(10) , Scale(1), WHITE);
        }
        else{
            DrawTextEx(&Global.DefaultFont, TextFormat("Paused: %.3f/%.3f", GetMusicTimePlayed(&Global.gameManager->backgroundMusic) * 1000000.0f, GetMusicTimeLength(&Global.gameManager->backgroundMusic)), {(int)ScaleCordX(5), (int)ScaleCordY(25)}, Scale(20.05) , Scale(2), WHITE);
            if(Global.errorDiv != 0)
                DrawTextEx(&Global.DefaultFont, TextFormat("Error Avg: %ld ms", (Global.errorSum/Global.errorDiv)/1000), {(int)ScaleCordX(5), (int)ScaleCordY(40)}, Scale(20.05) , Scale(2), WHITE);
        }
        if(GetMusicTimeLength(&Global.gameManager->backgroundMusic) != 0){
            DrawLineEx({0, GetScreenHeight() - Scale(2)}, {GetScreenWidth() * ((Global.currentOsuTime/1000.0) / GetMusicTimeLength(&Global.gameManager->backgroundMusic)), GetScreenHeight() - Scale(2)}, Scale(3), Fade(WHITE, 0.8));
        }
        MutexUnlock(ACCESSING_OBJECTS);

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
        DrawTextEx(&Global.DefaultFont, message.c_str(), {(int)ScaleCordX(320 - message.size() * 7.5f), (int)ScaleCordY(220)}, Scale(20.05), Scale(2), WHITE);
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
            message = "Parsing Sounds";
        }
        else if(Global.loadingState == 7){
            message = "Loading Textures";
        }
        DrawTextEx(&Global.DefaultFont, message.c_str(), {(int)ScaleCordX(320 - message.size() * 7.5f), (int)ScaleCordY(220)}, Scale(20.05), Scale(2), WHITE);
        //Global.mutex.unlock();
    }
    if(IsKeyDown(Global.AUDIO_SETUP_KEY ))
        volume.render();
}
void Game::unload(){
    //MutexLock(SWITCHING_STATE);
    //MutexLock(ACCESSING_OBJECTS);
    Global.gameManager->unloadGame();
    Global.NeedForBackgroundClear = true;
    //MutexUnlock(ACCESSING_OBJECTS);
    //MutexUnlock(SWITCHING_STATE);
}
void Game::textureOps(){
    //std::cout << "Trying to acquire Lock for accessing objects\n";
    MutexLock(ACCESSING_OBJECTS);
    //std::cout << "Got Permission!\n";
    if(Global.GameTextures == -1){
        Global.gameManager->unloadGameTextures();
    }
    else if(Global.GameTextures == 1)
        Global.gameManager->loadGameTextures();

    if(Global.sliderTexNeedDeleting)
        Global.gameManager->unloadSliderTextures();
    MutexUnlock(ACCESSING_OBJECTS);
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
	SetTextureFilter(&logo, TEXTURE_FILTER_BILINEAR );
    SetTextureFilter(&back, TEXTURE_FILTER_BILINEAR );
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
        _gpu_start_drawing(Global.window);
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
            DrawTextureOnCircle(&menu, 800, 240, 300*weirdSmooth, 0.4f, 0, tempAngle2 - 180.0f, temp);
            DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
            DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15*weirdSmooth, 7, WHITE);
        }
        DrawTextureRotate(&logo, 800, 240, 0.5f, angle, WHITE);

        DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
        DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
        renderMouse();
        DrawTextEx(&Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {(int)ScaleCordX(5), (int)ScaleCordY(5)}, Scale(20.05), Scale(2), GREEN);
        _gpu_end_drawing();
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
        DrawTextureOnCircle(&menu, 800, 240, 300, 0.4f, 0, tempAngle2 - 180.0f, temp);
        DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
        DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15, 7, WHITE);
    }
    
    DrawTextureCenter(&back, 145, 240, 0.45f, Color{255,255,255,255 * easeInOutCubic(animtime)});
    if(TempMeta.size() == 5){
        DrawTextLeft((TempMeta[0]).c_str(), 25, 55, 9, Color{255,255,255,255 * easeInOutCubic(animtime)});
        DrawTextLeft((TempMeta[1]).c_str(), 25, 70, 9, Color{255,255,255,255 * easeInOutCubic(animtime)});
        DrawTextLeft((TempMeta[2]).c_str(), 25, 85, 9, Color{255,255,255,255 * easeInOutCubic(animtime)});
        DrawTextLeft((TempMeta[3]).c_str(), 25, 100, 9, Color{255,255,255,255 * easeInOutCubic(animtime)});
        DrawTextLeft((TempMeta[4]).c_str(), 25, 115, 9, Color{255,255,255,255 * easeInOutCubic(animtime)});
    }

    DrawTextureRotate(&logo, 800, 240, 0.5f, angle, WHITE);
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
                    _gpu_start_drawing(Global.window);
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
                        DrawTextureOnCircle(&menu, 800, 240, 300*weirdSmooth, 0.4f, 0, tempAngle2 - 180.0f, temp);
                        DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
                        DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15*weirdSmooth, 7, WHITE);
                    }
                    DrawTextureRotate(&logo, 800, 240, 0.5f, angle, WHITE);

                    DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
                    DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
                    renderMouse();
                    DrawTextEx(&Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {(int)ScaleCordX(5), (int)ScaleCordY(5)}, Scale(20.05), Scale(2), GREEN);
                    _gpu_end_drawing();
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
                //std::cout << Path << std::endl;
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
                    _gpu_start_drawing(Global.window);
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
                        DrawTextureOnCircle(&menu, 800, 240, 300*weirdSmooth, 0.4f, 0, tempAngle2 - 180.0f, temp);
                        DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
                        DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15*weirdSmooth, 7, WHITE);
                    }
                    DrawTextureRotate(&logo, 800, 240, 0.5f, angle, WHITE);

                    DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
                    DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
                    renderMouse();
                    DrawTextEx(&Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {(int)ScaleCordX(5), (int)ScaleCordY(5)}, Scale(20.05), Scale(2), GREEN);
                    _gpu_end_drawing();
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

    if(IsKeyPressed(Global.GO_BACK_KEY) and CanGoBack){
        Path.pop_back();
        while(Path[Path.size()-1] != '/'){
            Path.pop_back();
        }
        if(Path.size() <= (Global.BeatmapLocation + "/").size()){
            Path = Global.BeatmapLocation + "/";
        }
        selectedIndex = -1;
        //std::cout << Path << std::endl;
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
            _gpu_start_drawing(Global.window);
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
                DrawTextureOnCircle(&menu, 800, 240, 300*weirdSmooth, 0.4f, 0, tempAngle2 - 180.0f, temp);
                DrawTextLeft((dir[dirNum]).c_str(), textpos.x, textpos.y, 9, WHITE);
                DrawTextLeft((std::to_string((tempindex + index + dir.size() + offset) % dir.size() + 1) + " out of " + std::to_string(dir.size())).c_str(), textpos.x, textpos.y + 15*weirdSmooth, 7, WHITE);
            }
            DrawTextureRotate(&logo, 800, 240, 0.5f, angle, WHITE);

            DrawRectangle(ScaleCordX(580), ScaleCordY(450), Scale(20), Scale(20),(Color) {0, (unsigned char)(255 * (int)Global.Key1P), (unsigned char)(255 * (int)Global.Key1D), 100});
            DrawRectangle(ScaleCordX(610), ScaleCordY(450), Scale(20), Scale(20), (Color){0, (unsigned char)(255 * (int)Global.Key2P), (unsigned char)(255 * (int)Global.Key2D), 100});
            renderMouse();
            DrawTextEx(&Global.DefaultFont, TextFormat("FPS: %d",  GetFPS()), {(int)ScaleCordX(5), (int)ScaleCordY(5)}, Scale(20.05), Scale(2), GREEN);
            _gpu_end_drawing();
        }
        applyMouse = true;
        init();
    }
    if(IsKeyPressed(Global.GO_BACK_KEY ) and !CanGoBack){
        MutexLock(SWITCHING_STATE);
        Global.CurrentState->unload();
        Global.CurrentState.reset(new MainMenu());
        Global.CurrentState->init();
        MutexUnlock(SWITCHING_STATE);
    }

}
void WIPMenu::unload(){
    //MutexLock(SWITCHING_STATE);
    dir.clear();
    subObjects.clear();
    UnloadTexture(&logo);
    UnloadTexture(&back);
    UnloadTexture(&menu);
    //MutexUnlock(SWITCHING_STATE);
}
void WIPMenu::textureOps(){
    
}


ResultsMenu::ResultsMenu() {
    name = TextBox({320,40}, {520,60}, {0,0,0,0}, "Results!", WHITE, 40, 50);
    close = Button({480, 440}, {160,30}, {255,135,198,255}, "Back to menu", BLACK, 20);
    maxCombo = TextBox({320,80}, {520,60}, {0,0,0,0}, "Maximum Combo: Not available", WHITE, 20, 50);
    hit300 = TextBox({320,120}, {520,60}, {0,0,0,0}, "300s hit: Not available", WHITE, 20, 50);
    hit100 = TextBox({320,160}, {520,60}, {0,0,0,0}, "100s hit: Not available", WHITE, 20, 50);
    hit50 = TextBox({320,200}, {520,60}, {0,0,0,0}, "50s hit: Not available", WHITE, 20, 50);
    hit0 = TextBox({320,240}, {520,60}, {0,0,0,0}, "Misses: Not available", WHITE, 20, 50);
    accuracy = TextBox({320,280}, {520,60}, {0,0,0,0}, "Accuracy: Not available", WHITE, 20, 50);
}

void ResultsMenu::init() {
    //std::cout << "loading the scores" << std::endl;;
    //std::cout << "Maximum Combo: " + std::to_string(Global.gameManager->maxCombo) << std::endl;
    maxCombo.text = "Maximum Combo: " + std::to_string(Global.gameManager->maxCombo);
    maxCombo.init();
    hit300.text = "300s hit: " + std::to_string(Global.gameManager->hit300s);
    hit300.init();
    hit100.text = "100s hit: " + std::to_string(Global.gameManager->hit100s);
    hit100.init();
    hit50.text = "50s hit: " + std::to_string(Global.gameManager->hit50s);
    hit50.init();
    hit0.text = "0s hit: " + std::to_string(Global.gameManager->hit0s);
    hit0.init();
    float accuracy = ((300.0f * Global.gameManager->hit300s + 10.0f * Global.gameManager->hit100s + 50.0f * Global.gameManager->hit50s) 
    / (300.0f * (Global.gameManager->hit300s + Global.gameManager->hit100s + Global.gameManager->hit50s + Global.gameManager->hit0s))) * 100.0f;
    //accuracy = std::ceil(accuracy * 100.0) / 100.0;
    hit0.text = "Accuracy: " + std::to_string(accuracy) + "%";
    hit0.init();
    Global.NeedForBackgroundClear = true;
    Global.useAuto = false;
    Global.LastFrameTime = getTimer();
    
}
void ResultsMenu::render() {
    MutexLock(ACCESSING_OBJECTS);
    close.render();
    name.render();
    maxCombo.render();
    hit300.render();
    hit100.render();
    hit50.render();
    hit0.render();
    MutexUnlock(ACCESSING_OBJECTS);
    //Global.mutex.unlock();
}
void ResultsMenu::update() {
    MutexLock(ACCESSING_OBJECTS);
    close.update();
    MutexUnlock(ACCESSING_OBJECTS);
    if(close.action){
        MutexLock(SWITCHING_STATE);
        Global.CurrentState->unload();
        Global.CurrentState.reset(new PlayMenu());
        Global.CurrentState->init();
        MutexUnlock(SWITCHING_STATE);
    }
}
void ResultsMenu::unload() {
    //MutexLock(SWITCHING_STATE);
    //MutexUnlock(SWITCHING_STATE);
}
void ResultsMenu::textureOps() {

}


WipMenu2::WipMenu2() {

}

void WipMenu2::init() {
    position = minimumPosition;
    const std::lock_guard<std::mutex> lock(scaryMulti);
    for(int i = 0; i < 16; i++){
        MenuItem tempItem;
        tempItem.location = i * 40;
        tempItem.folder = true;
        tempItem.folderID = i;
        locations.push_back(tempItem);
        folderNames.push_back("Folder: " + std::to_string(i));
    }
    for(int i = 0; i < 6; i++){
        itemNames.push_back("Item: " + std::to_string(i));
    }
    maximumPosition = minimumPosition + locations.back().location;
    //std::cout << "initilized the wip2 menu" << std::endl;
    removeStuffAt = -1;
    addStuffAt = -1;
    lastStuffAt = -1;
    canAddStuff = true;
    canRemoveStuff = false;
}
void WipMenu2::update() {
    MutexLock(ACCESSING_OBJECTS);
    if(Global.Key2P && addStuffAt == -1 && removeStuffAt == -1){
        removeStuffAt = lastStuffAt;
        addStuffAt = (lastStuffAt + 1) % 16;
    }

    bool addClamping = true;
    accel += (float)(Global.FrameTime / 1000.0f) * (float)(120.0f * -Global.Wheel);
    if(accel > 120.0f)
        accel = 120.0f;
    if(accel < -120.0f)
        accel = -120.0f;
    if(accel < 0.01f and accel > -0.01f){
        accel = 0.0f;
    }
    if(!(accel < 0.05f and accel > -0.05f)){
        addClamping = false;
    }
    
    float clampAccel = 0;
    float clampingPosition = graphicalPosition;
    if(addClamping){
        while(clampingPosition <= 0.0f)
            clampingPosition += 40.0f;
        while(clampingPosition >= 40.0f)
            clampingPosition -= 40.0f;
        if(clampingPosition >= 20.0f){
            clampAccel = (float)(Global.FrameTime / 1000.0f) * (float)(10.0f * (40.0f - clampingPosition));
            if(clampingPosition + clampAccel >= 40.0f){
                clampAccel = 40.0f - clampingPosition;
            }
        }
        else{
            clampAccel = -(float)(Global.FrameTime / 1000.0f) * (float)(10.0f * (clampingPosition));
            if(clampingPosition - clampAccel <= 0.0f){
                clampAccel = -clampingPosition;
            }
        }
    }
    if(Global.Key1P){
        lastMouse = Global.MousePosition.y;
    }
    if(Global.Key1D and Global.MouseInFocus and CheckCollisionPointRec(Global.MousePosition, Rectangle{320,-2000,320,6000})){
        accel = (Global.MousePosition.y - lastMouse) / -1.0f;
        addClamping = false;
    }
    if(Global.Key1R and Global.MouseInFocus and CheckCollisionPointRec(Global.MousePosition, Rectangle{320,-2000,320,6000})){
        accel += (Global.MousePosition.y - lastMouse) / -1.0f;
    }

    if(Global.MouseInFocus and CheckCollisionPointRec(Global.MousePosition, Rectangle{320,-2000,320,6000}))
        lastMouse = Global.MousePosition.y;


    graphicalPosition += accel;
    if(addClamping)
        graphicalPosition += clampAccel;

    if(graphicalPosition < minimumPosition){
        graphicalPosition = minimumPosition;
        accel = 0; 
        clampAccel = 0;
    }
    if(graphicalPosition > maximumPosition){
        graphicalPosition = maximumPosition; 
        accel = 0; 
        clampAccel = 0;
    }


    position = graphicalPosition;
    if(position < minimumPosition){
        position = minimumPosition; 
    }
    if(position > maximumPosition){
        position = maximumPosition; 
    }
    

    accel += ((-accel) / 2.0f) * ((float)(Global.FrameTime / 1000.0f) * 8.0f);
    if(removeStuffAt >= 0 and canRemoveStuff == true){
        auto beginIt = locations.begin();
        int i = 0;
        while(true){
            if(beginIt == locations.end())
                break;
            if(i >= removeStuffAt)
                break;
            beginIt++;
            i++;
        }
        //std::cout << "among us: " << i << std::endl;
        int removedItems = 0;
        while(true){
            auto locationIt = beginIt;
            if(beginIt != locations.end()){
                locationIt++;
            }
            else{
                break;
            }
            if(locationIt == locations.end()){
                break;
            }
            if((*locationIt).folder == true){
                break;
            }
            removedItems += 40;
            locations.erase(locationIt);
        }
        while(true){
            if(beginIt == locations.end())
                break;
            if(i > removeStuffAt){
                (*beginIt).location -= removedItems;
            }
            beginIt++;
            i++;
        }
        maximumPosition = minimumPosition + locations.back().location;
        if(graphicalPosition > maximumPosition){
            graphicalPosition = maximumPosition; 
        }
        position = graphicalPosition;
        if(position < minimumPosition){
            position = minimumPosition; 
        }
        if(position > maximumPosition){
            position = maximumPosition; 
        }
        canAddStuff = true;
        canRemoveStuff = false;
        removeStuffAt = -1;
    }
    if(addStuffAt >= 0 and canAddStuff == true){
        auto beginIt = locations.begin();
        int i = 0;
        while(true){
            if(beginIt == locations.end())
                break;
            if(i >= addStuffAt)
                break;
            beginIt++;
            i++;
        }
        //std::cout << "among us: " << i << std::endl;
        int locationToAdd = (*beginIt).location;

        //std::cout << "locationToAdd: " << locationToAdd << std::endl;
        int addedItems = 0;
        
        int addNumber = (rand() % 6) + 1;
        locationToAdd += 40 * addNumber;
        for(int j = 0; j < addNumber; j++){
            auto locationIt = beginIt;
            if(beginIt != locations.end()){
                locationIt++;
            }
            MenuItem tempItem;
            tempItem.location = locationToAdd;
            addedItems += 40;
            locationToAdd -= 40;
            tempItem.folder = false;
            tempItem.folderID = (*beginIt).folderID;
            tempItem.itemID = j;
            locations.insert(locationIt, tempItem);
        }
        int changedI = i + addNumber;
        while(true){
            if(beginIt == locations.end())
                break;
            if(i > changedI){
                (*beginIt).location += addedItems;
            }
            beginIt++;
            i++;
        }
        maximumPosition = minimumPosition + locations.back().location;
        if(graphicalPosition > maximumPosition){
            graphicalPosition = maximumPosition; 
        }
        position = graphicalPosition;
        if(position < minimumPosition){
            position = minimumPosition; 
        }
        if(position > maximumPosition){
            position = maximumPosition; 
        }
        canAddStuff = false;
        canRemoveStuff = true;
        lastStuffAt = addStuffAt;
        addStuffAt = -1;
    }
    
    MutexUnlock(ACCESSING_OBJECTS);
}
void WipMenu2::render(){
    MutexLock(ACCESSING_OBJECTS);
    Rectangle rect;
    rect.x = 322;
    rect.y = 240 -  18;
    rect.width =  2;
    rect.height = 36;
    DrawRectangleRec(ScaleRect(rect), {200, 120, 200, 128});
   
    rect.x = 5;
    rect.y = 5;
    rect.width =  315;
    rect.height = 470;

    DrawRectangleRec(ScaleRect(rect), {70, 30, 70, 170});
    for(MenuItem n : locations){
        Rectangle r;
        if(n.folder){
            r.x = 325;
            r.y = (n.location - 18) - graphicalPosition;
            r.width = 315;
            r.height = 36;
        }
        else{
            r.x = 330;
            r.y = (n.location - 18) - graphicalPosition;
            r.width = 310;
            r.height = 36;
        }
        if(r.y + r.width < -10 or r.y > 490)
            continue;
        if(n.folder){
            DrawRectangleRec(ScaleRect(r), {255, 255, 255, 255});
            std::string name = "unknown";
            if(n.folderID >= 0  && n.folderID < folderNames.size()){
                name = folderNames[n.folderID];
            }
            //DrawTextLeft(name.c_str(), r.x + 10, r.y + 18, 20.05, BLACK);
            DrawTextEx(&Global.DefaultFont, name.c_str(), {(int)ScaleCordX(r.x + 10), (int)ScaleCordY(r.y + 18 - 10)}, Scale(20.05), Scale(2), BLACK);
        }
        else{
            DrawRectangleRec(ScaleRect(r), {200, 150, 200, 255});
            std::string name = "unknown";
            if(n.itemID >= 0  && n.itemID < itemNames.size()){
                name = itemNames[n.itemID];
            }
            //DrawTextLeft(name.c_str(), r.x + 10, r.y + 18, 20.05, BLACK);
            DrawTextEx(&Global.DefaultFont, name.c_str(), {(int)ScaleCordX(r.x + 10), (int)ScaleCordY(r.y + 18 - 10)}, Scale(20.05), Scale(2), BLACK);
        }
    }
    MutexUnlock(ACCESSING_OBJECTS);
}
void WipMenu2::unload() {
    //MutexLock(SWITCHING_STATE);
    MutexLock(ACCESSING_OBJECTS);
    locations.clear();
    locations = std::list<MenuItem>();

    folderNames.clear();
    folderNames = std::vector<std::string>();

    itemNames.clear();
    itemNames = std::vector<std::string>();
    MutexUnlock(ACCESSING_OBJECTS);
    //MutexUnlock(SWITCHING_STATE);
}
void WipMenu2::textureOps() {
    locations.clear();
    locations = std::list<MenuItem>();

    folderNames.clear();
    folderNames = std::vector<std::string>();

    itemNames.clear();
    itemNames = std::vector<std::string>();
}
#include "SDLutils.hpp"

void initSDL(){
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_GAMECONTROLLER);
    SDL_Init(SDL_INIT_EVENTS);
    SDL_Init(SDL_INIT_JOYSTICK);
    SDL_Init(SDL_INIT_VIDEO);
    Global.win = SDL_CreateWindowFrom(GetWindowHandle());
    /*Global.win = SDL_CreateWindow("what the hell",
                              10, 10, 10,
                              10, SDL_WINDOW_BORDERLESS);*/
    //SDL_HideWindow(Global.win);
    SDL_SetWindowInputFocus(Global.win);
    //std::cout << SDL_GetWindowFlags(Global.win) << std::endl;
    //SDL_CaptureMouse(SDL_TRUE);
    std::cout << SDL_GetError() << std::endl;
    //SDL_ShowCursor(SDL_DISABLE);
}
void quitSDL(){
    SDL_Quit();
}
void SDLpollEvent(){
    SDL_PumpEvents();
    Global.Input.k1p = false;
    Global.Input.k2p = false;
    Global.Input.k1r = false;
    Global.Input.k2r = false;
    auto KeyboardState = SDL_GetKeyboardState(nullptr);
    SDL_Event Event;
    while(SDL_PollEvent(&Event)){
        if(Event.type == SDL_MOUSEMOTION){
            //SDL_GetMouseState(&Global.Input.mx, &Global.Input.my);
            Global.Input.mx = Event.motion.x;
            Global.Input.my = Event.motion.y;
        }
        else if(Event.type == SDL_MOUSEBUTTONDOWN){
            if(Event.button.button == SDL_BUTTON_LEFT){
                Global.Input.k1p = true;
                Global.Input.k1d = true;
            }
            if(Event.button.button == SDL_BUTTON_RIGHT){
                Global.Input.k2p = true;
                Global.Input.k2d = true;
            }
        }
        else if(Event.type == SDL_MOUSEBUTTONUP){
            if(Event.button.button == SDL_BUTTON_LEFT){
                Global.Input.k1r = true;
                Global.Input.k1d = false;
            }
            if(Event.button.button == SDL_BUTTON_RIGHT){
                Global.Input.k2r = true;
                Global.Input.k2d = false;
            }
        }
        else if(Event.type == SDL_KEYDOWN){
            if(Event.key.keysym.sym == SDLK_z){
                Global.Input.k1p = true;
                Global.Input.k1d = true;
            }
            if(Event.key.keysym.sym == SDLK_x){
                Global.Input.k2p = true;
                Global.Input.k2d = true;
            }
        }
        else if(Event.type == SDL_KEYUP){
            if(Event.key.keysym.sym == SDLK_z){
                Global.Input.k1r = true;
                Global.Input.k1d = false;
            }
            if(Event.key.keysym.sym == SDLK_x){
                Global.Input.k2r = true;
                Global.Input.k2d = false;
            }
        }
        else if(Event.type == SDL_QUIT)
            Global.quit = true;        
    }
}

void SDLGetMouse(){
    //Get the mouse position and also check if it is in the game area
    Global.MouseInFocus = CheckCollisionPointRec({Global.Input.mx, Global.Input.my}, (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()});
    //std::cout << Global.MouseInFocus << " " << Global.Input.mx << " " << Global.Input.my << std::endl;
    if(!Global.MouseInFocus){
        Global.MousePosition = {0,0};
    }
    else {
        Global.MousePosition = {(Global.Input.mx - Global.ZeroPoint.x) / Global.Scale, (Global.Input.my - Global.ZeroPoint.y) / Global.Scale};
    }
}

# osus-revented
The suspicious osu clone named osus, redone!


## Why do something like this?
This game, originally just a fun way to learn raylib, turned into me (ohasanov) rewriting the raylib library for our purposes. The library, based on raylib 3.5, originally used GLFW as the window and input manager, which I scrapped since it weirdly doesn't support multithreading. The miniaudio library that shipped with raylib also had a ton of lag in the audio department. Both of those were basically rewritten. Although it is a crude way to do it, it is a good learning experience.

The current way we handle audio and video is by using SDL2 as the backend and a modified miniaudio library. SDL2 also doesn't "support" multithreading but at least it allowed us to poll input on a different thread than the video thread. And yes, we know that this is a pretty hacky way of doing it but probably as every developer says, this problem is for our future selves to fix. 

The code is also structured in a way which allows a parallel Nintendo 3DS Port! This is achieved by just copying the needed graphics / sound / input functions and making system spesific functions that are seperated with `#ifdef`s.

Everything summerized the current implementation of our library is not the best (but it works!).

I (ohasanov) also am planning to add more game modes to this which originally may not have existed in osu!. This is just going to be a playground, in which we mess around with audio, 2D graphics and shaders.

## How does it perform?
Currently the game is running at a pretty constant speed, but it really is dependant on the OS, since we use OS Sleep functions to actually throttle our code (frame limit). On a mobile core2duo from about 2007-8 it manages to easily reach the 1500-1600Hz game logic speed. The game also achieves a good framerate on the 3DS, mostly being locked to 60fps, which is the maximum for that console, while polling input at the highest rate possible, at 250Hz. 

Funnily enough, the 3DS also has the best latency possible, close to actually none. This is probably because of the simpler OS, but the 40ish ms offset I add on desktop builds is not needed for the silly Nintendo Console :D

Here is a ScreenShot of both versions:

![alt text](https://github.com/ohasanov-hbrw/osus-revented/blob/3ds-pc-merge/resources/gameplay.png?raw=true)

## Features?
Basically speaking, most of the game is implemented and most (normal) maps should be completely playable. Of course, there are some maps that don't work, especially the ones that stretch the limits of even the original osu! engine. I also haven't really figured out how spinners work, which I plan to fix in the near future. Scoring is also something that always changes with the osu! client, but since this project does not exist for any kind of competitive purpose, the scoring system I implemented is enough for now. 

The rendering engine cannot really handle storyboards, since they tank the fps on 3DS, I _really_ dont plan on adding storyboard support. The fancy sliders are enough of a performance hit already.


## How to build the project?

### Desktop Builds:
To build the game for desktop you only need the basic tools installed:

- GCC and G++ (and also MinGW variants for the windows builds, there is no native windows build support planned)
- Make
- CMake
- SDL2

To actually build the game follow the steps below:

1. create directories called `build`, `object` and `bin`
2. run `./linuxmakeall.sh` or `./windowsmakeall.sh`


When you just want to run the game, use `./linuxmakefast` or `./windowsmakefast.sh`, or just move the executable to the parent directory and just run it like every other binary executable!

>(note: these scrips compile with 12-16 threads, the numbers may need to be changed on your pc, be careful with overloads)

### 3DS Builds:
To build the game for the Nintendo 3DS, you will need to have devkitPro installed on your system.

- GCC and G++ - from the devkitPro repository
- Make
- CMake
- SDL - from the devkitPro repository

To actually build the game follow the steps below:

1. create directories called `build`, `object` and `bin`
2. run `./3dsmakeall.sh`

When you have only changed the files in the `src` folder, use `./3dsmakefast.sh` to not waste time on recompiling the other libraries every time... To run the program, use the `.3dsx` file or create a `.cia` file using `makerom`. Just google for more information!

The 3DS build expects the `sdmc:/3ds/beatmaps` and `sdmc:/3ds/resources` folders to exist. Just create a beatmaps folder on your SD Card and extract the beatmaps into it. Also copy the resources folder with the skins.

>(note: these scrips compile with 12-16 threads, the numbers may need to be changed on your pc... be warned)


### Common Issues:
The game will crash if there isn't a folder named `beatmaps` present in the folder you are running the executable in. Just create one and extract your beatmaps into it using a unzip program. (lets say you the osus executable is in `~/osus-revented/bin/osus` and you run the game from `~/osus-revented/` by executing `bin/osus`. In this case, the folder `~/osus-revented/beatmaps/` should exist.)

**Just like the `beatmaps` folder, have the `resources` folder (and its data) ready in the same way.**

Expect to see many bugs and slowdowns, as this is a "Work In Progress" project, but I hope that I can make it better and for that I need your support!

When you encounter a bug, or see something missing, just report it using the "Issues" tab on github! I will check the issue as fast as I can and I will try to fix it :D


## Some things to keep in mind:
- make sure that the skins are uniform. I will implement some "dividers" to the global variables, if you absolutely need to use a skin that is non-standard. OSU!'s skin management is honestly not good. not good at all. even though some objects need to be 128x128 pixels for example, there are some skins that just have 160x160 pixel objects for no apparent reason. I will not try to support this BS. just rescale them yourselves. 
- some maps are also offset for some reason, I cant figure out why. I will try to implement a per-map-offset feature but yeah... cringe moment.
- this clone also uses OS sleep. This may lead to innacuracies or stutters. I really dont want to spin-lock, since that kills the performance.
- the WIP buttons obviously are... _work in progress_... If they dont work, don't be suprised...

## TODO List:
- [x] Create a TODO list.
- [x] Create a codebase for the 3DS Builds
- [ ] Debug some memory problems 
- [ ] Menu rewrite
- [ ] Error management
- [ ] Check-stresstest the zip library
- [ ] Skin customization with an options menu
- [ ] Parse beatmaps and cache them
- [ ] Enable offscreen MSAA for better looking sliders
- [ ] Add an options menu with the option to select opengl version
- [ ] Add modifiers like hidden to the game
- [ ] Add the game mode taiko!
- [ ] Fix the spinners
- [ ] Try to get beatmaps from a mirror (https://catboy.best/about)
- [ ] Use the second screen on the 3DS
- [ ] Dynamic slider resolution with more interpolation to cut down on calculation time





![alt text](https://github.com/ohasanov-hbrw/osus-revented/blob/3ds-pc-merge/resources/osus.png?raw=true)

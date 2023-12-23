# osus-revented
The suspicious osu clone named osus, redone!

This game, originally just a fun way to learn raylib, turned into me (ohasanov) rewriting the raylib library for our purposes. The library, based on raylib 3.5, originally used GLFW as the window and input manager, which I scrapped since it weirdly doesn't support multithreading. The miniaudio library that shipped with raylib also had a ton of lag in the audio department. Both of those were basically rewritten. Although it is a crude way to do it, it is a good learning experience.

The current way we handle audio and video is by using SDL2 as the backend and a modified miniaudio library. SDL2 also doesn't "support" multithreading but at least it allowed us to poll input on a different thread than the video thread. And yes, we know that this is a pretty hacky way of doing it but probably as every developer says, this problem is for our future selves to fix. 

Everything summerized the current implementation of our library is not the best (but it works!).

I (ohasanov) also am planning to add more game modes to this which originally may not have existed in osu!. This is just going to be a playground, in which we mess around with audio, 2D graphics and shaders.

Currently the game is running at a pretty constant speed, but it really is dependant on the OS, since we use OS Sleep functions to actually throttle our code (frame limit). On a mobile core2duo from about 2007-8 it manages to easily reach the 900-1000Hz game logic speed, but we will make the game even faster, as we learn how to properly do multithreading.


To build the game you only need the basic tools installed:

-> GCC and G++ (and also MinGW variants for the windows builds, there is no native windows build support right now)

-> Make

-> CMake

-> SDL2



To actually build the game follow the steps below:

1 - create directories called "build" and "object"

2 - run ```./linuxmakeall.sh``` or ```./windowsmakeall.sh```


When you just want to run the game, use ```./linuxmakefast``` or ```./windowsmakefast.sh```, or just move the executable to the parent directory and just run it like every other binary executable!

(note: these scrips compile with 12 threads, the numbers may need to be changed on your pc... be warned)


Expect to see many bugs and slowdowns, as this is a "Work In Progress" project, but we hope that we can make it better and for that we need your support!

When you encounter a bug, or see something missing, just report it using the "Issues" tab on github! We will check the issue as fast as we can and we will try to fix it :D


Some interesting things: 
-> make sure that the skins are uniform. i will implement some "dividers" to the global variables, if you absolutely need to use a skin that is non-standard. OSU!'s skin management is honestly not good. not good at all. even though some objects need to be 128x128 pixels for example, there are some skins that just have 160x160 pixel objects for no apparent reason. i will not try to support this BS. just rescale them yourselves. 
-> some maps are also offset for some reason, i cant figure out why. i will try to implement a per-map-offset feature but yeah... cringe moment.
-> this will be ported to the 3ds. the code already has opengl1.1 support... this is why the spinny menu is still wip, since the 3ds doesnt have enough vram to render that :)
-> this clone also uses OS sleep. This may lead to innacuracies or stutters. i really dont want to spin-lock, since that kills the performance.


![alt text](https://github.com/ohasanov-hbrw/osus-revented/blob/master/resources/osus.png?raw=true)

# osus-revented
The sussy osu clone named osus, revamped!

This game, originally just a fun way to learn raylib, turned into us rewriting the raylib library for out purposes. As such, the current implementation of out library isnt the best. I (ohasanov) also am planning to add more game modes to this which originally may not have existed in osu!. This is just going to be a playground, in which we mess around with audio, 2D graphics and shaders.

Currently the game is running at a pretty constant speed. On a mobile core2duo from about 2007-8 it manages to easily reach the 750Hz game logic speed, but we will make the game even faster, as we learn how to properly do multithreading.


To build the game you only need the basic tools installed:

-> GCC and G++ (and also MinGW variants for the windows builds, there is no native windows build support right now)

-> Make

-> CMake

-> SDL2



To actually build the game follow the steps below:

1 - create directories called "build" and "object"

2 - run ```./linuxmakeall.sh``` or ```./windowsmakeall.sh```


When you just want to run the game, use ```./linuxmakefast``` or ```./windowsmakefast.sh```, or just move the executable to the parent directory and just run it like every other binary executable!

(note: these scrips compile with 12 threads, the numbers may need to change on your pc... be warned)


Expect to see many bugs and slowdowns, as this is a "Work In Progress" project, but we hope that we can make it better and for that we need your support!

When you encounter a bug, or see something missing, just report it using the "Issues" tab on github! We will check the issue as fast as we can and we will try to fix it :D




![alt text](https://github.com/ohasanov-hbrw/osus-revented/blob/master/resources/osus.png?raw=true)

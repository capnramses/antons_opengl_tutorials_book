This series of demos accompanies the e-book "Anton's OpenGL 4 Tutorials":
http://www.amazon.com/gp/product/B00LAMQYF2

Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland. June 2014.
email: anton at antongerdelan dot net

A number of libraries are used for different demonstrations. See LICENCE.txt
and the respective documentation of each for copyright and licence information.

# Info #

See "LICENCE.txt" for licence information.

For updated information about this demonstration code, see:
http://antongerdelan.net/opengl/book_info.html

There is also an example of code for "Hello Triangle" for OpenGL 2.1 for
reference

All of these examples were built with a 64-bit Xubuntu Linux machine, so there
are Makefiles for 64-bit linux. I'm also adding project files for 32-bit linux,
32-bit windows (GCC and Visual Studio), and 64-bit Apple OS X but this will take
a few days and I might introduce some bugs. Send me an e-mail for any queries or
requests here.

Anton Gerdelan, 2 July 2014.

# Compiling #
The libraries reside in the common/ folder

* common/include - header files
* common/linux_i386 - 32-bit linux libraries
* common/linux_x86_64 - 64-bit linux libraries
* common/msvc110 - 32-bit Windows visual studio libraries
* common/osx_64 - 64-bit apple OS X libraries
* common/win32 - 32-bit Windows GCC (mingw) libraries

## Linux ##

* install the GNU Compiler Collection - usually by installing a
"build-essentials" package via the package manager on your distribution.
* open a terminal and cd to the demo of choice

64-bit systems:

`make -f Makefile.linux64`

32-bit systems:

`make -f Makefile.linux32`

## 64-bit Apple OS X ##

I don't have an Apple machine so these may not all have a Makefile available
right away. I'll do my best to update this and test all the projects - check
back for updates. The Apple drivers are not great - expect bugs in some demos.

* install the GNU Compiler Collection - usually by installing XCode
* open a terminal and cd to the demo of choice

`make -f Makefile.osx`

## Windows with GCC ##

I only provided 32-bit versions of the Makefile and libraries here. If you want
to add a 64-bit build it's pretty easy to copy the 32-bit Makefile and change
the folder. You will need to recompile GLFW, GLEW, AssImp, and Freetype though.
The 32-bit builds will run on all Windows machines - I use 32-bit builds on my
64-bit Windows.

* install the GNU Compiler Collection - usually by installing the MinGW toolkit.
http://www.mingw.org/
* open a console and cd to the demo of choice

`make -f Makefile.win32`

* copy the .dll files from the main folder to the demo folder

## Windows with Visual Studio ##

I provided some Visual Studio 2012 project files.
You can find an overarching solution file in the main folder. This should
convert well to most versions of visual studio. I used 32-bit versions of the
libraries, but there's no reason that you can't add 64-bit versions if you
prefer.

# SDL2 Port #

Dr Aidan Delaney at the University of Brighton has made an SDL2 port (as an alternative to using GLFW), which you can find on GitHub https://github.com/AidanDelaney/antons_opengl_tutorials_book/tree/sdl2-port/00_hello_triangle
(see Makefile.linux64)

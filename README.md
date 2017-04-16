# Anton's OpenGL 4 Tutorials book demo code #

This series of demos accompanies the e-book "Anton's OpenGL 4 Tutorials":
[antongerdelan.net/opengl](http://antongerdelan.net/opengl/)

Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.
email: antonofnote AT gmail

## Info ##

See "LICENCE.txt" for licence information.

Each chapter with major demonstration code has a corresponding demo here.
There is also an example of code for *Hello Triangle* for OpenGL 2.1 for reference.

Each demo has easy-to-read Makefiles for Linux, OS X, and 32-bit Windows.
There are also Visual Studio projects, but you may have to download newer
versions of the libraries in the `common/` folder to match your version of VS.

**This code is some years old now and builds may fall out of date**. I try to
maintain this so that it functions but be aware that Makefiles and build details
may differ slightly from book text for this reason.
If you have a [tidy] CMake setup or updated build **feel free to submit a pull request here**.

## Compiling ##

The demos here depend on the GLEW

The libraries reside in the common/ folder

* `common/include` - header files
* `common/linux_i386` - 32-bit linux libraries
* `common/linux_x86_64` - 64-bit linux libraries
* `common/msvc110` - 32-bit Windows visual studio libraries
* `common/osx_64` - 64-bit apple OS X libraries
* `common/win32` - 32-bit Windows GCC (mingw) libraries

### Linux ###

* Install a C and C++ compiler - usually by installing a "build-essential"
bundle package via the package manager on your distribution:

```
sudo apt-get install build-essential
```

* Install the GLFW3 and FreeType libraries:

```
sudo apt-get install libglfw3-dev
sudo apt-get install libfreetype6-dev
```

* Open a terminal and cd to the demo of choice, then

64-bit systems:

```
make -f Makefile.linux64
```

32-bit systems:

```
make -f Makefile.linux32
```

### Apple OS X ###

* Install Clang or GNU compiler and tools - usually by installing Apple XCode through the App Store. It's free.
* Open a terminal and cd to the demo of choice:

```
make -f Makefile.osx
```

### Windows with GCC ###

I only provided 32-bit versions of the Makefile and libraries here.
The 32-bit builds will run on all Windows machines - I use 32-bit builds on my
64-bit Windows.
If you want to add a 64-bit build it's pretty easy to copy the 32-bit Makefile and change
the folder. You will need to recompile GLFW, GLEW, AssImp, and Freetype though.

* install the GNU Compiler Collection - usually by installing the MinGW toolkit.
http://www.mingw.org/
* open a console and cd to the demo of choice

`make -f Makefile.win32` (may have been renamed to mingw-make32 or similar)

* copy the .dll files from the main folder to the demo folder

### Windows with Visual Studio ###

I provided some Visual Studio 2012 project files.
You can find an overarching solution file in the main folder. This should
convert well to most versions of Visual Studio. I used 32-bit versions of the
libraries, but there's no reason that you can't add 64-bit versions if you
prefer.

VS 2013 seems to work, but with 2015 you'll probably need to download a newer
version of the libraries used to get around linking errors. 

## Caveats ##

* There are some lazy `assert(call_some_function())` calls that will be wiped out if compiling in a release mode that removes asserts. Replace these with your own `if (!call_some_function()) { print error and do appropriate thing }`
* Code is directly copy-pasted from book sections. This means that there will be redundant OpenGL calls to bind things etc. but I think it's easier to follow along like this.
* Code explained in prior examples is moved to a file called `gl_utils.cpp` to avoid cluttering `main.cpp`. This means that `gl_utils.cpp` is not necessarily the same in each demo, but is built up gradually.

## SDL2 Port ##

Dr Aidan Delaney at the University of Brighton has made an SDL2 port (as an
alternative to using GLFW), which you can find on GitHub
https://github.com/AidanDelaney/antons_opengl_tutorials_book/

## Credits ##

Special thanks to all the readers over the years that have submitted additions,
bug reports, fixes, and feedback. If you have submitted a correction and don't
mind having your name/@ printed here please let me know.

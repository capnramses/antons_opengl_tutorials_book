# Anton's OpenGL 4 Tutorials book demo code

This series of demos accompanies the e-book "Anton's OpenGL 4 Tutorials":
[antongerdelan.net/opengl](http://antongerdelan.net/opengl/)

## Info

See `LICENCE.txt` for licence information.

Each chapter with major demonstration code has a corresponding demo here.
There is also an example of code for *Hello Triangle* for OpenGL 2.1 for reference.

Each demo has easy-to-read Makefiles for Linux, MacOS, and Windows.
You may need to download newer versions of the libraries in the `third_party/` folder.

This code is some years old now and builds may fall out of date.
I try to maintain this so that it functions, but be aware that Makefiles and
build details may differ slightly from book text for this reason.

## Compiling Demos

### Linux

* Install a C and C++ compiler - usually by installing a `build-essential`
bundle package via the package manager on your distribution.
E.g. for Ubuntu:

`sudo apt install build-essential`

* Install the GLEW, GLFW3, FreeType, and Assimp development libraries:
  `sudo apt install libglew-dev libglfw3-dev libfreetype-dev libassimp-dev`

* Open a terminal and `cd` to the demo of choice, then:
  `make -f Makefile.linux64`

* To build all the demos you can run `./build_all_linux_osx.sh` from the main directory.

### Apple macOS

* Install Clang or GNU compiler and tools - usually by installing Apple [XCode](https://apps.apple.com/us/app/xcode/id497799835?mt=12) through the App Store.

* You will probably wish to install libraries via [Homebrew](https://brew.sh/), similarly to Linux, above.
  `brew install glew glfw assimp freetype`

* Open a terminal and `cd` to the demo of choice.
  * For Intel Macs, run `make -f Makefile.osx`
  * For Apple Silicon Macs, run `make -f Makefile.osxARM`

* To build all the demos you can run `./build_all_linux_osx.sh` from the main directory.

### Windows with Visual Studio

Create a new _Empty_, _C++_, _Console_ project.
You can then easily install the required libraries with _NuGet_, under the _Project_ menu of a new C++ project, and you're ready to draw a triangle in 5 minutes.

#### Install Libraries with NuGet

1. In the _Browse_ tab search for and install; `glfw`, and `glew`. For later tutorials you can also find `assimp`, and `freetype`, when required.
2. You need to add the text `opengl32.lib` to your linker input string. You can find this in _Project->Properties->Configuration Properties->Linker->Input->Additional Dependencies_. Just add `opengl32.lib;` to the front of the long string of dependencies so that it changes to `opengl32.lib;kernel32.lib;user32;...`.
3. You do not need to add the libraries you installed _via_ NuGet to the linker string.
4. You can now compile a "Hello Triangle" demo for OpenGL using GLFW and GLEW.

#### Install Libraries Manually

If you would rather do things the old fashioned way, without using a package manager, I have recorded a 2020 video stream tutorial where I show how to get Visual Studio set up and start programming OpenGL, including downloading and setting up libraries.

[Tutorial: Intro to 3D Graphics Programming with OpenGL 4 (with Anton). Stream Recording.](https://youtu.be/qQJ7irgxZFQ)

This includes a very verbose set-up of Visual Studio 2019 with helper libraries.

### Windows with GCC

* Install the GNU Compiler Collection - usually by installing MinGW. I suggest the minimal MinGW GCC distro at [https://nuwen.net/mingw.html](https://nuwen.net/mingw.html).
* Open a console and `cd` to the demo of choice.
* `make -f Makefile.win64`

If you have trouble linking supporting libraries you may need to download and recompile GLFW, GLEW, AssImp, and FreeType. It's a good idea to do this anyway to stay up to date.

* https://www.glfw.org/
* https://glew.sourceforge.net/
* https://freetype.org/download.html
* https://github.com/assimp/assimp

## Caveats and Errata

* Since publication the most reliable version of newer OpenGL that will work everywhere, including macOS, is 4.1 Core. I suggest setting _window hints_ to try this version first.
* Code is directly copy-pasted from book sections. This means that there will be redundant OpenGL calls to bind things etc., but I think it's easier to follow along like this.
* Code explained in prior examples is moved to a file called `gl_utils.cpp` to avoid cluttering `main.cpp`. This means that `gl_utils.cpp` is not necessarily the same in each demo, but is built up gradually.
* Out-of-date build files have been removed; 32-bit builds, and older Visual Studio files.
* Sometimes people ask for C examples. OpenGL is a C API, and I would have used C if writing the text later. Readers wishing to use a C compiler should do so - only very minor code convention changes are required.

## Contributors

Special thanks to all the readers over the years that have submitted additions,
bug reports, fixes, and feedback. If you have submitted a correction and don't
mind having your name/@ printed here please let me know (or if you'd like to change these details).

* Olivier Nivoix
* Sarang Baheti <https://github.com/sarangbaheti>
* kevin
* Jon
* Julien Castelain <https://github.com/julien>
* Benjamin Summerton <https://github.com/define-private-public>
* Fwjrei
* guysherman
* 24kwakahana
* battila7
* Gnimuc <https://github.com/Gnimuc>
* Peter Getek <https://github.com/postfixNotation>
* Mikel Losada <https://github.com/Workshoft>
* Kevin Moran <https://github.com/kevinmoran>
* Jon <https://github.com/0xBAMA>
* Pablo Alonso-Villaverde Roza <https://github.com/pavroza>

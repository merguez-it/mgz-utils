# mgz-utils

Merguez-IT utils 

*This is a Work In Progress.* So test it but don't use it.

## Dependencies

### Compiling

* [cmake](http://www.cmake.org/)

### Documentation

* [doxygen](http://doxygen.org/) (optional)
* [graphviz](http://graphviz.org/) (optional)

### Testing 

* [gtest](http://code.google.com/p/googletest/) 

## Compiling

Install [cmake](http://www.cmake.org/cmake/help/install.html), [doxygen](http://www.stack.nl/~dimitri/doxygen/install.html) and [graphviz](http://www.graphviz.org/Download.php), then :

### On MacOSX, or Linux

    cd <path to mgz-utils root>
    mkdir build
    cd build
    cmake [-DCMAKE_INSTALL_PREFIX:PATH=/<path>] .. 
    make
    make test
    make doc
    make install

### On Windows

You need to install [MinGW](http://www.mingw.org/wiki/InstallationHOWTOforMinGW). Add the bin directory of your MinGW installation in your PATH. Then, open a Windows (not MSYS) console and :

    cd <path to mgz-utils root>
    mkdir build
    cd build
    cmake ..
    mingw32-make
    mingw32-make test
    mingw32-make doc
    mingw32-make install

## Licences

(C) 2013 Merguez-IT

This code use part of TinyXML by Lee Thomason<br />
http://www.grinninglizard.com/tinyxml/index.html<br />
(C) Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.

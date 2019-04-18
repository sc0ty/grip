# grip installation guide

## Requirements
- C++11 compiler (e.g. GCC 4.8+ or Clang 3.4+);
- Boost library: regex, filesystem and system.

## Compilation
Assuming above requirements are met
```
mkdir build
cd build
cmake ..
make
make install
```

Useful `cmake` switches:
* `-DCMAKE_BUILD_TYPE=[Debug,Release,RelWithDebInfo,MinSizeRel]` - build type;
* `-DCMAKE_INSTALL_PREFIX=dir` - where to install;
* `-DBoost_USE_STATIC_LIBS=ON -DBoost_USE_STATIC_RUNTIME=ON` - to link with boost statically (default for MinGW).

### Compilation under Windows
Install MinGW and add `bin` directory to your `PATH`.
Download Boost sources, extract it and compile with MinGW
```
bootstrap gcc
b2 toolset=gcc threading=single
```
Go to the grip directory and build it (fix directories accordingly)
```
mkdir build
cd build
set BOOST_ROOT=c:\projects\boost_1_70_0
set BOOST_LIBRARYDIR=c:\projects\boost_1_70_0\stage\lib
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
make
```

It should be possible to build it with Visual Studio or anythong that cmake supports.

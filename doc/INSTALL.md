# grip installation guide

## Requirements
- C++11 compiler (e.g. GCC 4.8+ or Clang 3.4+);
- POSIX-compatibile environment (tested on Ubuntu 16.04 and Debian 8) or Boost library.

## Compilation
Assuming above requirements are met
```
cd src
make
```
Compiler could be selected using `CXX` variable, e.g. `CXX=clang++ make`.
Compiler and linker flags could be changed with `CXXFLAGS` and `LDFLAGS` respectively.

Debug version (with debug symbols) could be build as follows
```
make clean
make debug
```
Make sure to clean project first, it would not rebuild otherwise.

### Compilation under Windows
Install MinGW and add `bin` directory to your `PATH`.
Download Boost sources, extract it and compile with MinGW
```
bootstrap gcc
b2 toolset=gcc threading=single
```
Go to the grip directory and build it (fix directories and library name accordingly)
```
cd src
set BOOST_CXXFLAGS=-Ic:\Boost\include\boost-1_64
set BOOST_LDFLAGS=-Lc:\Boost\lib -lboost_regex-mgw62-1_64 -lboost_system-mgw62-1_64 -lboost_filesystem-mgw62-1_64
mingw32-make USE_BOOST=manual STATIC=yes
```
You may need to add `-pthread` to `BOOST_LDFLAGS` if you are linking against multi-threaded boost lbrary version.

### Configuration
Makefile accepts several configuration variables:
* `STATIC=yes` - static linking with libraries;
* `USE_BOOST=yes/manual` - use boost library instead of POSIX functions, `manual` allows to configure compiler and linker flags manualli with `BOOST_CXXFLAGS` and `BOOST_LDFLAGS`;
* `CXX` - override compiler program;
* `CXXFLAGS` and `LDFLAGS` - override compiler and linker flags.

## Installation
To install to default location (`/usr/local`), run as root
```
make install
```
Or to install to specific directory
```
PREFIX=/my/dir make install
```
You could also just use created binaries directly.

## Deinstallation
Run as root
```
make uninstall
```
If `grip` was installed inside custom PREFIX directory, uninstall must be called with the same PREFIX.

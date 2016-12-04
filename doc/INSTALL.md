# grip installation guide

## Requirements
- C++11 compiler (e.g. GCC 4.8+ or Clang 3.4+);
- POSIX-compatibile environment (tested on Ubuntu 16.04 and Debian 8).

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

# grip
Indexed grep - fast search (grep like) in huge stack of files.

Author: [Mike Szymaniak](http://sc0ty.pl)

## Features
- fast search for pattern in text files;
- intended mainly (but not limited to) source code;
- support for Huge Blobs of Legacy Code&trade;;
- language & encoding agnostic (excluding UTF-16);
- case sensitive and case insensitive search;
- search limited to current subdirectory;
- context line control (`grep`s `-B`, `-A` and `-C` switches);
- colored output.

## Limitations
- index must be generated prior to search;
- and it must be up to date with files content;
- no regexp support (not yet);
- only limited `grep`s switches are implemented, no `--include`, `--exclude`, `-v`, `-x` and many others;
- search pattern is assumed to be encoded the same way as searched files (usually not the case for ASCII characters, e.g. source code).

## Requirements
- C++11 compiler (e.g. GCC 4.8+ or Clang 3.4+);
- POSIX-compatibile environment (tested on Ubuntu 16.04 and Debian 8).

## Installation
Assuming above requirements are met
```
cd src
make
make install
```
For more information see [installation guide](doc/INSTALL.md)

## Usage
First we need to generate index database
```
find -type f | gripgen
```
It will create database directory `.grip`.
In case of great number of files, this step will take some time. Indexer typically process 100 to 1000 files per second, and the resulting database size will be about 10% of the indexed data.

You could also provide file list to index
```
gripgen list.txt
```
Indexed files must be located inside current directory and its subdirectories.

Now you could perform search, e.g.:
```
grip printf
grip -i -C3 'hello world'
grip --list main
```
For more usage information type `grip --help`.

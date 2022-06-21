# Remote Dictionary

## Requirements:
1. C++17
2. CMake
3. Bitsery - serialize library

## Build instructions:
4. git clone https://github.com/ruslanti/remote_dictionary.git
5. git submodule update --init --recursive
6. mkdir build
7. cd build
8. cmake  -DCMAKE_BUILD_TYPE=Debug ../
9. make

## Executables

**dictionary** - remote server which bind by default port 2222

**client** - example of client usaga and simple benchamark tool

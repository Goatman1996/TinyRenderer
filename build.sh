#!/bin/bash

if [ ! -d "build" ]; then
  mkdir build
fi

cd ./build
cmake ..
make 

echo ========================
echo "Build Done!"
echo ========================

cd ..
cd ./bin
./TinyRenderer ./output.tga ../obj/african_head.obj
# ./TinyRenderer ./output.tga ../obj/MokeyHead.obj
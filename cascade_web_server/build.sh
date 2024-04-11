#!/bin/bash

# Check if the first argument is "clean"
if [ "$1" == "clean" ]; then
    echo "Cleaning the build directory..."
    rm -rf build
fi

mkdir -p build  # '-p' ensures that 'mkdir' does not throw an error if the directory already exists
cd build
cmake -DCMAKE_PREFIX_PATH=/root/opt-dev ..
make
cp cascade_web_server ../

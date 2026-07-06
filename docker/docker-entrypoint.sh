#!/bin/sh
[ -z $CONFIG ] && config=Release || config="$CONFIG"
[ -z $ARCH ] && arch=32 || arch="$ARCH"

if [ "$arch" = "64" ]; then
    cmake \
        -S . \
        -B build \
        -DCMAKE_BUILD_TYPE=$config \
    &&
    cmake \
        --build build \
        --config $config \
        --parallel $(nproc)
else
    cmake \
        -S . \
        -B build \
        -DCMAKE_BUILD_TYPE=$config \
        -DCMAKE_C_FLAGS=-m32 \
        -DCMAKE_CXX_FLAGS=-m32 \
    &&
    cmake \
        --build build \
        --config $config \
        --parallel $(nproc)
fi
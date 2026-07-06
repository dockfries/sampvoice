#!/bin/bash

# Available configs: Debug, RelWithDebInfo, [Release]
[[ -z "$CONFIG" ]] \
&& config=Release \
|| config="$CONFIG"

# Target architecture: [32] or 64
[[ -z "$ARCH" ]] \
&& arch=32 \
|| arch="$ARCH"

docker build \
    -t omp-voice/build:ubuntu-24.04 ./ \
|| exit 1

build_dir="build${arch}"

folders=("${build_dir}")
for folder in "${folders[@]}"; do
    if [[ ! -d "./${folder}" ]]; then
        mkdir ${folder}
    fi
    sudo chown -R 1000:1000 ${folder} || exit 1
done

docker run \
    --rm \
    -t \
    -w /code \
    -v $PWD/..:/code \
    -v $PWD/${build_dir}:/code/build \
    -e CONFIG=${config} \
    -e ARCH=${arch} \
    omp-voice/build:ubuntu-24.04
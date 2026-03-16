#!/usr/bin/env bash
set -e

BUILD_DIR=build
INSTALL_DIR=AppDir/usr

rm -rf $BUILD_DIR
cmake -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Release
cmake --build $BUILD_DIR -j$(nproc)
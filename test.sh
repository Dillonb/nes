#!/usr/bin/env sh
set -e
cd build
cmake ..
make
make test

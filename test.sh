#!/usr/bin/env bash
set -e
cd build
cmake ..
make
make test

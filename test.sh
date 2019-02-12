#!/usr/bin/env bash
set -e
cd build
cmake ..
make
make CTEST_OUTPUT_ON_FAILURE=1 test

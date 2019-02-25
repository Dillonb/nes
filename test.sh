#!/usr/bin/env bash
set -e
cd build
cmake ..
if [ `uname` == "Darwin" ]; then
    export LIBRARY_PATH="/usr/local/lib"
fi
make
make CTEST_OUTPUT_ON_FAILURE=1 test

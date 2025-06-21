#!/bin/bash

set -xue

sudo apt-get update
sudo apt-get install libssl-dev

sudo apt-get install g++-mingw-w64-x86-64 gcc-mingw-w64-x86-64 gcc-mingw-w64-i686-win32 g++-mingw-w64-i686-win32


sudo apt-get update
#Linux
sudo apt-get --yes install \
    g++ make cmake \
    libboost-all-dev
#Windows
MXE_TARGET=i686-w64-mingw32.static
MXE2_TARGET=$(echo "$MXE_TARGET" | sed 's/_/-/g')
sudo apt-get --yes install \
    mxe-${MXE2_TARGET}-boost


MXE_TARGET=x86-64-w64-mingw32.static

MXE2_TARGET=$(echo "$MXE_TARGET" | sed 's/_/-/g')
sudo apt-get --yes install \
    mxe-${MXE2_TARGET}-boost

#Coveralls
pip install --user cpp-coveralls

set +xue

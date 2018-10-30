#!/bin/bash

sudo apt install -y pkg-config libboost-serialization-dev libboost-filesystem-dev libboost-system-dev libboost-program-options-dev libboost-test-dev libeigen3-dev libode-dev
sudo apt install software-properties-common
sudo apt install -y cmake git libboost-system-dev libboost-thread-dev freeglut3 freeglut3-dev libglpk-dev python-dev python-opengl libxmu-dev libxi-dev libqt4-dev libassimp-dev ffmpeg g++-5 libboost1.55-all-dev libeigen3-dev libgtest-dev
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt -y upgrade
sudo apt install -y g++-5 cmake pkg-config libboost1.55-serialization-dev libboost1.55-filesystem-dev libboost1.55-system-dev libboost1.55-program-options-dev libboost1.55-test-dev libeigen3-dev libode-dev
export CXX=g++-5

cd /usr/src/gtest
sudo mkdir build
cd build
sudo cmake ..
sudo make
sudo cp -rf libgtest* /usr/lib/
sudo mkdir /usr/local/lib/gtest
sudo ln -s /usr/lib/libgtest.a /usr/local/lib/gtest/libgtest.a
sudo ln -s /usr/lib/libgtest_main.a /usr/local/lib/gtest/libgtest_main.a

### Dependencies

- Eigen 3.3
- Boost 1.55
- OMPL 1.4.0
- Klampt 0.6
  (modified version github.com/aorthey/Klampt and github.com/aorthey/KrisLibrary)

Above libraries will be installed by following Installation procedure.

### Installation
#### 1. Requirement software install
```bash
$ sudo apt-get install cmake git libboost-system-dev libboost-thread-dev freeglut3 freeglut3-dev libglpk-dev python-dev python-opengl libxmu-dev libxi-dev libqt4-dev libassimp-dev ffmpeg g++-5 libboost1.55-all-dev libeigen3-dev libgtest-dev
```

#### 2. gtest install
```bash
$ cd /usr/src/gtest
$ sudo mkdir build
$ cd build
$ sudo cmake ..
$ sudo make
$ sudo cp -rf libgtest* /usr/lib/
$ sudo mkdir /usr/local/lib/gtest
$ sudo ln -s /usr/lib/libgtest.a /usr/local/lib/gtest/libgtest.a
$ sudo ln -s /usr/lib/libgtest_main.a /usr/local/lib/gtest/libgtest_main.a
```

#### 3. Build Libraries
```bash
$ cd <orthoklampt>/Library
$ make unpack-deps
$ make deps
```

#### 4. Build orthoklampt itself
```bash
$ cd <orthoklampt>
$ mkdir build
$ cd build
$ cmake ..
$ make -j`nproc`
$ sudo make install
```

### Execution
```bash
$ cd <orthokoampt>
$ ./build/planner_hierarchy ./data/experiments/06D_doubleLshape.xml
```

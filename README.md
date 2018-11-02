### Dependencies

- Eigen 3.3
- Boost 1.55
- OMPL 1.4.0
- Klampt 0.6
  (modified version github.com/aorthey/Klampt and github.com/aorthey/KrisLibrary)

Above libraries will be installed by following Installation procedure.

### Installation
#### 1. Run the pre-requirement install script
```bash
$ cd <orthoklampt>
$ source pre-requirement.bash
```

#### 2. Build Libraries
```bash
$ cd <orthoklampt>/Library
$ make -j`nproc` unpack-deps
$ make -j`nproc` deps
```

#### 3. Build orthoklampt itself
```bash
$ cd <orthoklampt>
$ mkdir build
$ cd build
$ cmake ..
$ make -j`nproc`
```

### Execution
```bash
$ cd <orthokoampt>/build
$ ./planner_hierarchy ../data/experiments/06D_doubleLshape.xml
```

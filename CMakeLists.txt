CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT( Orthoklampt )
SET(ORTHOKLAMPT_SUBDIRS algorithms controller elements gui planner)

set(ORTHOKLAMPT_MAJOR_VERSION 0)
set(ORTHOKLAMPT_MINOR_VERSION 2)
set(ORTHOKLAMPT_PATCH_VERSION 0)
set(ORTHOKLAMPT_VERSION ${ORTHOKLAMPT_MAJOR_VERSION}.${ORTHOKLAMPT_MINOR_VERSION}.${ORTHOKLAMPT_PATCH_VERSION})
#set (CMAKE_CXX_STANDARD 11)

set(KLAMPT_ROOT ${PROJECT_SOURCE_DIR}/Library/Klampt)
LIST(APPEND CMAKE_MODULE_PATH "${KLAMPT_ROOT}/CMakeModules")
LIST(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")
INCLUDE(CMakeColors)
MESSAGE(COLOR_WHITE ${DEPENDENCIES})
set(CMAKE_C_COMPILER "gcc-5")
set(CMAKE_CXX_COMPILER "g++-5")

SET(CMAKE_BUILD_TYPE Debug)

###############################################################################
MESSAGE("${CMAKE_MODULE_PATH}")
###############################################################################
FIND_PACKAGE(ompl 1.4.0 REQUIRED)
SET(DEPENDENCIES ${DEPENDENCIES} OMPL)
MESSAGE($OMPL_VERSION)

### OMPL REquirements: CMAKE 2.8.7, Eigen 3.3, Boost 1.55, compile both with
### same libs

find_package(PkgConfig)
pkg_search_module(Eigen3 3.3 REQUIRED eigen3)
SET(DEPENDENCIES ${DEPENDENCIES} Eigen3)
#INCLUDE_DIRECTORIES(SYSTEM ${Eigen3_INCLUDE_DIRS})

FIND_PACKAGE(Klampt REQUIRED)
SET(DEPENDENCIES ${DEPENDENCIES} KLAMPT)

FIND_PACKAGE(Boost 1.55.0 REQUIRED COMPONENTS filesystem thread serialization system)
SET(DEPENDENCIES ${DEPENDENCIES} Boost)


MESSAGE(COLOR_WHITE "-------------------------------------")
MESSAGE(COLOR_WHITE "-- INCLUDING DEPENDENCIES")
MESSAGE(COLOR_WHITE "-------------------------------------")
MESSAGE(COLOR_WHITE "")

FOREACH(DEPENDENCY ${DEPENDENCIES})
  MESSAGE(COLOR_WHITE "-- ${DEPENDENCY}")
  MESSAGE("VERSION   :  ${${DEPENDENCY}_VERSION}")
  MESSAGE("LIBRARIES :  ${${DEPENDENCY}_LIBRARIES} ${${DEPENDENCY}_LIBS}")
  SET(ORTHOKLAMPT_LIBRARIES ${ORTHOKLAMPT_LIBRARIES} ${${DEPENDENCY}_LIBRARIES} ${${DEPENDENCY}_LIBS})
  MESSAGE("INCLUDE   :  ${${DEPENDENCY}_INCLUDE_DIRS} ${${DEPENDENCY}_INCLUDE_DIR}")
  INCLUDE_DIRECTORIES(${${DEPENDENCY}_INCLUDE_DIRS})
  INCLUDE_DIRECTORIES(${${DEPENDENCY}_INCLUDE_DIR})
  ADD_DEFINITIONS(${${DEPENDENCY}_DEFINITIONS})
ENDFOREACH(DEPENDENCY)

SET(ORTHOKLAMPT_LIBRARIES ${ORTHOKLAMPT_LIBRARIES} ${OMPLAPP_LIBRARIES} ompl_app_base)

INCLUDE_DIRECTORIES(src)

FOREACH(SOURCETYPE cpp cc C cxx c)
FOREACH(SUBDIR ${ORTHOKLAMPT_SUBDIRS})
  FILE(GLOB SUBDIR_SRC src/${SUBDIR}/*.${SOURCETYPE})
  SET(ORTHOKLAMPT_SRC ${ORTHOKLAMPT_SRC} ${SUBDIR_SRC})
  FILE(GLOB SUBDIR_SRC src/${SUBDIR}/*/*.${SOURCETYPE})
  SET(ORTHOKLAMPT_SRC ${ORTHOKLAMPT_SRC} ${SUBDIR_SRC})
  FILE(GLOB SUBDIR_SRC src/${SUBDIR}/*/*/*.${SOURCETYPE})
  SET(ORTHOKLAMPT_SRC ${ORTHOKLAMPT_SRC} ${SUBDIR_SRC})
  FILE(GLOB SUBDIR_SRC src/${SUBDIR}/*/*/*/*.${SOURCETYPE})
  SET(ORTHOKLAMPT_SRC ${ORTHOKLAMPT_SRC} ${SUBDIR_SRC})
ENDFOREACH(SUBDIR)
ENDFOREACH(SOURCETYPE)

FILE(GLOB SUBDIR_SRC src/*.cpp)
SET(ORTHOKLAMPT_SRC ${ORTHOKLAMPT_SRC} ${SUBDIR_SRC})

MESSAGE(COLOR_WHITE "-------------------------------------")
MESSAGE(COLOR_WHITE "-- BUILDING EXECUTABLES")
MESSAGE(COLOR_WHITE "-------------------------------------")
MESSAGE(COLOR_WHITE "")

FILE(GLOB EXECUTABLES_SOURCES test/*.cpp)
LIST(SORT EXECUTABLES_SOURCES)

SET(EXECUTABLES ${EXECUTABLES_SOURCES})
FOREACH(FILENAME ${EXECUTABLES})
  GET_FILENAME_COMPONENT( EXECUTABLE ${FILENAME} NAME_WE)
  MESSAGE(COLOR_WHITE "-- ${EXECUTABLE}")
  ADD_EXECUTABLE( ${EXECUTABLE} ${FILENAME} ${ORTHOKLAMPT_SRC})
  TARGET_LINK_LIBRARIES(${EXECUTABLE} ${ORTHOKLAMPT_LIBRARIES})
  TARGET_LINK_LIBRARIES(${EXECUTABLE} qhull)
ENDFOREACH(FILENAME)
MESSAGE("-------------------------------------")

FILE(GLOB EXECUTABLES_SOURCES standalone/*.cpp)
LIST(SORT EXECUTABLES_SOURCES)

SET(EXECUTABLES ${EXECUTABLES_SOURCES})
FOREACH(FILENAME ${EXECUTABLES})
  GET_FILENAME_COMPONENT( EXECUTABLE ${FILENAME} NAME_WE)
  MESSAGE(COLOR_WHITE "-- ${EXECUTABLE}")
  ADD_EXECUTABLE( ${EXECUTABLE} ${FILENAME})
  TARGET_LINK_LIBRARIES(${EXECUTABLE} )
ENDFOREACH(FILENAME)
MESSAGE("-------------------------------------")

#SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -Werror" )
#SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wdeprecated-declarations -std=c++11 -g -Wall -Werror -O3 -fmax-errors=10" )
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wdeprecated-declarations -std=c++11 -g -Wall -Werror -O3 -fmax-errors=10" )
MESSAGE("Compilers: ${CMAKE_CXX_COMPILER} ${CMAKE_C_COMPILER}")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

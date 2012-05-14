project(mordred-planet)


SET( OGRE_INCLUDE_DIR "${OGRE_INCLUDE_DIR}" CACHE STRING
   "OGRE_INCLUDE_DIR" FORCE )
SET( OGRE_LIBS "${OGRE_LIBS}" CACHE STRING
   "OGRE_LIBS" FORCE ) # OIS;OgreMain
SET( OGRE_LIB_LINK_DIR "${OGRE_LIB_LINK_DIR}" CACHE STRING
   "OGRE_LIB_LINK_DIR" FORCE ) 


SET( NOISEPP_INCLUDE_DIR "${NOISEPP_INCLUDE_DIR}" CACHE STRING
   "NOISEPP_INCLUDE_DIR" FORCE )
 # /mnt/sdb1/super/dump/foundations/projects/noisepp/noisepp.extended/noisepp/core;
 # /mnt/sdb1/super/dump/foundations/projects/noisepp/noisepp.extended/noisepp/threadpp;
 # /mnt/sdb1/super/dump/foundations/projects/noisepp/noisepp.extended/noisepp/utils
SET( NOISEPP_LIBS "${NOISEPP_LIBS}" CACHE STRING
   "NOISEPP_LIBS" FORCE ) # noisepp
SET( NOISEPP_LIB_LINK_DIR "${NOISEPP_LIB_LINK_DIR}" CACHE STRING
   "NOISEPP_LIB_LINK_DIR" FORCE ) # /mnt/sdb1/super/dump/foundations/projects/noisepp/noisepp.extended/build/lib/Release


include_directories(./include ./src ${OGRE_INCLUDE_DIR} ${NOISEPP_INCLUDE_DIR})

link_directories(${NOISEPP_LIB_LINK_DIR} ${OGRE_LIB_LINK_DIR})

add_executable(mordred-planet
  main.cpp
  src/planet_volume.cpp
  src/ogre_utility.cpp
  src/BaseApplication.cpp)

target_link_libraries(mordred-planet ${NOISEPP_LIBS} ${OGRE_LIBS})


add_library(gpunoise src/gpunoise/add3d.cpp src/gpunoise/const3d.cpp src/gpunoise/module3d.cpp)


add_subdirectory(include)

add_subdirectory(src)
add_subdirectory(media)
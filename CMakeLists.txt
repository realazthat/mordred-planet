project(mordred-planet)


SET( OGRE_INCLUDE_DIR "${OGRE_INCLUDE_DIR}" CACHE STRING
   "OGRE_INCLUDE_DIR" FORCE )
SET( OGRE_LIBS "${OGRE_LIBS}" CACHE STRING
   "OGRE_LIBS" FORCE ) # OIS;OgreMain
SET( OGRE_LIB_LINK_DIR "${OGRE_LIB_LINK_DIR}" CACHE STRING
   "OGRE_LIB_LINK_DIR" FORCE ) 

include_directories(./include ./src ${OGRE_INCLUDE_DIR})

add_executable(mordred-planet
  main.cpp
  src/planet_volume.cpp
  src/ogre_utility.cpp
  src/BaseApplication.cpp)

target_link_libraries(mordred-planet ${OGRE_LIBS})



add_subdirectory(include)

add_subdirectory(src)
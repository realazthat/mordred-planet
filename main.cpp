#include <iostream>

#include "tree/tree.h"
#include "square/square.h"

#include <OGRE/OgreMovableObject.h>
#include "src/planet_volume.h"






int main(int argc, char **argv) {
  
  using namespace Ogre;
  
  AxisAlignedBox bounds(Vector3(0,0,0), Vector3(1024,1024,1024));
  Real radius = 300;
  
  planet_renderer_t planet_renderer(bounds, radius);
  
  
  return 0;
}

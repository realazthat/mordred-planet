#include <iostream>

#include "tree/tree.h"
#include "square/square.h"

#include <OGRE/OgreMovableObject.h>
#include "planet_volume.h"

#include "BaseApplication.h"
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreManualObject.h>
#include <ogre_utility.h>

struct MordredApplication
  : BaseApplication
{
  MordredApplication();
  
  virtual void createScene();
  virtual bool keyPressed(const OIS::KeyEvent& arg);
  
private:
  boost::scoped_ptr< planet_renderer_t > planet_renderer;
  Ogre::ManualObject* debug_manual;
};

MordredApplication::MordredApplication()
  : debug_manual(NULL)
{

}


void MordredApplication::createScene()
{
  using namespace Ogre;
  
  AxisAlignedBox bounds(Vector3(0,0,0), Vector3(1024,1024,1024));
  Real radius = 300;
  std::size_t max_levels = 8;
  
  {
    SceneNode* ogre_head_node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    Entity* ogre_entity = mSceneMgr->createEntity("ogrehead.mesh");
    ogre_head_node->attachObject(ogre_entity);
  }
  
  {
    ManualObject* axis_man = mSceneMgr->createManualObject();
    
    axis_man->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_LIST);
    
    draw_axis(axis_man);
    
    axis_man->end();
    
    SceneNode* axis_sn = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    
    axis_sn->attachObject(axis_man);
  }
  
  {
    SceneNode* planet_scene_node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    
    planet_renderer.reset(new planet_renderer_t(bounds, radius, max_levels));
    
    planet_renderer->mcamera = mCamera;
    
    planet_scene_node->attachObject(planet_renderer.get());
    
  }
}

bool MordredApplication::keyPressed(const OIS::KeyEvent& arg)
{
  if (arg.key == OIS::KC_1)
  {
    if (mCamera->getPolygonMode() != Ogre::PM_WIREFRAME)
    {
      mCamera->setPolygonMode(Ogre::PM_WIREFRAME);
    } else {
      mCamera->setPolygonMode(Ogre::PM_SOLID);
    }
  } else if (arg.key == OIS::KC_2) {
    if(!planet_renderer->mcamera)
      planet_renderer->mcamera = mCamera;
    else
      planet_renderer->mcamera = NULL;
  } else if (arg.key == OIS::KC_3) {
    if (debug_manual)
    {
      debug_manual->setVisible(!debug_manual->isVisible());
    }
  } else if (arg.key == OIS::KC_4) {
    /*
    if (planet_renderer->show_transitions == planet_volume::NONE)
      planet->show_transitions = planet_volume::ALL;
    else if (planet->show_transitions == planet_volume::ALL)
      planet->show_transitions = planet_volume::PATCH_ONLY;
    else if (planet->show_transitions == planet_volume::PATCH_ONLY)
      planet->show_transitions = planet_volume::NONE;
    else
      BOOST_ASSERT(false);
    */
    
    planet_renderer->render_transitions();
    //std::cout << "vrenderer->show_transitions = " << planet->show_transitions << std::endl;
  }

  return BaseApplication::keyPressed(arg);
}







#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
      (void)argc;
      (void)argv;
      


      try {
        // Create application object
        MordredApplication app;
        
        app.go();
      } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
          MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
          std::cerr << "An exception has occured: " <<
              e.getFullDescription().c_str() << std::endl;
#endif
      }

      return 0;
    }

#ifdef __cplusplus
}
#endif
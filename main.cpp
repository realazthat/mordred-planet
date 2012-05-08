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
  virtual bool mouseMoved(const OIS::MouseEvent& arg);
private:
  boost::scoped_ptr< planet_renderer_t > planet_renderer;
  Ogre::SceneNode* planet_scene_node;
  Ogre::ManualObject* debug_manual;
};

MordredApplication::MordredApplication()
  : planet_scene_node(NULL)
  , debug_manual(NULL)
{

}


void MordredApplication::createScene()
{
  using namespace Ogre;
  
  //mSceneMgr->setCameraRelativeRendering(true);
  
  mCamera->setNearClipDistance(Real(1) / Real(10));
  
  
  Real radius = 6353;
  AxisAlignedBox bounds(Vector3(-radius,-radius,-radius), Vector3(radius,radius,radius));
  std::size_t max_levels = 33;
  mCamera->setFarClipDistance(0);
  mCamera->setPosition(radius + 500,radius + 500,radius + 500);
  
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
    planet_scene_node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    
    planet_renderer.reset(new planet_renderer_t(bounds, radius, max_levels));
    
    planet_renderer->mcamera = mCamera;
    
    planet_scene_node->attachObject(planet_renderer.get());
    planet_scene_node->setPosition(0,0,0);
    
    //mCamera->detachFromParent();
    //planet_scene_node->attachObject(mCamera);
    //mCamera->setPosition(radius + 500,radius + 500,radius + 500);
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


bool MordredApplication::mouseMoved(const OIS::MouseEvent& evt)
{
  using namespace Ogre;
  
  if (mKeyboard->isKeyDown(OIS::KC_E))
  {
    if (evt.state.Z.rel > 0) {
      mCamera->setNearClipDistance(mCamera->getNearClipDistance() * Real(2));
    } else if (evt.state.Z.rel < 0) {
      mCamera->setNearClipDistance(mCamera->getNearClipDistance() / Real(2));
    }
  } else if(!evt.state.buttonDown(OIS::MB_Right))
  {
    Vector3 scale = planet_scene_node->getScale();
    Vector3 world_camera_position = mCamera->getDerivedPosition();
    
    //Vector3 planet_camera_position = planet_scene_node->convertWorldToLocalPosition(world_camera_position);
    
    if (evt.state.Z.rel > 0) {
      planet_scene_node->setScale(scale * Real(2));
      mCamera->setPosition(mCamera->getPosition() * Real(2));
    } else if (evt.state.Z.rel < 0) {
      planet_scene_node->setScale(scale / Real(2)); 
      mCamera->setPosition(mCamera->getPosition() / Real(2));
    }
  }
  
  return BaseApplication::mouseMoved(evt);
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
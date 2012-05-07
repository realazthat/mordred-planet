/*
-----------------------------------------------------------------------------
Filename:    BaseApplication.cpp
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _ 
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/                              
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#include "BaseApplication.h"

#include <OGRE/OgreString.h>
#include <OGRE/OgreRoot.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreCamera.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreConfigFile.h>

#include <stdexcept>


//-------------------------------------------------------------------------------------
BaseApplication::BaseApplication(void)
  : mRoot(0)
  , mViewport(0)
  , mCamera(0)
  , mSceneMgr(0)
  , mWindow(0)
  , mResourcesCfg(Ogre::StringUtil::BLANK)
  , mPluginsCfg(Ogre::StringUtil::BLANK)
  , mShutDown(false)
  , mInputManager(0)
  , mMouse(0)
  , mKeyboard(0)
  , speed(30)
{

  
}


//-------------------------------------------------------------------------------------
BaseApplication::~BaseApplication(void)
{

    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}


bool BaseApplication::configure_plugins()
{
  
  return true;
}


//-------------------------------------------------------------------------------------
bool BaseApplication::configure_window(void)
{
  // Show the configuration dialog and initialise the system
  // You can skip this and use root.restoreConfig() to load configuration
  // settings if you were sure there are valid ones saved in ogre.cfg
  
  if(mRoot->restoreConfig() || mRoot->showConfigDialog())
  {
      // If returned true, user clicked OK so initialise
      // Here we choose to let the system create a default rendering window by passing 'true'
      mWindow = mRoot->initialise(true, "TutorialApplication Render Window");

      return true;
  }
  else
  {
      return false;
  }
}
//-------------------------------------------------------------------------------------
void BaseApplication::chooseSceneManager(void)
{
  // Get the SceneManager, in this case a generic one
  mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
  if (!mSceneMgr)
    throw std::runtime_error("Unable to create scene manager");
}
//-------------------------------------------------------------------------------------
void BaseApplication::createCamera(void)
{
  using namespace Ogre;
  // Create the camera
  mCamera = mSceneMgr->createCamera("PlayerCam");

  SceneNode* camera_node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
  
  camera_node->attachObject(mCamera);
  
  // Position it at 500 in Z direction
  mCamera->setPosition(Ogre::Vector3(0,0,80));
  // Look back along -Z
  mCamera->lookAt(Ogre::Vector3(0,0,-300));
  mCamera->setNearClipDistance(.1);

}




//-------------------------------------------------------------------------------------

void BaseApplication::createInputDevices()
{
  Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
  OIS::ParamList pl;
  std::size_t windowHnd = 0;
  std::ostringstream windowHndStr;

  mWindow->getCustomAttribute("WINDOW", &windowHnd);
  
  windowHndStr << windowHnd;
  pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

#if defined OIS_WIN32_PLATFORM
  pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
  pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
  pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
  pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
#elif defined OIS_LINUX_PLATFORM

///Turn this on for debug; don't wanna get stuck with mouse lock during a breakpoint
#if 1
  pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
  pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
  pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
  pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
#endif
#if 0
  pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("true")));
  pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("true")));
  //pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
  pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
#endif
  
#endif
  
  mInputManager = OIS::InputManager::createInputSystem( pl );

  mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
  mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

  mMouse->setEventCallback(this);
  mKeyboard->setEventCallback(this);

}

//-------------------------------------------------------------------------------------
void BaseApplication::createFrameListener(void)
{
  //Set initial mouse clipping size
  windowResized(mWindow);


  mRoot->addFrameListener(this);
  //Register as a Window listener
  Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);
}
//-------------------------------------------------------------------------------------
void BaseApplication::destroyScene(void)
{
}
//-------------------------------------------------------------------------------------
void BaseApplication::createViewports(void)
{
  
  // Create one viewport, entire window
  mViewport = mWindow->addViewport(mCamera);
  
  //mViewport->setClearEveryFrame(true);


  // Alter the camera aspect ratio to match the viewport
  //mCamera->setAspectRatio(Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight()));
}
//-------------------------------------------------------------------------------------
void BaseApplication::setupResources(void)
{
  if (mResourcesCfg.empty())
    return;

  // Load resource paths from config file
  Ogre::ConfigFile cf;
  cf.load(mResourcesCfg);

  // Go through all sections & settings in the file
  Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

  Ogre::String secName, typeName, archName;
  while (seci.hasMoreElements())
  {
    secName = seci.peekNextKey();
    Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
    Ogre::ConfigFile::SettingsMultiMap::iterator i;
    for (i = settings->begin(); i != settings->end(); ++i)
    {
      typeName = i->first;
      archName = i->second;
      Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          archName, typeName, secName);
    }
  }
}
//-------------------------------------------------------------------------------------
void BaseApplication::createResourceListener(void)
{

}
//-------------------------------------------------------------------------------------
void BaseApplication::loadResources(void)
{
  Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
//-------------------------------------------------------------------------------------

#ifndef __native_client__
void BaseApplication::go(void)
{
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";

    if (!setup())
        return;

    mRoot->startRendering();

    // clean up
    destroyScene();
}
#else // #ifndef __native_client__

bool BaseApplication::load_nacl()
{
  mResourcesCfg = "";
  mPluginsCfg = "";

  return setup();
  
  ///FIXME: someones gotta call destroyScene()
}
#endif // #ifndef __native_client__


//-------------------------------------------------------------------------------------
bool BaseApplication::setup(void)
{
  using namespace Ogre;
  mRoot = new Root(mPluginsCfg);

  if (!configure_plugins())
    return false;
  
  
  setupResources();

  if (!configure_window())
    return false;

  chooseSceneManager();
  createCamera();
  createViewports();

  // Set default mipmap level (NB some APIs ignore this)
  Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

  // Create any resource listeners (for loading screens)
  createResourceListener();
  // Load resources
  loadResources();

  
  createInputDevices();
  
  // Create the scene
  createScene();

  createFrameListener();

  ///FIXME:??
  //mRoot->getRenderSystem()->_initRenderTargets();

  return true;
}
//-------------------------------------------------------------------------------------
bool BaseApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
  using namespace Ogre;
  
  if(mWindow->isClosed())
    return false;

  if(mShutDown)
    return false;

  //Need to capture/update each device
  mKeyboard->capture();
  mMouse->capture();
  
  
  Vector3 camera_move_vector(Vector3::ZERO);
  Vector3 speedv(speed,speed,speed);
  Vector3 forward(0,0,-1);
  Vector3 left(-1,0,0);
  Vector3 right(1,0,0);
  Vector3 back(0,0,1);
  Vector3 up(0,1,0);
  
  if (mKeyboard->isKeyDown(OIS::KC_LSHIFT))
  {
    speedv *= 5;
  }
  
  Vector3 distance = speedv * evt.timeSinceLastFrame;
  
  if (mKeyboard->isKeyDown(OIS::KC_W))
  {
    camera_move_vector += forward * distance;
  }
  
  if (mKeyboard->isKeyDown(OIS::KC_A))
  {
    camera_move_vector += left * distance;
  }
  
  if (mKeyboard->isKeyDown(OIS::KC_D))
  {
    camera_move_vector += right * distance;
  }
  
  if (mKeyboard->isKeyDown(OIS::KC_S))
  {
    camera_move_vector += back * distance;
  }
  
  if (mKeyboard->isKeyDown(OIS::KC_SPACE))
  {
    camera_move_vector += up * distance;
  }
  
  camera().moveRelative(camera_move_vector);
  
  return true;
}
//-------------------------------------------------------------------------------------
bool BaseApplication::keyPressed( const OIS::KeyEvent &arg )
{
  //Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
  //Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
      
  if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
  {
    Ogre::String newVal;
    Ogre::PolygonMode pm;

    switch (mCamera->getPolygonMode())
    {
    case Ogre::PM_SOLID:
      newVal = "Wireframe";
      pm = Ogre::PM_WIREFRAME;
      break;
    case Ogre::PM_WIREFRAME:
      newVal = "Points";
      pm = Ogre::PM_POINTS;
      break;
    default:
      newVal = "Solid";
      pm = Ogre::PM_SOLID;
    }

    mCamera->setPolygonMode(pm);
  }
  else if(arg.key == OIS::KC_F5)   // refresh all textures
  {
      Ogre::TextureManager::getSingleton().reloadAll();
  }
  else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
  {
      mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
  }
  else if (arg.key == OIS::KC_ESCAPE)
  {
      mShutDown = true;
  }
  
  return true;
}

bool BaseApplication::keyReleased( const OIS::KeyEvent& arg )
{
  return true;
}



bool BaseApplication::mouseMoved( const OIS::MouseEvent& evt )
{
  using namespace Ogre;
  
  if (evt.state.buttonDown(OIS::MB_Right))
  {
    Real xratio = -Real(evt.state.X.rel) / Real(renderWindow().getWidth());
    Real yratio = -Real(evt.state.Y.rel) / Real(renderWindow().getHeight());
    
    Radian full_circle( Ogre::Math::PI * 2.0);
    
    camera().yaw(full_circle * xratio);
    camera().pitch(full_circle * yratio);
    
    if (evt.state.Z.rel > 0) {
      speed *= 2;
    } else if (evt.state.Z.rel < 0) {
      speed /= 2;
    }
  }
  
  
  return true;
}

bool BaseApplication::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  return true;
}

bool BaseApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  return true;
}

//Adjust mouse clipping area
void BaseApplication::windowResized(Ogre::RenderWindow* rw)
{
  unsigned int width, height, depth;
  int left, top;
  rw->getMetrics(width, height, depth, left, top);

  const OIS::MouseState &ms = mMouse->getMouseState();
  ms.width = width;
  ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void BaseApplication::windowClosed(Ogre::RenderWindow* rw)
{
  //Only close for window that created OIS (the main window in these demos)
  if( rw == mWindow )
  {
    if( mInputManager )
    {
      mInputManager->destroyInputObject( mMouse );
      mInputManager->destroyInputObject( mKeyboard );

      OIS::InputManager::destroyInputSystem(mInputManager);
      mInputManager = 0;
    }
  }
}

Ogre::RenderWindow& BaseApplication::renderWindow()
{
  if (!mWindow)
    throw std::runtime_error("renderWindow() called, but invalid mWindow");
  return *mWindow;
}

Ogre::Root& BaseApplication::root()
{
  if (!mRoot)
    throw std::runtime_error("root() called, but invalid mRoot");
  return *mRoot;
}
Ogre::Camera& BaseApplication::camera()
{
  if (!mCamera)
    throw std::runtime_error("camera() called, but invalid mCamera");
  return *mCamera;
}

Ogre::Viewport& BaseApplication::viewPort()
{
  if (!mViewport)
    throw std::runtime_error("rootviewPort called, but invalid mViewport");
  return *mViewport;
}


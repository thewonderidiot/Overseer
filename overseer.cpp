#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <dfhack/DFTileTypes.h>
#include <iostream>

#include "SimpleIni.h"

#include <osgViewer/GraphicsHandleWin32>


#include "Overseer.h"

using namespace std;
using namespace osg;

Overseer::Overseer()
{
    keepRendering = true;
    root = new Group();
    velocity.set(0,0,0);
    shiftspeed = 1;
}

Overseer::~Overseer()
{
    im->destroyInputObject(mouse);
    im->destroyInputObject(keyboard);
    OIS::InputManager::destroyInputSystem(im);
    im = 0;
}

void Overseer::loadSettings()
{
    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile("overseer.ini");
    enableRamps = ini.GetBoolValue("overseer","enableRamps",true);
	fullscreen = ini.GetBoolValue("overseer","fullscreen");
	tristrip = ini.GetBoolValue("overseer","tristripping");
	startz = ini.GetLongValue("overseer","zstart",0);
	mouseSensitivity = ini.GetLongValue("overseer","mouseSensitivity",2347);
}

bool Overseer::connectToDF()
{
    try
    {
        DFMgr = new DFHack::ContextManager("Memory.xml");
        DF = DFMgr->getSingleContext();
        DF->Attach();
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
        return 1;
    }

    Maps = DF->getMaps();
    if(!Maps->Start())
    {
        cerr << "Can't init map." << endl;
        return 1;
    }
    cout << "Connected to Dwarf Fortress!" << endl;
}

bool Overseer::go()
{
    loadSettings();
    connectToDF();
    dg = new DwarfGeometry(Maps, root, startz, enableRamps, tristrip);
    dg->start();
    dg->drawGeometry();
    DF->Detach();

    osgViewer::Viewer viewer;
    viewer.setSceneData(root);
    if (!fullscreen) viewer.setUpViewInWindow(20, 20, 1044, 788);
    viewer.realize();
    //viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
   // viewer.addEventHandler(new osgViewer::StatsHandler);
    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);
    osgViewer::GraphicsHandleWin32* hwnd = dynamic_cast<osgViewer::GraphicsHandleWin32*>(windows[0]);
    OIS::InputManager *im = OIS::InputManager::createInputSystem((size_t)hwnd->getHWND());
    OIS::Mouse *mouse = static_cast<OIS::Mouse*>(im->createInputObject(OIS::OISMouse, true));
    OIS::Keyboard *keyboard = static_cast<OIS::Keyboard*>(im->createInputObject(OIS::OISKeyboard,true));
    mouse->setEventCallback(this);
    keyboard->setEventCallback(this);
    Camera *c = viewer.getCamera();
    int camz = dg->getGeometryMax();
    c->setViewMatrixAsLookAt(Vec3(0,0,camz),Vec3(1,1,camz),Vec3(0,0,1));
    Matrixd test = c->getViewMatrix();
    Vec3d eye;
    Quat qrot;
    Vec3d scale;
    Quat so;
    test.decompose(eye,qrot,scale,so);
    rot.makeRotate(qrot);
    yaw.makeRotate(0,Vec3(0,1,0));
    pitch.makeRotate(0,Vec3(0,0,1));
    while (keepRendering)
    {
        Timer_t startFrameTick = osg::Timer::instance()->tick();
        mouse->capture();
        keyboard->capture();
        viewer.frame();
        Timer_t endFrameTick = osg::Timer::instance()->tick();
        int dt = endFrameTick-startFrameTick;
        eye += yaw*pitch*velocity*dt*shiftspeed;
        c->setViewMatrix(rot*Matrixd::translate(eye)*yaw*pitch);
    }
}

bool Overseer::keyPressed(const OIS::KeyEvent &e)
{
    switch (e.key)
    {
    case OIS::KC_W:
        velocity.set(velocity.x(),velocity.y(),velocity.z()+.000005);
        break;
    case OIS::KC_S:
        velocity.set(velocity.x(),velocity.y(),velocity.z()-.000005);
        break;
    case OIS::KC_A:
        velocity.set(velocity.x()+.000005,velocity.y(),velocity.z());
        break;
    case OIS::KC_D:
        velocity.set(velocity.x()-.000005,velocity.y(),velocity.z());
        break;
    case OIS::KC_R:
        velocity.set(velocity.x(),velocity.y()-.000005,velocity.z());
        break;
    case OIS::KC_F:
        velocity.set(velocity.x(),velocity.y()+.000005,velocity.z());
        break;
    case OIS::KC_RSHIFT:
    case OIS::KC_LSHIFT:
        shiftspeed = 10;
        break;
    case OIS::KC_ESCAPE:
        keepRendering = false;
        break;
    default:
        break;
    }
    return true;
}
bool Overseer::keyReleased(const OIS::KeyEvent &e)
{
    switch (e.key)
    {
    case OIS::KC_W:
        velocity.set(velocity.x(),velocity.y(),velocity.z()-.000005);
        break;
    case OIS::KC_S:
        velocity.set(velocity.x(),velocity.y(),velocity.z()+.000005);
        break;
    case OIS::KC_A:
        velocity.set(velocity.x()-.000005,velocity.y(),velocity.z());
        break;
    case OIS::KC_D:
        velocity.set(velocity.x()+.000005,velocity.y(),velocity.z());
        break;
    case OIS::KC_R:
        velocity.set(velocity.x(),velocity.y()+.000005,velocity.z());
        break;
    case OIS::KC_F:
        velocity.set(velocity.x(),velocity.y()-.000005,velocity.z());
        break;
    case OIS::KC_RSHIFT:
    case OIS::KC_LSHIFT:
        shiftspeed = 1;
        break;
    default:
        break;
    }
    return true;
}
bool Overseer::mouseMoved(const OIS::MouseEvent &e)
{
    yaw *= Matrixd::rotate(inDegrees(0.2*e.state.X.rel), Vec3(0,1,0));
    pitch *= Matrixd::rotate(inDegrees(0.2*e.state.Y.rel), Vec3(1,0,0));
    //rot *= yaw;
    return true;
}
bool Overseer::mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    return true;
}
bool Overseer::mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
    return true;
}

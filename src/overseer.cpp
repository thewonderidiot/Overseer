#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <dfhack/DFTileTypes.h>
#include <osgDB/WriteFile>
#include <osgUtil/TriStripVisitor>
#include <iostream>

#include "SimpleIni.h"

#include <osgViewer/GraphicsHandleWin32>

#include "Overseer.h"
#include <windows.h>


#include <osg/Light>

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
	fullscreen = ini.GetBoolValue("overseer","fullscreen");
	tristrip = ini.GetBoolValue("overseer","tristripping");
    doCulling = ini.GetBoolValue("overseer","culling");
    doVeggies = ini.GetBoolValue("overseer","vegetation");

	startz = ini.GetLongValue("overseer","zstart",0);
	string sens = ini.GetValue("overseer","mouseSensitivity",".2");
	string mov = ini.GetValue("overseer","moveSpeed",".000005");
	string ch = ini.GetValue("overseer","ceilingHeight",".05");
	string tz = ini.GetValue("overseer","treeSize","1");

	char *end;
	moveSpeed = strtod(mov.c_str(),&end);
	mouseSensitivity = strtod(sens.c_str(),&end);
    ceilingHeight = strtod(ch.c_str(), &end);
    treeSize = strtod(tz.c_str(),&end);
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
    Mats = DF->getMaterials();
    Cons = DF->getConstructions();
    Vegs = DF->getVegetation();
    if(!Maps->Start())
    {
        cerr << "Can't init map." << endl;
        return 1;
    }
    if (!Mats->ReadInorganicMaterials() || !Mats->ReadOrganicMaterials())
    {
        cerr << "Can't init materials." << endl;
        return 1;
    }

    cout << "Connected to Dwarf Fortress!" << endl;
    return true;
}

bool Overseer::go()
{
    loadSettings();
    connectToDF();

    dg = new DwarfGeometry(Maps, Mats, Cons, Vegs, root, startz, ceilingHeight, treeSize, tristrip, doCulling);
    dg->start();
    dg->drawGeometry();
    if (doVeggies) dg->drawVegetation();
    dg->drawSkybox();
    DF->Detach();

    osgViewer::Viewer viewer;
    viewer.setSceneData(root);

    StateSet *ss = root->getOrCreateStateSet();
    ss->setMode(GL_LIGHTING, StateAttribute::ON);
    ss->setMode(GL_LIGHT0, StateAttribute::ON);
    ss->setMode(GL_LIGHT1, StateAttribute::ON);

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

    ref_ptr<Light> light = new Light;
    /*light->setLightNum(0);
    light->setAmbient(Vec4(.1,.1,.1,1.0));
    light->setDiffuse(Vec4(1,1,1,1.0));
    light->setSpecular(Vec4(.8,.8,.8,1.0));
    light->setPosition(Vec4(0.0,0.0,0.0,1.0));
    //light->setConstantAttenuation(.05);
    light->setQuadraticAttenuation(.005);
    //light->setDirection(Vec3(0.0,0.0,-1.0));
    //light->setSpotCutoff(25);
    ls = new LightSource;
    ls->setLight(light.get());
    ls->setReferenceFrame(LightSource::ABSOLUTE_RF);
    //root->addChild(ls.get());

    light = new Light;*/
    light->setLightNum(0);
    light->setAmbient(Vec4(.2,.2,.2,1));
    light->setDiffuse(Vec4(1,1,1,1));
    light->setSpecular(Vec4(1,1,1,1));
    light->setDirection(Vec3(1,1,-1));
    ref_ptr<LightSource> ls = new LightSource;
    ls->setLight(light.get());
    ls->setReferenceFrame(LightSource::ABSOLUTE_RF);
    root->addChild(ls.get());

    int camz = dg->getGeometryMax();

    light->setPosition(Vec4(-20,-20,camz+20,1));

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
    osgViewer::StatsHandler *s = new osgViewer::StatsHandler();
    s->setKeyEventTogglesOnScreenStats(osgGA::GUIEventAdapter::KEY_F1);
    s->setKeyEventPrintsOutStats(osgGA::GUIEventAdapter::KEY_F3);
    s->setKeyEventToggleVSync(osgGA::GUIEventAdapter::KEY_F2);
    viewer.addEventHandler(s);
    while (keepRendering)
    {
        Timer_t startFrameTick = osg::Timer::instance()->tick();
        mouse->capture();
        keyboard->capture();
        viewer.frame();
        Timer_t endFrameTick = osg::Timer::instance()->tick();
        int dt = endFrameTick-startFrameTick;
        eye += yaw*pitch*velocity*dt*shiftspeed;
        //light->setPosition(Vec4(eye.x(),eye.y(),eye.z(),50.0));
        c->setViewMatrix(rot*Matrixd::translate(eye)*yaw*pitch);
    }
    return true;
}

bool Overseer::keyPressed(const OIS::KeyEvent &e)
{
    switch (e.key)
    {
    case OIS::KC_W:
        velocity.set(velocity.x(),velocity.y(),velocity.z()+moveSpeed);
        break;
    case OIS::KC_S:
        velocity.set(velocity.x(),velocity.y(),velocity.z()-moveSpeed);
        break;
    case OIS::KC_A:
        velocity.set(velocity.x()+moveSpeed,velocity.y(),velocity.z());
        break;
    case OIS::KC_D:
        velocity.set(velocity.x()-moveSpeed,velocity.y(),velocity.z());
        break;
    case OIS::KC_R:
        velocity.set(velocity.x(),velocity.y()-moveSpeed,velocity.z());
        break;
    case OIS::KC_F:
        velocity.set(velocity.x(),velocity.y()+moveSpeed,velocity.z());
        break;
    case OIS::KC_X:
        OPENFILENAME ofn;
        char szFile[100];
    	ZeroMemory( &ofn , sizeof( ofn));
        ofn.lStructSize = sizeof ( ofn );
        ofn.hwndOwner = NULL  ;
        ofn.lpstrFile = szFile ;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof( szFile );
        ofn.lpstrFilter = "Wavefront (*.obj)\0*.obj\0Autodesk 3dsMax (*.3ds)\0*.3ds\0AC3D (*.ac)\0*.ac\0Autodesk DXF (*.dxf)\0*.dxf\0Lightwave (*.lwo)\0*.lwo\0PovRay (*.pov)\0*.pov\0";
        ofn.nFilterIndex =1;
        ofn.lpstrFileTitle = NULL ;
        ofn.nMaxFileTitle = 0 ;
        ofn.lpstrInitialDir=NULL ;
        ofn.lpstrDefExt="obj";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT ;

        if (!GetSaveFileName( &ofn )) break;
        fileName = new string(ofn.lpstrFile);
        cout << "Writing " << *fileName << "...";
        osgDB::writeNodeFile(*root,*fileName);
        cout << "done." << endl;
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
        velocity.set(velocity.x(),velocity.y(),velocity.z()-moveSpeed);
        break;
    case OIS::KC_S:
        velocity.set(velocity.x(),velocity.y(),velocity.z()+moveSpeed);
        break;
    case OIS::KC_A:
        velocity.set(velocity.x()-moveSpeed,velocity.y(),velocity.z());
        break;
    case OIS::KC_D:
        velocity.set(velocity.x()+moveSpeed,velocity.y(),velocity.z());
        break;
    case OIS::KC_R:
        velocity.set(velocity.x(),velocity.y()+moveSpeed,velocity.z());
        break;
    case OIS::KC_F:
        velocity.set(velocity.x(),velocity.y()-moveSpeed,velocity.z());
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
    yaw *= Matrixd::rotate(inDegrees(mouseSensitivity*e.state.X.rel), Vec3(0,1,0));
    pitch *= Matrixd::rotate(inDegrees(mouseSensitivity*e.state.Y.rel), Vec3(1,0,0));
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

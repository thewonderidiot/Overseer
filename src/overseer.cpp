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
#include "DwarfEvents.h"
#include <windows.h>


#include <osg/Light>

using namespace std;
using namespace osg;

Overseer::Overseer()
{
    root = new Group();
}

Overseer::~Overseer()
{
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
    //if (doVeggies) dg->drawVegetation();
    dg->drawSkybox();
    DF->Detach();

    osgViewer::Viewer viewer;
    viewer.setSceneData(root);

    /*StateSet *ss = root->getOrCreateStateSet();
    ss->setMode(GL_LIGHTING, StateAttribute::ON);
    ss->setMode(GL_LIGHT0, StateAttribute::ON);*/
    //ss->setMode(GL_LIGHT1, StateAttribute::ON);

    //viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
   // viewer.addEventHandler(new osgViewer::StatsHandler);
    //osgViewer::Viewer::Windows windows;
    //viewer.getWindows(windows);
    //osgViewer::GraphicsHandleWin32* hwnd = dynamic_cast<osgViewer::GraphicsHandleWin32*>(windows[0]);
    Camera *c = viewer.getCamera();
    //viewer.
    ref_ptr<Light> light = new Light;
    /*light->setLightNum(1);
    light->setAmbient(Vec4(.1,.1,.1,1.0));
    light->setDiffuse(Vec4(1,1,1,1.0));
    light->setSpecular(Vec4(.8,.8,.8,1.0));
    light->setPosition(Vec4(0.0,0.0,0.0,1.0));
    //light->setConstantAttenuation(.05);
    light->setQuadraticAttenuation(.005);
    //light->setDirection(Vec3(0.0,0.0,-1.0));
    //light->setSpotCutoff(25);*/
    //ref_ptr<LightSource> ls = new LightSource;
    //ls->setLight(light.get());
    //ls->setReferenceFrame(LightSource::ABSOLUTE_RF);
    //root->addChild(ls.get());

    //light->setLightNum(1);
    light->setAmbient(Vec4(.6,.6,.6,1));
    light->setDiffuse(Vec4(1,1,1,1));
    light->setSpecular(Vec4(.8,.8,.8,1));
    viewer.setLight(light);
    light->setDirection(Vec3(1,1,-1));
    //ls->setLight(light.get());
    //ls->setReferenceFrame(LightSource::ABSOLUTE_RF);
    //root->addChild(ls.get());

    int camz = dg->getGeometryMax();

    //light->setPosition(Vec4(-20,-20,camz+20,1));

    c->setViewMatrixAsLookAt(Vec3(0,0,camz),Vec3(1,1,camz),Vec3(0,0,1));

    osgViewer::StatsHandler *s = new osgViewer::StatsHandler();
    s->setKeyEventTogglesOnScreenStats(osgGA::GUIEventAdapter::KEY_F1);
    s->setKeyEventPrintsOutStats(osgGA::GUIEventAdapter::KEY_F3);
    s->setKeyEventToggleVSync(osgGA::GUIEventAdapter::KEY_F2);
    //viewer.addEventHandler(s);
    if (!fullscreen) viewer.setUpViewInWindow(20, 20, 1044, 788);
    viewer.realize();
    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);
    DwarfEvents *de = new DwarfEvents(windows[0],c, moveSpeed, mouseSensitivity);
    viewer.addEventHandler(de);
    int dt = 0;
    viewer.setLightingMode(osgViewer::Viewer::SKY_LIGHT);
    while (de->update(dt) && viewer.isRealized())
    {
        Timer_t startFrameTick = osg::Timer::instance()->tick();
        viewer.frame();
        Timer_t endFrameTick = osg::Timer::instance()->tick();
        dt = endFrameTick-startFrameTick;
    }
    return true;
}

/*bool Overseer::keyPressed(const OIS::KeyEvent &e)
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
}*/
/*bool Overseer::keyReleased(const OIS::KeyEvent &e)
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
}*/
/*bool Overseer::mouseMoved(const OIS::MouseEvent &e)
{
    yaw *= Matrixd::rotate(inDegrees(mouseSensitivity*e.state.X.rel), Vec3(0,1,0));
    pitch *= Matrixd::rotate(inDegrees(mouseSensitivity*e.state.Y.rel), Vec3(1,0,0));
    //rot *= yaw;
    return true;
}*/

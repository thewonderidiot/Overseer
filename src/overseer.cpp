#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <dfhack/DFTileTypes.h>
#include <osgUtil/TriStripVisitor>
#include <iostream>

#include "SimpleIni.h"

#include <osgViewer/GraphicsHandleWin32>

#include "Overseer.h"
#include "DwarfEvents.h"


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
	fullscreen = ini.GetBoolValue("overseer","fullscreen",false);
	tristrip = ini.GetBoolValue("overseer","tristripping",false);
    doCulling = ini.GetBoolValue("overseer","culling",true);
    doVeggies = ini.GetBoolValue("overseer","vegetation",true);
    useHeadlight = ini.GetBoolValue("overseer","headlight",false);
    useShaders = ini.GetBoolValue("overseer","useShaders",false);
    doImageScaling = ini.GetBoolValue("overseer","doImageScaling",true);

	startz = ini.GetLongValue("overseer","zstart",0);
	imageSize = ini.GetLongValue("overseer","imageSize", 512);
	string sens = ini.GetValue("overseer","mouseSensitivity",".2");
	string mov = ini.GetValue("overseer","moveSpeed",".000005");
	string ch = ini.GetValue("overseer","ceilingHeight",".05");
	string amb = ini.GetValue("overseer","ambient",".6");
	string dif = ini.GetValue("overseer","diffuse","1");
	string spec = ini.GetValue("overseer","specular",".8");

	char *end;
	moveSpeed = strtod(mov.c_str(),&end);
	mouseSensitivity = strtod(sens.c_str(),&end);
    ceilingHeight = strtod(ch.c_str(), &end);
    ambient = strtod(amb.c_str(), &end);
    diffuse = strtod(dif.c_str(), &end);
    specular = strtod(spec.c_str(), &end);

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
        return false;
    }

    Maps = DF->getMaps();
    Mats = DF->getMaterials();
    Cons = DF->getConstructions();
    Vegs = DF->getVegetation();
    if(!Maps->Start())
    {
        cerr << "Can't init map." << endl;
        return false;
    }
    if (!Mats->ReadInorganicMaterials() || !Mats->ReadOrganicMaterials())
    {
        cerr << "Can't init materials." << endl;
        return false;
    }

    cout << "Connected to Dwarf Fortress!" << endl;
    return true;
}

bool Overseer::go()
{
    loadSettings();
    if (!connectToDF())
    {
        cout << "DF is not running!";
        cin.ignore();
        return false;
    }

    dg = new DwarfGeometry(Maps, Mats, Cons, Vegs, root, startz, ceilingHeight, tristrip, doCulling, imageSize, useShaders, doImageScaling);
    dg->start();
    dg->drawGeometry();
    if (doVeggies) dg->drawVegetation();
    dg->drawSkybox();
    dg->clean();
    DF->Detach();

    osgViewer::Viewer viewer;
    viewer.setSceneData(root);
    if (!fullscreen) viewer.setUpViewInWindow(20, 20, 1044, 788);
    viewer.realize();

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
    light->setAmbient(Vec4(ambient,ambient,ambient,1));
    light->setDiffuse(Vec4(diffuse,diffuse,diffuse,1));
    light->setSpecular(Vec4(specular,specular,specular,1));
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

    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);
    DwarfEvents *de = new DwarfEvents(windows[0],c, root, moveSpeed, mouseSensitivity);
    viewer.addEventHandler(de);
    int dt = 0;
    if (useHeadlight) viewer.setLightingMode(osgViewer::Viewer::HEADLIGHT);
    else viewer.setLightingMode(osgViewer::Viewer::SKY_LIGHT);
    cout << "Have fun!" << endl;
    while (de->update(dt) && viewer.isRealized())
    {
        Timer_t startFrameTick = osg::Timer::instance()->tick();
        viewer.frame();
        Timer_t endFrameTick = osg::Timer::instance()->tick();
        dt = endFrameTick-startFrameTick;
    }
    return true;
}

#include <osgViewer/Viewer>
#include <osgFX/BumpMapping>
#include <DFHack.h>
#include <dfhack/DFTileTypes.h>
#include <iostream>

#include "DwarfGeometry.h"
#include "SimpleIni.h"
#include <sys/timeb.h>

using namespace std;
using namespace osg;

Group *root;

bool enableRamps;
bool fullscreen;
bool tristrip;
int startz;


int main(int argc, char **argv)
{
    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile("overseer.ini");

    enableRamps = ini.GetBoolValue("overseer","enableRamps",true);

	fullscreen = ini.GetBoolValue("overseer","fullscreen");

	tristrip = ini.GetBoolValue("overseer","tristripping");

	startz = ini.GetLongValue("overseer","zstart",0);

	int mouseSensitivity = ini.GetLongValue("overseer","mouseSensitivity",2347);

    osgViewer::Viewer viewer;
    root = new Group();
    viewer.setSceneData(root);
    osgFX::BumpMapping *bump;
    if (!enableRamps)
    {
        bump = new osgFX::BumpMapping();
        root->addChild(bump);
    }

    Group *geometryGroup = (enableRamps? root : bump);

    DFHack::ContextManager *DFMgr = new DFHack::ContextManager("Memory.xml");
	DFHack::Context *DF;

    try
    {
        DF = DFMgr->getSingleContext();
        DF->Attach();
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
        return 1;
    }

    DFHack::Maps *Maps = DF->getMaps();
    if(!Maps->Start())
    {
        cerr << "Can't init map." << endl;
        return 1;
    }
    cout << "Connected to Dwarf Fortress!" << endl;

    DwarfGeometry *dg = new DwarfGeometry(Maps, geometryGroup, startz, enableRamps, tristrip);
    dg->start();

    dg->drawGeometry();

	if (!enableRamps) bump->prepareChildren();

	Light *light = new Light();
	light->setLightNum(1);
	light->setPosition(Vec4(5,0,0,1));
	light->setDiffuse(Vec4(1,1,1,1));
	light->setSpecular(Vec4(1,1,1,1));
	light->setAmbient(Vec4(1,1,1,1));
	light->setConstantAttenuation(0.1);

	LightSource *ls = new LightSource();
	ls->setLight(light);
	ls->setLocalStateSetModes(StateAttribute::ON);
	ls->setStateSetModes(*(root->getOrCreateStateSet()),StateAttribute::ON);

    if (!fullscreen) viewer.setUpViewInWindow(20, 20, 1044, 788);
    viewer.realize();
//    viewer.setCameraManipulator(new DwarfManipulator(root, mouseSensitivity));
    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);
    //for (osgViewer::Viewer::Windows::iterator itr = windows.begin(); itr!=windows.end(); itr++) (*itr)->useCursor(false);
    DF->Detach();
    viewer.run();
}

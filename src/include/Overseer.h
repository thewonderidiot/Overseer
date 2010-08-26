#include <DFHack.h>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>
#include <OIS/OIS.h>
#include <iostream>
#include "DwarfGeometry.h"

class Overseer : public OIS::KeyListener, public OIS::MouseListener
{
    public:
        Overseer();
        ~Overseer();

        bool go();
    private:
        bool connectToDF();
        void loadSettings();
        virtual bool keyPressed(const OIS::KeyEvent &e);
        virtual bool keyReleased(const OIS::KeyEvent &e);
        virtual bool mouseMoved(const OIS::MouseEvent &e);
        virtual bool mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id);
        virtual bool mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id);
        OIS::Mouse *mouse;
        OIS::Keyboard *keyboard;
        OIS::InputManager *im;

        osg::Group *root;

        DFHack::ContextManager *DFMgr;
        DFHack::Context *DF;
        DFHack::Maps *Maps;
        DFHack::Materials *Mats;
        DFHack::Constructions *Cons;

        DwarfGeometry *dg;

        bool enableRamps;
        bool fullscreen;
        bool tristrip;
        int startz;
        int camz;
        double mouseSensitivity;
        double moveSpeed;

        bool keepRendering;
        std::string *fileName;

        osg::Vec3d velocity;
        osg::Matrixd rot;
        osg::Matrixd yaw;
        osg::Matrixd pitch;
        int shiftspeed;
};

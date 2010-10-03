#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIEventHandler>
#include <osgViewer/GraphicsWindow>
#include <osgDB/writeFile>

class DwarfEvents : public osgGA::GUIEventHandler
{
    public:
        DwarfEvents(osgViewer::GraphicsWindow *w, osg::Camera *cam, osg::Group *r, double movs, double ms, std::string *keys);
        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);
        virtual void accept(osgGA::GUIEventHandlerVisitor& v);
        bool update(int dt);
    private:
        osgViewer::GraphicsWindow *win;
        osg::Camera *c;
        osg::Group *root;
        osg::Vec3d eye;
        osg::Vec3d velocity;
        osg::Matrixd rot;
        osg::Matrixd yaw;
        osg::Matrixd pitch;
        int shiftspeed;
        double moveSpeed;
        double mouseSensitivity;
        bool keepRendering;
        bool grabMouse;
        std::string *fileName;
        std::string *keys;
};

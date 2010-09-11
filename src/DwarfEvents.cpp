#include "DwarfEvents.h"
#include <iostream>
using namespace std;
using namespace osg;
using namespace osgGA;


DwarfEvents::DwarfEvents(osgViewer::GraphicsWindow *w, Camera *cam, double movs, double ms)
{
    win = w;
    c = cam;
    moveSpeed = movs;
    mouseSensitivity = ms;
    Matrixd test = c->getViewMatrix();
    Quat qrot;
    Vec3d scale;
    Quat so;
    test.decompose(eye,qrot,scale,so);
    rot.makeRotate(qrot);
    yaw.makeRotate(0,Vec3(0,1,0));
    pitch.makeRotate(0,Vec3(0,0,1));
    velocity.set(0,0,0);
    shiftspeed = 1;
    keepRendering = true;
    grabMouse = false;
}

bool DwarfEvents::handle(const GUIEventAdapter& ea,GUIActionAdapter& aa)
{
    switch (ea.getEventType())
    {
    case (GUIEventAdapter::KEYDOWN):
        if (!grabMouse) break;
        switch (ea.getKey())
        {
        case GUIEventAdapter::KEY_Escape:
            keepRendering = false;
            break;
        case GUIEventAdapter::KEY_Shift_L:
        case GUIEventAdapter::KEY_Shift_R:
            shiftspeed = 10;
            break;
        case 'w':
        case 'W':
            velocity.set(velocity.x(),velocity.y(),velocity.z()<moveSpeed?velocity.z()+moveSpeed:velocity.z());
            break;
        case 's':
        case 'S':
            velocity.set(velocity.x(),velocity.y(),velocity.z()>-moveSpeed?velocity.z()-moveSpeed:velocity.z());
            break;
        case 'a':
        case 'A':
            velocity.set(velocity.x()<moveSpeed?velocity.x()+moveSpeed:velocity.x(),velocity.y(),velocity.z());
            break;
        case 'd':
        case 'D':
            velocity.set(velocity.x()>-moveSpeed?velocity.x()-moveSpeed:velocity.x(),velocity.y(),velocity.z());
            break;
        case 'r':
        case 'R':
            velocity.set(velocity.x(),velocity.y()>-moveSpeed?velocity.y()-moveSpeed:velocity.y(),velocity.z());
            break;
        case 'f':
        case 'F':
            velocity.set(velocity.x(),velocity.y()<moveSpeed?velocity.y()+moveSpeed:velocity.y(),velocity.z());
            break;
        default:
            break;
        }
        break;
    case (GUIEventAdapter::KEYUP):
        if (!grabMouse) break;
        switch (ea.getKey())
        {
        case GUIEventAdapter::KEY_Shift_L:
        case GUIEventAdapter::KEY_Shift_R:
            shiftspeed = 1;
            break;
        case 'w':
        case 'W':
            velocity.set(velocity.x(),velocity.y(),velocity.z()-moveSpeed);
            break;
        case 's':
        case 'S':
            velocity.set(velocity.x(),velocity.y(),velocity.z()+moveSpeed);
            break;
        case 'a':
        case 'A':
            velocity.set(velocity.x()-moveSpeed,velocity.y(),velocity.z());
            break;
        case 'd':
        case 'D':
            velocity.set(velocity.x()+moveSpeed,velocity.y(),velocity.z());
            break;
        case 'r':
        case 'R':
            velocity.set(velocity.x(),velocity.y()+moveSpeed,velocity.z());
            break;
        case 'f':
        case 'F':
            velocity.set(velocity.x(),velocity.y()-moveSpeed,velocity.z());
            break;
        default:
            break;
        }
        break;
    case GUIEventAdapter::PUSH:
        switch (ea.getButton())
        {
        case GUIEventAdapter::LEFT_MOUSE_BUTTON:
            if (!grabMouse) velocity.set(0,0,0);
            grabMouse = true;
            aa.requestWarpPointer((ea.getXmin()+ea.getXmax())/2.0f,(ea.getYmin()+ea.getYmax())/2.0f);
            win->setCursor(osgViewer::GraphicsWindow::NoCursor);
            break;
        case GUIEventAdapter::RIGHT_MOUSE_BUTTON:
            if (grabMouse) velocity.set(0,0,0);
            grabMouse = false;
            win->setCursor(osgViewer::GraphicsWindow::LeftArrowCursor);
            break;
        default:
            break;
        }
        break;
    case GUIEventAdapter::MOVE:
        if (grabMouse)
        {
            int x = ea.getX() - (ea.getXmin()+ea.getXmax())/2.0f;
            int y = ea.getY() - (ea.getYmin()+ea.getYmax())/2.0f;
            yaw *= Matrixd::rotate(inDegrees(mouseSensitivity*x), Vec3(0,1,0));
            pitch *= Matrixd::rotate(-inDegrees(mouseSensitivity*y), Vec3(1,0,0));
            aa.requestWarpPointer((ea.getXmin()+ea.getXmax())/2.0f,(ea.getYmin()+ea.getYmax())/2.0f);
        }
        break;
    default:
            break;
    }
    return true;
}


void DwarfEvents::accept(GUIEventHandlerVisitor& v)
{
    v.visit(*this);
}

bool DwarfEvents::update(int dt)
{
    eye += yaw*pitch*velocity*dt*shiftspeed;
    //light->setPosition(Vec4(eye.x(),eye.y(),eye.z(),50.0));
    c->setViewMatrix(rot*Matrixd::translate(eye)*yaw*pitch);
    return keepRendering;
}

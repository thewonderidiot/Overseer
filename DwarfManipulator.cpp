#include "DwarfManipulator.h"
#include <osg/Notify>
#include <iostream>

using namespace std;
using namespace osg;
using namespace osgGA;

DwarfManipulator::DwarfManipulator()
{
    _velocity.set(0,0,0);
    shiftspeed = 1;
    warped = 0;
    scroll = GUIEventAdapter::SCROLL_NONE;
}


DwarfManipulator::~DwarfManipulator()
{
}


void DwarfManipulator::setNode(osg::Node* node)
{
    _node = node;
    if (_node.get())
    {
        const osg::BoundingSphere& boundingSphere=_node->getBound();
        _modelScale = boundingSphere._radius;
    }

    if (getAutoComputeHomePosition()) computeHomePosition();
}


const osg::Node* DwarfManipulator::getNode() const
{
    return _node.get();
}



osg::Node* DwarfManipulator::getNode()
{
    return _node.get();
}


void DwarfManipulator::home(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    if (getAutoComputeHomePosition()) computeHomePosition();

    computePosition(_homeEye, _homeCenter, _homeUp);

    us.requestRedraw();
    us.requestContinuousUpdate(false);
    us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2.0f,(ea.getYmin()+ea.getYmax())/2.0f);

    flushMouseEventStack();
}


void DwarfManipulator::init(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    flushMouseEventStack();

    us.requestContinuousUpdate(false);

    if (ea.getEventType()!=GUIEventAdapter::RESIZE) us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2.0f,(ea.getYmin()+ea.getYmax())/2.0f);
}


bool DwarfManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    switch(ea.getEventType())
    {
        case(GUIEventAdapter::FRAME):
            addMouseEvent(ea);
            if (calcMovement()) us.requestRedraw();
            return false;

        case(GUIEventAdapter::RESIZE):
            init(ea,us);
            us.requestRedraw();
            return true;
        default:
            break;
    }

    if (ea.getHandled()) return false;

    switch(ea.getEventType())
    {
        case(GUIEventAdapter::PUSH):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();
            return true;
        }

        case(GUIEventAdapter::RELEASE):
        {

            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();
            return true;
        }

        case(GUIEventAdapter::DRAG):
        {
            addMouseEvent(ea);
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();
            return true;
        }

        case(GUIEventAdapter::MOVE):
        {
            GUIEventAdapter *cea = new GUIEventAdapter(ea);
            if (ea.getX()>ea.getXmax()-20)
            {
                us.requestWarpPointer(ea.getXmin()+20,ea.getY());
                cea->setX(ea.getXmin()+20);
                warped = 2;
            }
            if (ea.getX()<ea.getXmin()+20)
            {
                us.requestWarpPointer(ea.getXmax()-20,ea.getY());
                cea->setX(ea.getXmax()-20);
                warped = 2;
            }
            if (ea.getY()>ea.getYmax()-20)
            {
                us.requestWarpPointer(ea.getX(),ea.getYmin()+20);
                cea->setY(ea.getYmin()+20);
                warped = 2;
            }
            if (ea.getY()<ea.getYmin()+20)
            {
                us.requestWarpPointer(ea.getX(),ea.getYmax()-20);
                cea->setY(ea.getYmax()-20);
                warped = 2;
            }
            addMouseEvent(ea);
            _ga_c = cea;
            us.requestContinuousUpdate(true);
            if (calcMovement()) us.requestRedraw();
            return true;
        }

        case(GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()==GUIEventAdapter::KEY_Space)
            {
                flushMouseEventStack();
                home(ea,us);
                us.requestRedraw();
                us.requestContinuousUpdate(false);
                return true;
            }
            if (ea.getKey()=='w' || ea.getKey()=='W')
            {
                _velocity.set(_velocity.x(),_velocity.y(),-5);
                return true;
            }
            if (ea.getKey()=='s' || ea.getKey()=='S')
            {
                _velocity.set(_velocity.x(),_velocity.y(),5);
                return true;
            }
            if (ea.getKey()=='a' || ea.getKey()=='A')
            {
                _velocity.set(-5,_velocity.y(),_velocity.z());
                return true;
            }
            if (ea.getKey()=='d' || ea.getKey()=='D')
            {
                _velocity.set(5,_velocity.y(),_velocity.z());
                return true;
            }
            if (ea.getKey()==GUIEventAdapter::KEY_Shift_L || ea.getKey()==GUIEventAdapter::KEY_Shift_R)
            {
                shiftspeed = 10;
                return true;
            }
            return false;
        }

        case (GUIEventAdapter::KEYUP):
        {
            if (ea.getKey() == 'w' || ea.getKey() == 's' || ea.getKey() == 'W' || ea.getKey() == 'S')
            {
               _velocity.set(_velocity.x(),_velocity.y(),0);
               return true;
            }
            if (ea.getKey() == 'a' || ea.getKey() == 'd' || ea.getKey() == 'A' || ea.getKey() == 'D')
            {
               _velocity.set(0,_velocity.y(),_velocity.z());
               return true;
            }
            if (ea.getKey()==GUIEventAdapter::KEY_Shift_L || ea.getKey()==GUIEventAdapter::KEY_Shift_R)
            {
                shiftspeed = 1;
                return true;
            }
        }
        case (GUIEventAdapter::SCROLL):
        {
            scroll = ea.getScrollingMotion();
        }
        default:
            return false;
    }
}


void DwarfManipulator::flushMouseEventStack()
{
    _ga_t1 = NULL;
    _ga_t0 = NULL;
}


void DwarfManipulator::addMouseEvent(const GUIEventAdapter& ea)
{
    _ga_t1 = _ga_t0;
    _ga_t0 = &ea;
}


void DwarfManipulator::setByMatrix(const osg::Matrixd& matrix)
{
    _eye = matrix.getTrans();
    _rotation = matrix.getRotate();
    _distance = 1.0f;
}

osg::Matrixd DwarfManipulator::getMatrix() const
{
    return osg::Matrixd::rotate(_rotation)*osg::Matrixd::translate(_eye);
}

osg::Matrixd DwarfManipulator::getInverseMatrix() const
{
    return osg::Matrixd::translate(-_eye)*osg::Matrixd::rotate(_rotation.inverse());
}

void DwarfManipulator::computePosition(const osg::Vec3& eye,const osg::Vec3& center,const osg::Vec3& up)
{
    osg::Vec3d lv = center-eye;

    osg::Vec3 f(lv);
    f.normalize();
    osg::Vec3 s(f^up);
    s.normalize();
    osg::Vec3 u(s^f);
    u.normalize();

    osg::Matrixd rotation_matrix(s[0],     u[0],     -f[0],     0.0f,
                                s[1],     u[1],     -f[1],     0.0f,
                                s[2],     u[2],     -f[2],     0.0f,
                                0.0f,     0.0f,     0.0f,      1.0f);

    _eye = eye;
    _distance = lv.length();
    _rotation = rotation_matrix.getRotate().inverse();
}


bool DwarfManipulator::calcMovement()
{
    // return if less then two events have been added.
    if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;


    double dt = _ga_t0->getTime()-_ga_t1->getTime();
    unsigned int buttonMask = _ga_t1->getButtonMask();
    float dx=_ga_t0->getXnormalized()-_ga_t1->getXnormalized(),dy=_ga_t0->getYnormalized()-_ga_t1->getYnormalized();
    //if (buttonMask==GUIEventAdapter::LEFT_MOUSE_BUTTON)
    if (warped==2) warped--;
    else if (warped==1 && (dx>1 || dx<-1 || dy>1 || dy<-1))
    {
        dx = _ga_t0->getXnormalized()-_ga_c->getXnormalized();
        dy = _ga_t0->getYnormalized()-_ga_c->getYnormalized();

        warped--;
    }
    osg::CoordinateFrame cf=getCoordinateFrame(_eye);

    osg::Matrixd rotation_matrix;
    rotation_matrix.makeRotate(_rotation);
    double roll = 0;
    if (scroll == GUIEventAdapter::SCROLL_DOWN) roll = inDegrees(-1.0);
    if (scroll == GUIEventAdapter::SCROLL_UP) roll = inDegrees(1.0);
    scroll = GUIEventAdapter::SCROLL_NONE;
    double yaw = inDegrees(dx*5000*dt);
    double pitch = inDegrees(dy*5000*dt);
    osg::Quat yaw_rotate, pitch_rotate,roll_rotate;
    yaw_rotate.makeRotate(yaw,osg::Vec3(0,0,-1));
    pitch_rotate.makeRotate(pitch,osg::Vec3(1,0,0) * rotation_matrix);
    roll_rotate.makeRotate(roll,osg::Vec3(0,0,1)*rotation_matrix);
    _eye += _velocity*rotation_matrix*dt*shiftspeed;
    _rotation = _rotation*yaw_rotate*pitch_rotate*roll_rotate;

    return true;
}

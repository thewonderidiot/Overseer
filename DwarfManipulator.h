#include <osgGA/MatrixManipulator>
#include <osg/Quat>


class DwarfManipulator : public osgGA::MatrixManipulator
{
    public:
        DwarfManipulator();
        virtual ~DwarfManipulator();
        virtual void setByMatrix(const osg::Matrixd& matrix);
        virtual void setByInverseMatrix(const osg::Matrixd& matrix) { setByMatrix(osg::Matrixd::inverse(matrix)); }
        virtual osg::Matrixd getMatrix() const;
        virtual osg::Matrixd getInverseMatrix() const;
        virtual void home(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);
        virtual void init(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);
        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us);
        virtual void setNode(osg::Node*);
        virtual const osg::Node* getNode() const;
        virtual osg::Node* getNode();

    private:
        void flushMouseEventStack();
        void addMouseEvent(const osgGA::GUIEventAdapter& ea);
        void computePosition(const osg::Vec3& eye,const osg::Vec3& lv,const osg::Vec3& up);
        bool calcMovement();
        bool isMouseMoving();
        osg::ref_ptr<osg::Node> _node;
        // Internal event stack comprising last three mouse events.
        osg::ref_ptr<const osgGA::GUIEventAdapter> _ga_t1;
        osg::ref_ptr<const osgGA::GUIEventAdapter> _ga_t0;
        osg::ref_ptr<const osgGA::GUIEventAdapter> _ga_c;
        osg::Vec3 _center;
        osg::Quat _rotation;
        double _acceleration;
        osg::Vec3d _velocity;
        double _modelScale;
        osg::Vec3d  _eye;
        double      _distance;
        double shiftspeed;
        short warped;
        osgGA::GUIEventAdapter::ScrollingMotion scroll;
};

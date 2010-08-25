#include <DFHack.h>
#include <dfhack/DFTileTypes.h>
#include <osg/Drawable>
#include <osg/Geometry>
#include <osgGA/StateSetManipulator>
#include <osgUtil/TriStripVisitor>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osg/Material>
#include <osg/Texture2D>
#include <vector>

enum RampType
{
    NONE,
    NORTH,
    SOUTH,
    EAST,
    WEST,
    NW_UP,
    NW_DOWN,
    NE_UP,
    NE_DOWN,
    SW_UP,
    SW_DOWN,
    SE_UP,
    SE_DOWN,
    HILL,
    UNKNOWN
};

typedef struct
{
    int16_t tiletype;
    DFHack::t_designation designation;
    DFHack::t_occupancy occupancy;
    struct
    {
        uint16_t type;
        uint32_t index;
    } material;
    RampType ramptype;
    bool ceiling;
} Tile;

class DwarfGeometry
{
    public:
        DwarfGeometry();
        DwarfGeometry(DFHack::Maps *m, osg::Group *g, int sz, bool ts);
        bool drawGeometryOld();
        bool drawGeometry();
        bool start();
        int getGeometryMax();
    private:
        bool drawNorthWalls(uint32_t z);
        bool drawSouthWalls(uint32_t z);
        bool drawWestWalls(uint32_t z);
        bool drawEastWalls(uint32_t z);
        bool drawFloors(uint32_t z);
        bool drawCeilings(uint32_t z);
        bool drawRamps(uint32_t z);
        void drawNorthRampWestBoundaries(uint32_t x, uint32_t y, uint32_t z);
        void drawNorthRampEastBoundaries(uint32_t x, uint32_t y, uint32_t z);
        void drawNorthRampSouthBoundaries(uint32_t x, uint32_t y, uint32_t z);

        void drawSouthRampWestBoundaries(uint32_t x, uint32_t y, uint32_t z);
        void drawSouthRampEastBoundaries(uint32_t x, uint32_t y, uint32_t z);
        void drawSouthRampNorthBoundaries(uint32_t x, uint32_t y, uint32_t z);

        void drawWestRampNorthBoundaries(uint32_t x, uint32_t y, uint32_t z);
        void drawWestRampSouthBoundaries(uint32_t x, uint32_t y, uint32_t z);
        void drawWestRampEastBoundaries(uint32_t x, uint32_t y, uint32_t z);

        void drawEastRampNorthBoundaries(uint32_t x, uint32_t y, uint32_t z);
        void drawEastRampSouthBoundaries(uint32_t x, uint32_t y, uint32_t z);
        void drawEastRampWestBoundaries(uint32_t x, uint32_t y, uint32_t z);

        void drawCeilingBorders(uint32_t z);

        void processRamps();
        inline bool isCeiling(int16_t t, int16_t up)
        {
            return !DFHack::isWallTerrain(t) && (DFHack::isRampTerrain(up) || DFHack::isFloorTerrain(up) || DFHack::isWallTerrain(up));
        }
        bool tristrip;
        DFHack::Maps *Map;
        osg::ref_ptr<osg::Group> geometryGroup;
        int startz;
        osg::ref_ptr<osg::Geode> blockGeode;
        osg::ref_ptr<osg::Geometry> bg;
        osg::ref_ptr<osg::Vec3Array> vertices;
        osg::ref_ptr<osg::Vec3Array> normals;
        osg::ref_ptr<osg::Vec2Array> texcoords;
        osg::ref_ptr<osg::DrawElementsUInt> face;
        std::vector<std::vector<std::vector<Tile> > > tiles;
        uint32_t xmax,ymax,zmax;
        int geomax;
        float ceilingHeight;
};

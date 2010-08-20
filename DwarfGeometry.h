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
} Tile;

class DwarfGeometry
{
    public:
        DwarfGeometry();
        DwarfGeometry(DFHack::Maps *m, osg::Group *g, int sz, bool er, bool ts);
        bool drawGeometryOld();
        bool drawGeometry();
        bool start();
    private:
        bool drawNorthWalls(uint32_t z);
        bool drawSouthWalls(uint32_t z);
        bool drawWestWalls(uint32_t z);
        bool drawEastWalls(uint32_t z);
        bool drawFloors(uint32_t z);
        bool drawCeilings(uint32_t z);
        //void drawNorthRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *northblock, DFHack::mapblock40d *southblock, bool doNorthBoundary, bool doSouthBoundary);
        //void drawSouthRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *southblock, DFHack::mapblock40d *northblock, bool doSouthBoundary, bool doNorthBoundary);
        //void drawWestRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *westblock, DFHack::mapblock40d *eastblock, bool doWestBoundary, bool doEastBoundary);
        //void drawEastRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *eastblock, DFHack::mapblock40d *westblock, bool doEastBoundary, bool doWestBoundary);
        void processRamps();
        bool drawRamps(int x, int y, int z);
        void drawBlock(int x, int y, int z);
        bool enableRamps;
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
};

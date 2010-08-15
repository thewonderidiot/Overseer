class DwarfGeometry
{
    public:
        DwarfGeometry();
        DwarfGeometry(DFHack::Maps *m, osg::Group *g, int sz, bool er, bool ts);
        bool drawGeometry();
    private:
        bool drawNorthWalls(int y, int x, int z, DFHack::mapblock40d *block, DFHack::mapblock40d *northblock, bool doNorthBoundary);
        bool drawSouthWalls(int y, int x, int z, DFHack::mapblock40d *block, DFHack::mapblock40d *southblock, bool doSouthBoundary);
        bool drawWestWalls(int y, int x, int z, DFHack::mapblock40d *block, DFHack::mapblock40d *westblock, bool doWestBoundary);
        bool drawEastWalls(int y, int x, int z, DFHack::mapblock40d *block, DFHack::mapblock40d *eastblock, bool doEastBoundary);
        bool drawFloors(int y, int x, int z, DFHack::mapblock40d *block, DFHack::mapblock40d *downblock, bool doDownBoundary);
        void drawNorthRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *northblock, DFHack::mapblock40d *southblock, bool doNorthBoundary, bool doSouthBoundary);
        void drawSouthRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *southblock, DFHack::mapblock40d *northblock, bool doSouthBoundary, bool doNorthBoundary);
        void drawWestRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *westblock, DFHack::mapblock40d *eastblock, bool doWestBoundary, bool doEastBoundary);
        void drawEastRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *eastblock, DFHack::mapblock40d *westblock, bool doEastBoundary, bool doWestBoundary);
        bool drawRamps(int y, int x, int z, DFHack::mapblock40d *block, //these should be changed to accept an array, so blocks[][][] and exists[][][] need to be reformatted for z,y,z
                    DFHack::mapblock40d *northwestblock, DFHack::mapblock40d *northblock, DFHack::mapblock40d *northeastblock, DFHack::mapblock40d *westblock, DFHack::mapblock40d *eastblock,
                    DFHack::mapblock40d *southwestblock, DFHack::mapblock40d *southblock, DFHack::mapblock40d *southeastblock,
                    bool doNorthwestBoundary, bool doNorthBoundary, bool doNortheastBoundary, bool doWestBoundary, bool doEastBoundary, bool doSouthwestBoundary, bool doSouthBoundary, bool doSoutheastBoundary);
        bool enableRamps;
        bool tristrip;
        DFHack::Maps *Maps;
        osg::Group *geometryGroup;
        int startz;
        osg::Geode *blockGeode;
        osg::Geometry *bg;
        osg::Vec3Array *vertices;
        osg::Vec3Array *normals;
        osg::Vec2Array *texcoords;
        osg::DrawElementsUInt* face;
};

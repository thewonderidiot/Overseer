#include "DwarfGeometry.h"
using namespace std;
using namespace osg;

DwarfGeometry::DwarfGeometry()
{
}
DwarfGeometry::DwarfGeometry(DFHack::Maps *m, DFHack::Materials *mt, DFHack::Constructions *cns, osg::Group *g, int sz, bool ts)
{
    tristrip = ts;
    Map = m;
    geometryGroup = g;
    startz = sz;
    geomax = 0;
    ceilingHeight = 0.01;
    Mats = mt;
    Cons = cns;
}

void DwarfGeometry::processRamps()
{
    cout << "Determining ramp directions... ";
    for (uint32_t z = 0; z < zmax; z++)
    {
        for (uint32_t y = 0; y < ymax; y++)
        {
            for (uint32_t x = 0; x < xmax; x++)
            {
                if (DFHack::isRampTerrain(tiles[z][y][x].tiletype))
                {
                    if (x>0 && y>0 && DFHack::isRampTerrain(tiles[z][y][x-1].tiletype) && DFHack::isRampTerrain(tiles[z][y-1][x].tiletype))
                    {
                        if (DFHack::isWallTerrain(tiles[z][y-1][x-1].tiletype))
                        {
                            tiles[z][y][x].ramptype = NW_UP;
                            continue;
                        }
                        else if (x < xmax-1 && y < ymax-1 && DFHack::isWallTerrain(tiles[z][y][x+1].tiletype) && DFHack::isWallTerrain(tiles[z][y+1][x].tiletype))
                        {
                            tiles[z][y][x].ramptype = NW_DOWN;
                            continue;
                        }
                    }
                    if (x>0 && y<ymax-1 && DFHack::isRampTerrain(tiles[z][y][x-1].tiletype) && DFHack::isRampTerrain(tiles[z][y+1][x].tiletype))
                    {
                        if (DFHack::isWallTerrain(tiles[z][y+1][x-1].tiletype))
                        {
                            tiles[z][y][x].ramptype = NE_UP;
                            continue;
                        }
                        else if (x < xmax-1 && y > 0 && DFHack::isWallTerrain(tiles[z][y][x+1].tiletype) && DFHack::isWallTerrain(tiles[z][y-1][x].tiletype))
                        {
                            tiles[z][y][x].ramptype = NE_DOWN;
                            continue;
                        }
                    }
                    if (x<xmax-1 && y>0 && DFHack::isRampTerrain(tiles[z][y][x+1].tiletype) && DFHack::isRampTerrain(tiles[z][y-1][x].tiletype))
                    {
                        if (DFHack::isWallTerrain(tiles[z][y-1][x+1].tiletype))
                        {
                            tiles[z][y][x].ramptype = SW_UP;
                            continue;
                        }
                        else if (x > 0 && y < ymax-1 && DFHack::isWallTerrain(tiles[z][y][x-1].tiletype) && DFHack::isWallTerrain(tiles[z][y+1][x].tiletype))
                        {
                            tiles[z][y][x].ramptype = SW_DOWN;
                            continue;
                        }
                    }
                    if (x<xmax-1 && y<ymax-1 && DFHack::isRampTerrain(tiles[z][y][x+1].tiletype) && DFHack::isRampTerrain(tiles[z][y+1][x].tiletype))
                    {
                        if (DFHack::isWallTerrain(tiles[z][y+1][x+1].tiletype))
                        {
                            tiles[z][y][x].ramptype = SE_UP;
                            continue;
                        }
                        else if (x>0 && y>0 && DFHack::isWallTerrain(tiles[z][y][x-1].tiletype) && DFHack::isWallTerrain(tiles[z][y-1][x].tiletype))
                        {
                            tiles[z][y][x].ramptype = SE_DOWN;
                            continue;
                        }
                    }
                    if (((x>0 && DFHack::isWallTerrain(tiles[z][y][x-1].tiletype))||x==0) && ((x<xmax-1 && DFHack::isFloorTerrain(tiles[z][y][x+1].tiletype))||x==xmax-1))
                    {
                        tiles[z][y][x].ramptype = NORTH;
                        continue;
                    }
                    if (((x<xmax-1 && DFHack::isWallTerrain(tiles[z][y][x+1].tiletype))||x==xmax-1) && ((x>0 && DFHack::isFloorTerrain(tiles[z][y][x-1].tiletype))||x==0))
                    {
                        tiles[z][y][x].ramptype = SOUTH;
                        continue;
                    }
                    if (((y>0 && DFHack::isWallTerrain(tiles[z][y-1][x].tiletype))||y==0) && ((y<ymax-1 && DFHack::isFloorTerrain(tiles[z][y+1][x].tiletype))||y==ymax-1))
                    {
                        tiles[z][y][x].ramptype = WEST;
                        continue;
                    }
                    if (((y<ymax-1 && DFHack::isWallTerrain(tiles[z][y+1][x].tiletype))||y==ymax-1) && ((y>0 && DFHack::isFloorTerrain(tiles[z][y-1][x].tiletype))||y==0))
                    {
                        tiles[z][y][x].ramptype = EAST;
                        continue;
                    }
                    if ((x>0 && DFHack::isWallTerrain(tiles[z][y][x-1].tiletype))||x==0)
                    {
                        tiles[z][y][x].ramptype = NORTH;
                        continue;
                    }
                    if ((x<xmax-1 && DFHack::isWallTerrain(tiles[z][y][x+1].tiletype))||x==xmax-1)
                    {
                        tiles[z][y][x].ramptype = SOUTH;
                        continue;
                    }
                    if ((y>0 && DFHack::isWallTerrain(tiles[z][y-1][x].tiletype))||y==0)
                    {
                        tiles[z][y][x].ramptype = WEST;
                        continue;
                    }
                    if ((y<ymax-1 && DFHack::isWallTerrain(tiles[z][y+1][x].tiletype))||y==ymax-1)
                    {
                        tiles[z][y][x].ramptype = EAST;
                        continue;
                    }
                    tiles[z][y][x].ramptype = HILL;
                }
                else tiles[z][y][x].ramptype = NONE;
            }
        }
    }
    cout << "done." << endl;
}


bool DwarfGeometry::drawRamps(uint32_t z)
{
    uint32_t wallmat = 0;
    for (uint32_t x = 0; x < xmax; x++)
    {
        for (uint32_t y = 0; y < ymax; y++)
        {
            if (tiles[z][y][x].ramptype == NONE) continue;
            wallmat = tiles[z][y][x].material.index;
            if ((*vertices)[wallmat]==NULL) (*vertices)[wallmat] = new Vec3Array();
            if ((*normals)[wallmat]==NULL) (*normals)[wallmat] = new Vec3Array();
            if ((*texcoords)[wallmat]==NULL) (*texcoords)[wallmat] = new Vec2Array();
            if ((*bg)[wallmat]==NULL) (*bg)[wallmat] = new Geometry();
            int s;
            Vec3 norm;
            switch (tiles[z][y][x].ramptype)
            {
            case NORTH:
                (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                norm.set(1,0,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                drawNorthRampEastBoundaries(x,y,z,wallmat);
                drawNorthRampWestBoundaries(x,y,z,wallmat);
                drawNorthRampSouthBoundaries(x,y,z,wallmat);
                break;
            case SOUTH:
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                norm.set(-1,0,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                drawSouthRampWestBoundaries(x,y,z,wallmat);
                drawSouthRampEastBoundaries(x,y,z,wallmat);
                drawSouthRampNorthBoundaries(x,y,z,wallmat);
                break;
            case WEST:
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
                norm.set(0,1,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                drawWestRampNorthBoundaries(x,y,z,wallmat);
                drawWestRampSouthBoundaries(x,y,z,wallmat);
                drawWestRampEastBoundaries(x,y,z,wallmat);
                break;
            case EAST:
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                norm.set(0,-1,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                drawEastRampNorthBoundaries(x,y,z,wallmat);
                drawEastRampSouthBoundaries(x,y,z,wallmat);
                drawEastRampWestBoundaries(x,y,z,wallmat);
                break;
            case NW_UP:
                (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
                norm.set(1,0,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                norm.set(0,1,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                face->push_back(s-3);
                face->push_back(s-4);
                face->push_back(s-5);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                drawWestRampNorthBoundaries(x,y,z,wallmat);
                drawWestRampEastBoundaries(x,y,z,wallmat);
                drawNorthRampWestBoundaries(x,y,z,wallmat);
                drawNorthRampSouthBoundaries(x,y,z,wallmat);
                break;
            case NE_UP:
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                norm.set(0,-1,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                norm.set(1,0,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                face->push_back(s-3);
                face->push_back(s-4);
                face->push_back(s-5);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                drawNorthRampEastBoundaries(x,y,z,wallmat);
                drawNorthRampSouthBoundaries(x,y,z,wallmat);
                drawEastRampNorthBoundaries(x,y,z,wallmat);
                drawEastRampWestBoundaries(x,y,z,wallmat);
                break;
            case SW_UP:
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                norm.set(0,1,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                norm.set(-1,0,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                face->push_back(s-3);
                face->push_back(s-4);
                face->push_back(s-5);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                drawSouthRampWestBoundaries(x,y,z,wallmat);
                drawSouthRampNorthBoundaries(x,y,z,wallmat);
                drawWestRampSouthBoundaries(x,y,z,wallmat);
                drawWestRampEastBoundaries(x,y,z,wallmat);
                break;
            case SE_UP:
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                norm.set(-1,0,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                norm.set(0,-1,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                face->push_back(s-3);
                face->push_back(s-4);
                face->push_back(s-5);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                drawSouthRampEastBoundaries(x,y,z,wallmat);
                drawSouthRampNorthBoundaries(x,y,z,wallmat);
                drawEastRampSouthBoundaries(x,y,z,wallmat);
                drawEastRampWestBoundaries(x,y,z,wallmat);
                break;
            case NW_DOWN:
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                norm.set(0,-1,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                norm.set(-1,0,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                face->push_back(s-3);
                face->push_back(s-4);
                face->push_back(s-5);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                drawSouthRampWestBoundaries(x,y,z,wallmat);
                drawEastRampNorthBoundaries(x,y,z,wallmat);
                break;
            case NE_DOWN:
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                norm.set(-1,0,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                norm.set(0,1,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                face->push_back(s-3);
                face->push_back(s-4);
                face->push_back(s-5);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                drawWestRampNorthBoundaries(x,y,z,wallmat);
                drawSouthRampEastBoundaries(x,y,z,wallmat);
                break;
            case SW_DOWN:
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                norm.set(1,0,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                norm.set(0,-1,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                face->push_back(s-3);
                face->push_back(s-4);
                face->push_back(s-5);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                drawNorthRampWestBoundaries(x,y,z,wallmat);
                drawEastRampSouthBoundaries(x,y,z,wallmat);
                break;
            case SE_DOWN:
                (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
                norm.set(0,1,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                norm.set(1,0,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(0,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,1.414213562));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                face->push_back(s-3);
                face->push_back(s-4);
                face->push_back(s-5);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                drawWestRampSouthBoundaries(x,y,z,wallmat);
                drawNorthRampEastBoundaries(x,y,z,wallmat);
                break;
            case HILL:
                (*vertices)[wallmat]->push_back(Vec3(x+.5,y+.5,z+.5));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x+.5,y+.5,z+.5));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x+.5,y+.5,z+.5));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                (*vertices)[wallmat]->push_back(Vec3(x+.5,y+.5,z+.5));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
                norm.set(1,0,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                norm.set(0,-1,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                norm.set(-1,0,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                norm.set(0,1,1);
                norm.normalize();
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*normals)[wallmat]->push_back(norm);
                (*texcoords)[wallmat]->push_back(Vec2(.5,.7071067812));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                (*texcoords)[wallmat]->push_back(Vec2(.5,.7071067812));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                (*texcoords)[wallmat]->push_back(Vec2(.5,.7071067812));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                (*texcoords)[wallmat]->push_back(Vec2(.5,.7071067812));
                (*texcoords)[wallmat]->push_back(Vec2(1,0));
                (*texcoords)[wallmat]->push_back(Vec2(0,0));
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                face->push_back(s-3);
                face->push_back(s-4);
                face->push_back(s-5);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                s = (*vertices)[wallmat]->size()-1;
                face->push_back(s-6);
                face->push_back(s-7);
                face->push_back(s-8);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                face->push_back(s-9);
                face->push_back(s-10);
                face->push_back(s-11);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
                drawNorthRampSouthBoundaries(x,y,z,wallmat);
                drawSouthRampNorthBoundaries(x,y,z,wallmat);
                drawEastRampWestBoundaries(x,y,z,wallmat);
                drawWestRampEastBoundaries(x,y,z,wallmat);
                break;
            default:
                break;
            }

        }
    }
    return true;
}
void DwarfGeometry::drawNorthRampWestBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat)
{
    if (y > 0) //west cases
    {
        if (DFHack::isWallTerrain(tiles[z][y-1][x].tiletype) || tiles[z][y-1][x].ramptype==NW_DOWN || tiles[z][y-1][x].ramptype==SW_DOWN) //wall cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,1));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (DFHack::isFloorTerrain(tiles[z][y-1][x].tiletype) || DFHack::isOpenTerrain(tiles[z][y-1][x].tiletype) || tiles[z][y-1][x].ramptype==WEST || tiles[z][y-1][x].ramptype==NW_UP || tiles[z][y-1][x].ramptype==SW_UP || tiles[z][y-1][x].ramptype==HILL) //open cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
            (*vertices)[wallmat]->push_back(Vec3(x,y,z));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (tiles[z][y-1][x].ramptype==SOUTH || tiles[z][y-1][x].ramptype==SE_UP || tiles[z][y-1][x].ramptype==NE_DOWN) //intersect cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x+.5,y,z+.5));
            (*vertices)[wallmat]->push_back(Vec3(x,y,z));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(.5,.5));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
    }
}
void DwarfGeometry::drawNorthRampEastBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat)
{
    if (y < ymax-1)
    {
        if (DFHack::isWallTerrain(tiles[z][y+1][x].tiletype) || tiles[z][y+1][x].ramptype==NE_DOWN || tiles[z][y+1][x].ramptype==SE_DOWN) //wall cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,1));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (DFHack::isFloorTerrain(tiles[z][y+1][x].tiletype) || DFHack::isOpenTerrain(tiles[z][y+1][x].tiletype) || tiles[z][y+1][x].ramptype==EAST || tiles[z][y+1][x].ramptype==NE_UP || tiles[z][y+1][x].ramptype==SE_UP || tiles[z][y+1][x].ramptype==HILL) //open cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (tiles[z][y+1][x].ramptype==SOUTH || tiles[z][y+1][x].ramptype==SW_UP || tiles[z][y+1][x].ramptype==NW_DOWN) //intersect cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x+.5,y+1,z+.5));
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(.5,.5));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
    }
}
void DwarfGeometry::drawNorthRampSouthBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat)
{
    if (x < xmax-1 && DFHack::isWallTerrain(tiles[z][y][x+1].tiletype)) //opposite case
    {
        (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
        (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
        (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
        (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
        (*normals)[wallmat]->push_back(Vec3(-1,0,0));
        (*normals)[wallmat]->push_back(Vec3(-1,0,0));
        (*normals)[wallmat]->push_back(Vec3(-1,0,0));
        (*normals)[wallmat]->push_back(Vec3(-1,0,0));
        (*texcoords)[wallmat]->push_back(Vec2(0,1));
        (*texcoords)[wallmat]->push_back(Vec2(1,1));
        (*texcoords)[wallmat]->push_back(Vec2(1,0));
        (*texcoords)[wallmat]->push_back(Vec2(0,0));
        face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
        int s = (*vertices)[wallmat]->size()-1;
        face->push_back(s);
        face->push_back(s-1);
        face->push_back(s-2);
        face->push_back(s-3);
        (*bg)[wallmat]->addPrimitiveSet(face.get());
    }
}

void DwarfGeometry::drawSouthRampWestBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat)
{
    if (y > 0) //west cases
    {
        if (DFHack::isWallTerrain(tiles[z][y-1][x].tiletype) || tiles[z][y-1][x].ramptype==NW_DOWN || tiles[z][y-1][x].ramptype==SW_DOWN) //wall cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y,z));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,1));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (DFHack::isFloorTerrain(tiles[z][y-1][x].tiletype) || DFHack::isOpenTerrain(tiles[z][y-1][x].tiletype) || tiles[z][y-1][x].ramptype==WEST || tiles[z][y-1][x].ramptype==NW_UP || tiles[z][y-1][x].ramptype==SW_UP || tiles[z][y-1][x].ramptype==HILL) //open cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y,z));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (tiles[z][y-1][x].ramptype==NORTH || tiles[z][y-1][x].ramptype==NE_UP || tiles[z][y-1][x].ramptype==SE_DOWN) //intersect cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
            (*vertices)[wallmat]->push_back(Vec3(x+.5,y,z+.5));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(.5,.5));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
    }
}
void DwarfGeometry::drawSouthRampEastBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat)
{
    if (y < ymax-1) //east cases
    {
        if (DFHack::isWallTerrain(tiles[z][y+1][x].tiletype) || tiles[z][y+1][x].ramptype==NE_DOWN || tiles[z][y+1][x].ramptype==SE_DOWN) //wall cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*normals)[wallmat]->push_back(Vec3(0,-1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,1));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (DFHack::isFloorTerrain(tiles[z][y+1][x].tiletype) || DFHack::isOpenTerrain(tiles[z][y+1][x].tiletype) || tiles[z][y+1][x].ramptype==EAST || tiles[z][y+1][x].ramptype==NE_UP || tiles[z][y+1][x].ramptype==SE_UP || tiles[z][y+1][x].ramptype==HILL) //open cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (tiles[z][y+1][x].ramptype==NORTH || tiles[z][y+1][x].ramptype==NW_UP || tiles[z][y+1][x].ramptype==SW_DOWN) //intersect cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x+.5,y+1,z+.5));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*normals)[wallmat]->push_back(Vec3(0,1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(.5,.5));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
    }
}
void DwarfGeometry::drawSouthRampNorthBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat)
{
    if (x > 0 && DFHack::isWallTerrain(tiles[z][y][x-1].tiletype)) //opposite case
    {
        (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
        (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
        (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
        (*vertices)[wallmat]->push_back(Vec3(x,y,z));
        (*normals)[wallmat]->push_back(Vec3(1,0,0));
        (*normals)[wallmat]->push_back(Vec3(1,0,0));
        (*normals)[wallmat]->push_back(Vec3(1,0,0));
        (*normals)[wallmat]->push_back(Vec3(1,0,0));
        (*texcoords)[wallmat]->push_back(Vec2(0,1));
        (*texcoords)[wallmat]->push_back(Vec2(1,1));
        (*texcoords)[wallmat]->push_back(Vec2(1,0));
        (*texcoords)[wallmat]->push_back(Vec2(0,0));
        face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
        int s = (*vertices)[wallmat]->size()-1;
        face->push_back(s);
        face->push_back(s-1);
        face->push_back(s-2);
        face->push_back(s-3);
        (*bg)[wallmat]->addPrimitiveSet(face.get());
    }
}

void DwarfGeometry::drawWestRampNorthBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat)
{
    if (x > 0) //north cases
    {
        if (DFHack::isWallTerrain(tiles[z][y][x-1].tiletype) || tiles[z][y][x-1].ramptype==NE_DOWN || tiles[z][y][x-1].ramptype==NW_DOWN) //wall cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,1));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (DFHack::isFloorTerrain(tiles[z][y][x-1].tiletype) || DFHack::isOpenTerrain(tiles[z][y][x-1].tiletype) || tiles[z][y][x-1].ramptype==NORTH || tiles[z][y][x-1].ramptype==NW_UP || tiles[z][y][x-1].ramptype==NE_UP || tiles[z][y][x-1].ramptype==HILL) //open cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y,z));
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (tiles[z][y][x-1].ramptype==EAST || tiles[z][y][x-1].ramptype==SE_UP || tiles[z][y][x-1].ramptype==SW_DOWN) //intersect cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y,z));
            (*vertices)[wallmat]->push_back(Vec3(x,y+.5,z+.5));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(.5,.5));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
    }
}
void DwarfGeometry::drawWestRampSouthBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat)
{
    if (x < xmax-1) //south cases
    {
        if (DFHack::isWallTerrain(tiles[z][y][x+1].tiletype) || tiles[z][y][x+1].ramptype==SW_DOWN || tiles[z][y][x+1].ramptype==SE_DOWN) //wall cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,1));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (DFHack::isFloorTerrain(tiles[z][y][x+1].tiletype) || DFHack::isOpenTerrain(tiles[z][y][x+1].tiletype) || tiles[z][y][x+1].ramptype==SOUTH || tiles[z][y][x+1].ramptype==SW_UP || tiles[z][y][x+1].ramptype==SE_UP || tiles[z][y][x+1].ramptype==HILL) //open cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (tiles[z][y][x+1].ramptype==EAST || tiles[z][y][x+1].ramptype==NE_UP || tiles[z][y][x+1].ramptype==NW_DOWN) //intersect cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+.5,z+.5));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(.5,.5));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
    }
}
void DwarfGeometry::drawWestRampEastBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat)
{
    if (y < ymax-1 && DFHack::isWallTerrain(tiles[z][y+1][x].tiletype)) //opposite case
    {
        (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
        (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
        (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
        (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
        (*normals)[wallmat]->push_back(Vec3(0,-1,0));
        (*normals)[wallmat]->push_back(Vec3(0,-1,0));
        (*normals)[wallmat]->push_back(Vec3(0,-1,0));
        (*normals)[wallmat]->push_back(Vec3(0,-1,0));
        (*texcoords)[wallmat]->push_back(Vec2(0,1));
        (*texcoords)[wallmat]->push_back(Vec2(1,1));
        (*texcoords)[wallmat]->push_back(Vec2(1,0));
        (*texcoords)[wallmat]->push_back(Vec2(0,0));
        face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
        int s = (*vertices)[wallmat]->size()-1;
        face->push_back(s);
        face->push_back(s-1);
        face->push_back(s-2);
        face->push_back(s-3);
        (*bg)[wallmat]->addPrimitiveSet(face.get());
    }
}

void DwarfGeometry::drawEastRampNorthBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat)
{
    if (x > 0) //north cases
    {
        if (DFHack::isWallTerrain(tiles[z][y][x-1].tiletype) || tiles[z][y][x-1].ramptype==NW_DOWN || tiles[z][y][x-1].ramptype==NE_DOWN) //wall cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y,z));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,1));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (DFHack::isFloorTerrain(tiles[z][y][x-1].tiletype) || DFHack::isOpenTerrain(tiles[z][y][x-1].tiletype) || tiles[z][y][x-1].ramptype==NORTH || tiles[z][y][x-1].ramptype==NW_UP || tiles[z][y][x-1].ramptype==NE_UP || tiles[z][y][x-1].ramptype==HILL) //open cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
            (*vertices)[wallmat]->push_back(Vec3(x,y,z));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (tiles[z][y][x-1].ramptype==WEST || tiles[z][y][x-1].ramptype==SW_UP || tiles[z][y][x-1].ramptype==SE_DOWN) //intersect cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
            (*vertices)[wallmat]->push_back(Vec3(x,y+.5,z+.5));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(.5,.5));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
    }
}
void DwarfGeometry::drawEastRampSouthBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat)
{
    if (x < xmax-1) //south cases
    {
        if (DFHack::isWallTerrain(tiles[z][y][x+1].tiletype) || tiles[z][y][x+1].ramptype==SE_DOWN || tiles[z][y][x+1].ramptype==SW_DOWN) //wall cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*normals)[wallmat]->push_back(Vec3(-1,0,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,1));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (DFHack::isFloorTerrain(tiles[z][y][x+1].tiletype) || DFHack::isOpenTerrain(tiles[z][y][x+1].tiletype) || tiles[z][y][x+1].ramptype==SOUTH || tiles[z][y][x+1].ramptype==SW_UP || tiles[z][y][x+1].ramptype==SE_UP || tiles[z][y][x+1].ramptype==HILL) //open cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(1,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
        else if (tiles[z][y][x+1].ramptype==WEST || tiles[z][y][x+1].ramptype==NW_UP || tiles[z][y][x+1].ramptype==NE_DOWN) //intersect cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
            (*vertices)[wallmat]->push_back(Vec3(x+1,y+.5,z+.5));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*normals)[wallmat]->push_back(Vec3(1,0,0));
            (*texcoords)[wallmat]->push_back(Vec2(0,1));
            (*texcoords)[wallmat]->push_back(Vec2(.5,.5));
            (*texcoords)[wallmat]->push_back(Vec2(0,0));
            face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
            int s = (*vertices)[wallmat]->size()-1;
            face->push_back(s);
            face->push_back(s-1);
            face->push_back(s-2);
            (*bg)[wallmat]->addPrimitiveSet(face.get());
        }
    }
}
void DwarfGeometry::drawEastRampWestBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat)
{
    if (y > 0 && DFHack::isWallTerrain(tiles[z][y-1][x].tiletype)) //opposite case
    {
        (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
        (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
        (*vertices)[wallmat]->push_back(Vec3(x,y,z));
        (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
        (*normals)[wallmat]->push_back(Vec3(0,1,0));
        (*normals)[wallmat]->push_back(Vec3(0,1,0));
        (*normals)[wallmat]->push_back(Vec3(0,1,0));
        (*normals)[wallmat]->push_back(Vec3(0,1,0));
        (*texcoords)[wallmat]->push_back(Vec2(0,1));
        (*texcoords)[wallmat]->push_back(Vec2(1,1));
        (*texcoords)[wallmat]->push_back(Vec2(1,0));
        (*texcoords)[wallmat]->push_back(Vec2(0,0));
        face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
        int s = (*vertices)[wallmat]->size()-1;
        face->push_back(s);
        face->push_back(s-1);
        face->push_back(s-2);
        face->push_back(s-3);
        (*bg)[wallmat]->addPrimitiveSet(face.get());
    }
}

bool DwarfGeometry::drawNorthWalls(uint32_t z)
{
    bool wallStarted = false;
    uint32_t wallmat = 0;
    short length = 0;
    for (uint32_t x = 0; x < xmax; x++)
    {
        for (uint32_t y = 0; y < ymax; y++)
        {
            if (!(DFHack::isWallTerrain(tiles[z][y][x].tiletype) || DFHack::isRampTerrain(tiles[z][y][x].tiletype)) && ((x>0 && DFHack::isWallTerrain(tiles[z][y][x-1].tiletype)) || (x==0 && tiles[z][y][x].designation.bits.subterranean)))
            {
                uint32_t northmat = tiles[z][y][(x==0?x:x-1)].material.index;
                if (wallStarted && wallmat != northmat)
                {
                    wallStarted = false;
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                    (*normals)[wallmat]->push_back(Vec3(1,0,0));
                    (*normals)[wallmat]->push_back(Vec3(1,0,0));
                    (*texcoords)[wallmat]->push_back(Vec2(length,1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
                if (!wallStarted)
                {
                    wallStarted = true;
                    wallmat = northmat;
                    if ((*vertices)[wallmat]==NULL) (*vertices)[wallmat] = new Vec3Array();
                    if ((*normals)[wallmat]==NULL) (*normals)[wallmat] = new Vec3Array();
                    if ((*texcoords)[wallmat]==NULL) (*texcoords)[wallmat] = new Vec2Array();
                    if ((*bg)[wallmat]==NULL) (*bg)[wallmat] = new Geometry();
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                    (*normals)[wallmat]->push_back(Vec3(1,0,0));
                    (*normals)[wallmat]->push_back(Vec3(1,0,0));
                    (*texcoords)[wallmat]->push_back(Vec2(0,0));
                    (*texcoords)[wallmat]->push_back(Vec2(0,1));
                    length = 1;
                }
                else length++;
                if (y == ymax-1 && wallStarted)
                {
                    wallStarted = false;
                    (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                    (*normals)[wallmat]->push_back(Vec3(1,0,0));
                    (*normals)[wallmat]->push_back(Vec3(1,0,0));
                    (*texcoords)[wallmat]->push_back(Vec2(length,1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
            }
            else if (wallStarted)
            {
                wallStarted = false;
                (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                (*normals)[wallmat]->push_back(Vec3(1,0,0));
                (*normals)[wallmat]->push_back(Vec3(1,0,0));
                (*texcoords)[wallmat]->push_back(Vec2(length,1));
                (*texcoords)[wallmat]->push_back(Vec2(length,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
            }
        }
    }
    return true;
}

bool DwarfGeometry::drawSouthWalls(uint32_t z)
{
    bool wallStarted = false;
    uint32_t wallmat = 0;
    short length = 0;
    for (uint32_t x = 0; x < xmax; x++)
    {
        for (uint32_t y = 0; y < ymax; y++)
        {
            uint32_t southmat = tiles[z][y][(x==xmax-1?x:x+1)].material.index;
            if (!(DFHack::isWallTerrain(tiles[z][y][x].tiletype) || DFHack::isRampTerrain(tiles[z][y][x].tiletype)) && ((x<xmax-1 && DFHack::isWallTerrain(tiles[z][y][x+1].tiletype)) || (x==xmax-1 && tiles[z][y][x].designation.bits.subterranean)))
            {
                if (wallStarted && wallmat != southmat)
                {
                    wallStarted = false;
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                    (*normals)[wallmat]->push_back(Vec3(-1,0,0));
                    (*normals)[wallmat]->push_back(Vec3(-1,0,0));
                    (*texcoords)[wallmat]->push_back(Vec2(length,1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
                if (!wallStarted)
                {
                    wallStarted = true;
                    wallmat = southmat;
                    if ((*vertices)[wallmat]==NULL) (*vertices)[wallmat] = new Vec3Array();
                    if ((*normals)[wallmat]==NULL) (*normals)[wallmat] = new Vec3Array();
                    if ((*texcoords)[wallmat]==NULL) (*texcoords)[wallmat] = new Vec2Array();
                    if ((*bg)[wallmat]==NULL) (*bg)[wallmat] = new Geometry();
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
                    (*normals)[wallmat]->push_back(Vec3(-1,0,0));
                    (*normals)[wallmat]->push_back(Vec3(-1,0,0));
                    (*texcoords)[wallmat]->push_back(Vec2(0,0));
                    (*texcoords)[wallmat]->push_back(Vec2(0,1));
                    length = 1;
                }
                else length++;
                if (y == ymax-1 && wallStarted)
                {
                    wallStarted = false;
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
                    (*normals)[wallmat]->push_back(Vec3(-1,0,0));
                    (*normals)[wallmat]->push_back(Vec3(-1,0,0));
                    (*texcoords)[wallmat]->push_back(Vec2(length,1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
            }
            else if (wallStarted)
            {
                wallStarted = false;
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                (*normals)[wallmat]->push_back(Vec3(-1,0,0));
                (*normals)[wallmat]->push_back(Vec3(-1,0,0));
                (*texcoords)[wallmat]->push_back(Vec2(length,1));
                (*texcoords)[wallmat]->push_back(Vec2(length,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
            }
        }
    }
    return true;
}

bool DwarfGeometry::drawWestWalls(uint32_t z)
{
    bool wallStarted = false;
    uint32_t wallmat = 0;
    short length = 0;
    for (uint32_t y = 0; y < ymax; y++)
    {
        for (uint32_t x = 0; x < xmax; x++)
        {
            uint32_t westmat = tiles[z][(y==0?y:y-1)][x].material.index;
            if (!(DFHack::isWallTerrain(tiles[z][y][x].tiletype) || DFHack::isRampTerrain(tiles[z][y][x].tiletype)) && ((y>0 && DFHack::isWallTerrain(tiles[z][y-1][x].tiletype)) || (y==0 && tiles[z][y][x].designation.bits.subterranean)))
            {
                if (wallStarted && wallmat != westmat)
                {
                    wallStarted = false;
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                    (*normals)[wallmat]->push_back(Vec3(0,1,0));
                    (*normals)[wallmat]->push_back(Vec3(0,1,0));
                    (*texcoords)[wallmat]->push_back(Vec2(length,1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
                if (!wallStarted)
                {
                    wallStarted = true;
                    wallmat = westmat;
                    if ((*vertices)[wallmat]==NULL) (*vertices)[wallmat] = new Vec3Array();
                    if ((*normals)[wallmat]==NULL) (*normals)[wallmat] = new Vec3Array();
                    if ((*texcoords)[wallmat]==NULL) (*texcoords)[wallmat] = new Vec2Array();
                    if ((*bg)[wallmat]==NULL) (*bg)[wallmat] = new Geometry();
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                    (*normals)[wallmat]->push_back(Vec3(0,1,0));
                    (*normals)[wallmat]->push_back(Vec3(0,1,0));
                    (*texcoords)[wallmat]->push_back(Vec2(0,0));
                    (*texcoords)[wallmat]->push_back(Vec2(0,1));
                    length = 1;
                }
                else length++;
                if (x == xmax-1 && wallStarted)
                {
                    wallStarted = false;
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                    (*normals)[wallmat]->push_back(Vec3(0,1,0));
                    (*normals)[wallmat]->push_back(Vec3(0,1,0));
                    (*texcoords)[wallmat]->push_back(Vec2(length,1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
            }
            else if (wallStarted)
            {
                wallStarted = false;
                (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                (*normals)[wallmat]->push_back(Vec3(0,1,0));
                (*normals)[wallmat]->push_back(Vec3(0,1,0));
                (*texcoords)[wallmat]->push_back(Vec2(length,1));
                (*texcoords)[wallmat]->push_back(Vec2(length,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
            }
        }
    }
    return true;
}

bool DwarfGeometry::drawEastWalls(uint32_t z)
{
    bool wallStarted = false;
    uint32_t wallmat = 0;
    short length = 0;
    for (uint32_t y = 0; y < ymax; y++)
    {
        for (uint32_t x = 0; x < xmax; x++)
        {
            uint32_t eastmat = tiles[z][(y==ymax-1?y:y+1)][x].material.index;
            if (!(DFHack::isWallTerrain(tiles[z][y][x].tiletype) || DFHack::isRampTerrain(tiles[z][y][x].tiletype)) && ((y<ymax-1 && DFHack::isWallTerrain(tiles[z][y+1][x].tiletype)) || (y==ymax-1 && tiles[z][y][x].designation.bits.subterranean)))
            {
                if (wallStarted && wallmat != eastmat)
                {
                    wallStarted = false;
                    (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                    (*normals)[wallmat]->push_back(Vec3(0,-1,0));
                    (*normals)[wallmat]->push_back(Vec3(0,-1,0));
                    (*texcoords)[wallmat]->push_back(Vec2(length,1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
                if (!wallStarted)
                {
                    wallStarted = true;
                    wallmat = eastmat;
                    if ((*vertices)[wallmat]==NULL) (*vertices)[wallmat] = new Vec3Array();
                    if ((*normals)[wallmat]==NULL) (*normals)[wallmat] = new Vec3Array();
                    if ((*texcoords)[wallmat]==NULL) (*texcoords)[wallmat] = new Vec2Array();
                    if ((*bg)[wallmat]==NULL) (*bg)[wallmat] = new Geometry();
                    (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                    (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
                    (*normals)[wallmat]->push_back(Vec3(0,-1,0));
                    (*normals)[wallmat]->push_back(Vec3(0,-1,0));
                    (*texcoords)[wallmat]->push_back(Vec2(0,0));
                    (*texcoords)[wallmat]->push_back(Vec2(0,1));
                    length = 1;
                }
                else length++;
                if (x == xmax-1 && wallStarted)
                {
                    wallStarted = false;
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
                    (*normals)[wallmat]->push_back(Vec3(0,-1,0));
                    (*normals)[wallmat]->push_back(Vec3(0,-1,0));
                    (*texcoords)[wallmat]->push_back(Vec2(length,1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
            }
            else if (wallStarted)
            {
                wallStarted = false;
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
                (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                (*normals)[wallmat]->push_back(Vec3(0,-1,0));
                (*normals)[wallmat]->push_back(Vec3(0,-1,0));
                (*texcoords)[wallmat]->push_back(Vec2(length,1));
                (*texcoords)[wallmat]->push_back(Vec2(length,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
            }
        }
    }
    return true;
}

bool DwarfGeometry::drawFloors(uint32_t z)
{
    bool wallStarted = false;
    uint32_t wallmat = 0;
    short length = 0;
    for (uint32_t x = 0; x < xmax; x++)
    {
        for (uint32_t y = 0; y < ymax; y++)
        {
            if (DFHack::isFloorTerrain(tiles[z][y][x].tiletype))
            {
                if (wallStarted && wallmat != tiles[z][y][x].material.index)
                {
                    wallStarted = false;
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                    (*normals)[wallmat]->push_back(Vec3(0,0,1));
                    (*normals)[wallmat]->push_back(Vec3(0,0,1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
                if (!wallStarted)
                {
                    wallStarted = true;
                    wallmat = tiles[z][y][x].material.index;
                    if ((*vertices)[wallmat]==NULL) (*vertices)[wallmat] = new Vec3Array();
                    if ((*normals)[wallmat]==NULL) (*normals)[wallmat] = new Vec3Array();
                    if ((*texcoords)[wallmat]==NULL) (*texcoords)[wallmat] = new Vec2Array();
                    if ((*bg)[wallmat]==NULL) (*bg)[wallmat] = new Geometry();
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                    (*normals)[wallmat]->push_back(Vec3(0,0,1));
                    (*normals)[wallmat]->push_back(Vec3(0,0,1));
                    (*texcoords)[wallmat]->push_back(Vec2(0,0));
                    (*texcoords)[wallmat]->push_back(Vec2(0,1));
                    length = 1;
                }
                else length++;
                if (y == ymax-1 && wallStarted)
                {
                    wallStarted = false;
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
                    (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                    (*normals)[wallmat]->push_back(Vec3(0,0,1));
                    (*normals)[wallmat]->push_back(Vec3(0,0,1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
            }
            else if (wallStarted)
            {
                wallStarted = false;
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                (*normals)[wallmat]->push_back(Vec3(0,0,1));
                (*normals)[wallmat]->push_back(Vec3(0,0,1));
                (*texcoords)[wallmat]->push_back(Vec2(length,1));
                (*texcoords)[wallmat]->push_back(Vec2(length,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
            }
        }
    }
    return true;
}

bool DwarfGeometry::drawCeilings(uint32_t z)
{
    bool wallStarted = false;
    uint32_t wallmat = 0;
    short length = 0;
    for (uint32_t x = 0; x < xmax; x++)
    {
        for (uint32_t y = 0; y < ymax; y++)
        {
            if (z < zmax-1 && isCeiling(tiles[z][y][x].tiletype,tiles[z+1][y][x].tiletype))
            {
                tiles[z][y][x].ceiling = true;
                if (wallStarted && wallmat != tiles[z+1][y][x].material.index)
                {
                    wallStarted = false;
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1-ceilingHeight));
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z+1-ceilingHeight));
                    (*normals)[wallmat]->push_back(Vec3(0,0,-1));
                    (*normals)[wallmat]->push_back(Vec3(0,0,-1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
                if (!wallStarted)
                {
                    wallStarted = true;
                    wallmat = tiles[z+1][y][x].material.index;
                    if ((*vertices)[wallmat]==NULL) (*vertices)[wallmat] = new Vec3Array();
                    if ((*normals)[wallmat]==NULL) (*normals)[wallmat] = new Vec3Array();
                    if ((*texcoords)[wallmat]==NULL) (*texcoords)[wallmat] = new Vec2Array();
                    if ((*bg)[wallmat]==NULL) (*bg)[wallmat] = new Geometry();
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z+1-ceilingHeight));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1-ceilingHeight));
                    (*normals)[wallmat]->push_back(Vec3(0,0,-1));
                    (*normals)[wallmat]->push_back(Vec3(0,0,-1));
                    (*texcoords)[wallmat]->push_back(Vec2(0,0));
                    (*texcoords)[wallmat]->push_back(Vec2(0,1));
                    length = 1;
                }
                else length++;
                if (y == ymax-1 && wallStarted)
                {
                    wallStarted = false;
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1-ceilingHeight));
                    (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1-ceilingHeight));
                    (*normals)[wallmat]->push_back(Vec3(0,0,-1));
                    (*normals)[wallmat]->push_back(Vec3(0,0,-1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,1));
                    (*texcoords)[wallmat]->push_back(Vec2(length,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
            }
            else if (wallStarted)
            {
                wallStarted = false;
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1-ceilingHeight));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z+1-ceilingHeight));
                (*normals)[wallmat]->push_back(Vec3(0,0,-1));
                (*normals)[wallmat]->push_back(Vec3(0,0,-1));
                (*texcoords)[wallmat]->push_back(Vec2(length,1));
                (*texcoords)[wallmat]->push_back(Vec2(length,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = (*vertices)[wallmat]->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                (*bg)[wallmat]->addPrimitiveSet(face.get());
            }
        }
    }
    drawCeilingBorders(z);
    return true;
}

void DwarfGeometry::drawCeilingBorders(uint32_t z)
{
    uint32_t wallmat = 0;
    for (uint32_t x = 0; x < xmax; x++)
    {
        for (uint32_t y = 0; y < ymax; y++)
        {
            if (tiles[z][y][x].ceiling)
            {
                wallmat = tiles[z][y][x].material.index;
                if ((*vertices)[wallmat]==NULL) (*vertices)[wallmat] = new Vec3Array();
                if ((*normals)[wallmat]==NULL) (*normals)[wallmat] = new Vec3Array();
                if ((*texcoords)[wallmat]==NULL) (*texcoords)[wallmat] = new Vec2Array();
                if ((*bg)[wallmat]==NULL) (*bg)[wallmat] = new Geometry();

                if (x > 0 && !tiles[z][y][x-1].ceiling && !DFHack::isWallTerrain(tiles[z][y][x-1].tiletype))
                {
                    (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z+1-ceilingHeight));
                    (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1-ceilingHeight));
                    (*normals)[wallmat]->push_back(Vec3(-1,0,0));
                    (*normals)[wallmat]->push_back(Vec3(-1,0,0));
                    (*normals)[wallmat]->push_back(Vec3(-1,0,0));
                    (*normals)[wallmat]->push_back(Vec3(-1,0,0));
                    (*texcoords)[wallmat]->push_back(Vec2(0,1));
                    (*texcoords)[wallmat]->push_back(Vec2(1,1));
                    (*texcoords)[wallmat]->push_back(Vec2(1,1-ceilingHeight));
                    (*texcoords)[wallmat]->push_back(Vec2(0,1-ceilingHeight));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
                if (x < xmax-1 && !tiles[z][y][x+1].ceiling && !DFHack::isWallTerrain(tiles[z][y][x+1].tiletype))
                {
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1-ceilingHeight));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1-ceilingHeight));
                    (*normals)[wallmat]->push_back(Vec3(1,0,0));
                    (*normals)[wallmat]->push_back(Vec3(1,0,0));
                    (*normals)[wallmat]->push_back(Vec3(1,0,0));
                    (*normals)[wallmat]->push_back(Vec3(1,0,0));
                    (*texcoords)[wallmat]->push_back(Vec2(0,1));
                    (*texcoords)[wallmat]->push_back(Vec2(1,1));
                    (*texcoords)[wallmat]->push_back(Vec2(1,1-ceilingHeight));
                    (*texcoords)[wallmat]->push_back(Vec2(0,1-ceilingHeight));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
                if (y > 0 && !tiles[z][y-1][x].ceiling && !DFHack::isWallTerrain(tiles[z][y-1][x].tiletype))
                {
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1-ceilingHeight));
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z+1-ceilingHeight));
                    (*normals)[wallmat]->push_back(Vec3(0,-1,0));
                    (*normals)[wallmat]->push_back(Vec3(0,-1,0));
                    (*normals)[wallmat]->push_back(Vec3(0,-1,0));
                    (*normals)[wallmat]->push_back(Vec3(0,-1,0));
                    (*texcoords)[wallmat]->push_back(Vec2(0,1));
                    (*texcoords)[wallmat]->push_back(Vec2(1,1));
                    (*texcoords)[wallmat]->push_back(Vec2(1,1-ceilingHeight));
                    (*texcoords)[wallmat]->push_back(Vec2(0,1-ceilingHeight));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
                if (y < ymax-1 && !tiles[z][y+1][x].ceiling && !DFHack::isWallTerrain(tiles[z][y+1][x].tiletype))
                {
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1-ceilingHeight));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1-ceilingHeight));
                    (*normals)[wallmat]->push_back(Vec3(0,1,0));
                    (*normals)[wallmat]->push_back(Vec3(0,1,0));
                    (*normals)[wallmat]->push_back(Vec3(0,1,0));
                    (*normals)[wallmat]->push_back(Vec3(0,1,0));
                    (*texcoords)[wallmat]->push_back(Vec2(0,1));
                    (*texcoords)[wallmat]->push_back(Vec2(1,1));
                    (*texcoords)[wallmat]->push_back(Vec2(1,1-ceilingHeight));
                    (*texcoords)[wallmat]->push_back(Vec2(0,1-ceilingHeight));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = (*vertices)[wallmat]->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    (*bg)[wallmat]->addPrimitiveSet(face.get());
                }
            }
        }
    }
}

bool DwarfGeometry::start()
{
    vector<vector<uint16_t> > geology;
    if (!Map->ReadGeology(geology)) return false;
    DFHack::mapblock40d block;
    DFHack::biome_indices40d offsets;
    vector<DFHack::t_vein> veins;
    vector<DFHack::t_frozenliquidvein> ices;
    vector<DFHack::t_spattervein> splatter;
    vector<DFHack::t_feature> global_features;
    map<DFHack::planecoord, vector<DFHack::t_feature*> > local_features;
    Map->getSize(xmax,ymax,zmax);
    Map->ReadGlobalFeatures(global_features);
    Map->ReadLocalFeatures(local_features);
    tiles.resize(zmax);
    for (uint32_t z = 0; z < zmax; z++)
    {
        tiles[z].resize(16*ymax);
        for (uint32_t y = 0; y < 16*ymax; y++)
        {
            tiles[z][y].resize(16*xmax);
        }
    }
    bool hasgeo;
    cout << "Reading embark data...";
    for (uint32_t z=startz; z<zmax; z++)
    {
        hasgeo = false;
        for (uint32_t y=0; y<ymax; y++)
        {
            for (uint32_t x=0; x<xmax; x++)
            {
                if (Map->isValidBlock(x,y,z))
                {
                    Map->ReadBlock40d(x,y,z,&block);
                    Map->ReadRegionOffsets(x,y,z,&offsets);
                    veins.clear();
                    ices.clear();
                    splatter.clear();
                    Map->ReadVeins(x,y,z,&veins,&ices,&splatter);
                    for (int i=0; i<16; i++)
                    {
                        for (int j=0; j<16; j++)
                        {
                            int geolayer = block.designation[i][j].bits.geolayer_index;
                            int biome = block.designation[i][j].bits.biome;
                            tiles[z][16*y+j][16*x+i].material.type = DFHack::tileTypeTable[block.tiletypes[i][j]].m;
                            tiles[z][16*y+j][16*x+i].material.index = geology[offsets[biome]][geolayer];
                            tiles[z][16*y+j][16*x+i].tiletype = block.tiletypes[i][j];
                            tiles[z][16*y+j][16*x+i].occupancy = block.occupancy[i][j];
                            tiles[z][16*y+j][16*x+i].designation = block.designation[i][j];
                            tiles[z][16*y+j][16*x+i].ceiling = false;
                            for (uint32_t v = 0; v < veins.size(); v++)
                            {
                                if (veins[v].assignment[j] &(1<<i))
                                {
                                    tiles[z][16*y+j][16*x+i].material.type = DFHack::VEIN;
                                    tiles[z][16*y+j][16*x+i].material.index = veins[v].type;
                                }
                            }
                            if (block.global_feature != -1 && uint16_t(block.global_feature) < global_features.size() && block.designation[i][j].bits.feature_global && global_features[block.global_feature].main_material == 0)
                            {
                                tiles[z][16*y+j][16*x+i].material.type = DFHack::STONE;
                                tiles[z][16*y+j][16*x+i].material.index = global_features[block.global_feature].sub_material;
                            }
                            if (block.local_feature != -1)
                            {
                                DFHack::planecoord pc;
                                pc.dim.x = x;
                                pc.dim.y = y;
                                map<DFHack::planecoord, vector<DFHack::t_feature*> >::iterator it;
                                it = local_features.find(pc);
                                if (it != local_features.end())
                                {
                                    vector<DFHack::t_feature *>& vectr = (*it).second;
                                    if(uint16_t(block.local_feature) < vectr.size() && vectr[block.local_feature]->main_material != -1)
                                    {
                                        if (block.designation[i][j].bits.feature_local && vectr[block.local_feature]->main_material == 0)
                                        {
                                            tiles[z][16*y+j][16*x+i].material.type = DFHack::VEIN;
                                            tiles[z][16*y+j][16*x+i].material.index = vectr[block.local_feature]->sub_material;
                                        }
                                    }
                                }
                            }
                            switch (DFHack::tileTypeTable[block.tiletypes[i][j]].m)
                            {
                            case DFHack::GRASS:
                                tiles[z][16*y+j][16*x+i].material.index |= MAT_GRASS;
                                break;
                            case DFHack::GRASS2:
                                tiles[z][16*y+j][16*x+i].material.index |= MAT_GRASS2;
                                break;
                            case DFHack::GRASS_DEAD:
                                tiles[z][16*y+j][16*x+i].material.index |= MAT_GRASS_DEAD;
                                break;
                            case DFHack::GRASS_DRY:
                                tiles[z][16*y+j][16*x+i].material.index |= MAT_GRASS_DRY;
                                break;
                            case DFHack::MAGMA:
                                tiles[z][16*y+j][16*x+i].material.index |= MAT_MAGMA;
                                break;
                            case DFHack::ICE:
                                tiles[z][16*y+j][16*x+i].material.index |= MAT_ICE;
                                break;
                            case DFHack::OBSIDIAN:
                                tiles[z][16*y+j][16*x+i].material.index |= MAT_OBSIDIAN;
                                break;
                            default:
                                break;
                            }
                            if (!DFHack::isOpenTerrain(block.tiletypes[i][j])) hasgeo = true;
                        }
                    }
                }
                else
                {
                    for (int i=0; i<16; i++)
                    {
                        for (int j=0; j<16; j++)
                        {
                            tiles[z][16*y+j][16*x+i].tiletype = -1;
                            tiles[z][16*y+j][16*x+i].ceiling = false;
                        }
                    }
                }
            }
        }
        if (hasgeo == false && geomax == 0) geomax = z;
    }
    if (geomax == 0) geomax = zmax;
    xmax*=16;
    ymax*=16;
    uint32_t numConstr;
    Cons->Start(numConstr);
    for (uint32_t i=0; i<numConstr; i++)
    {
        DFHack::t_construction c;
        Cons->Read(i,c);
        tiles[c.z][c.y][c.x].material.type = c.mat_type;
        tiles[c.z][c.y][c.x].material.index = c.mat_idx;
    }
    Cons->Finish();
    cout << " done." << endl;
    processRamps();
    return true;
}

int DwarfGeometry::getGeometryMax()
{
    return geomax;
}

bool DwarfGeometry::drawGeometry()
{
    osgUtil::TriStripVisitor tsv(new osgUtil::Optimizer());
    for (uint32_t z = 0; z < zmax; z++)
    {
        cout << "Drawing z-level " << z << endl;
        ref_ptr<Group> zgroup = new Group();
        bg = new map<uint32_t, ref_ptr<Geometry> >();
        //blockGeode->addDrawable(bg.get());
        //geometryGroup->addChild(blockGeode.get());
        vertices = new map<uint32_t, ref_ptr<Vec3Array> >();
        normals = new map<uint32_t, ref_ptr<Vec3Array> >();
        texcoords = new map<uint32_t, ref_ptr<Vec2Array> >();
        drawNorthWalls(z);
        drawSouthWalls(z);
        drawWestWalls(z);
        drawEastWalls(z);
        drawFloors(z);
        drawCeilings(z);
        drawRamps(z);
        for (map<uint32_t, ref_ptr<Geometry> >::iterator it = bg->begin(); it != bg->end(); ++it)
        {
            blockGeode = new Geode();
            blockGeode->addDrawable((*bg)[it->first].get());
            (*bg)[it->first]->setVertexArray((*vertices)[it->first]);
            (*bg)[it->first]->setNormalArray((*normals)[it->first]);
            (*bg)[it->first]->setNormalBinding(Geometry::BIND_PER_VERTEX);
            zgroup->addChild(blockGeode.get());
            //cout << "Geode of " << Mats->inorganic[it->first].id << " created." << endl;
            string matstring;
            if (it->first&MAT_GRASS) matstring = "grass";
            else if (it->first&MAT_GRASS2) matstring = "grass2";
            else if (it->first&MAT_GRASS_DEAD) matstring = "grass_dead";
            else if (it->first&MAT_GRASS_DRY) matstring = "grass_dry";
            else if (it->first&MAT_MAGMA) matstring = "magma";
            else if (it->first&MAT_ICE) matstring = "ice";
            else if (it->first&MAT_OBSIDIAN) matstring = "obsidian";
            else matstring = Mats->inorganic[it->first].id;
            for (uint32_t i = 0; i < matstring.length(); i++)
            {
                if (matstring[i]==' ') matstring[i]='_';
                else matstring[i] = tolower(matstring[i]);
            }
            //cout << matstring << endl;
            ref_ptr<Image> wallimg = osgDB::readImageFile("materials/images/"+matstring+".bmp");
            if (wallimg == NULL) continue;
            (*bg)[it->first]->setTexCoordArray(0,(*texcoords)[it->first].get());
            ref_ptr<Texture2D> walltex = new Texture2D;
            walltex->setDataVariance(Object::DYNAMIC);
            walltex->setImage(wallimg.get());
            walltex->setWrap(Texture::WRAP_S,Texture::REPEAT);
            walltex->setWrap(Texture::WRAP_T,Texture::REPEAT);
            blockGeode->getOrCreateStateSet()->setTextureAttributeAndModes(0,walltex.get(),StateAttribute::ON);
        }
        geometryGroup->addChild(zgroup.get());
        //if (tristrip) tsv.stripify(*bg.get());
        //(*bg)[wallmat]->setVertexArray(vertices.get());
        //(*bg)[wallmat]->setNormalArray(normals.get());
        //(*bg)[wallmat]->setNormalBinding(Geometry::BIND_PER_VERTEX);
    }
    return true;
}




/*bool DwarfGeometry::drawGeometryOld()
{
    uint32_t xmax,ymax,zmax;
    Map->getSize(xmax,ymax,zmax);

    DFHack::mapblock40d blocks[3][3][3];
    bool exists[3][3][3] = {{{false,false,false},{false,false,false},{false,false,false}},{{false,false,false},{false,false,false},{false,false,false}},{{false,false,false},{false,false,false},{false,false,false}}};

    Texture2D *walltex = new Texture2D;
    walltex->setDataVariance(Object::DYNAMIC);
    Image *wallimg = osgDB::readImageFile("Wall.dds");
    walltex->setImage(wallimg);
    walltex->setWrap(Texture::WRAP_S,Texture::REPEAT);
    walltex->setWrap(Texture::WRAP_T,Texture::REPEAT);

    Texture2D *wallnmap = new Texture2D;
    wallnmap->setDataVariance(Object::DYNAMIC);
    Image *wallnimg = osgDB::readImageFile("WallN.dds");
    wallnmap->setImage(wallnimg);
    wallnmap->setWrap(Texture::WRAP_S,Texture::REPEAT);
    wallnmap->setWrap(Texture::WRAP_T,Texture::REPEAT);

    Texture2D *floortex = new Texture2D;
    floortex->setDataVariance(Object::DYNAMIC);
    Image *floorimg = osgDB::readImageFile("Floor.dds");
    floortex->setImage(floorimg);
    floortex->setWrap(Texture::WRAP_S,Texture::REPEAT);
    floortex->setWrap(Texture::WRAP_T,Texture::REPEAT);

    Texture2D *floornmap = new Texture2D;
    floornmap->setDataVariance(Object::DYNAMIC);
    Image *floornimg = osgDB::readImageFile("FloorN.dds");
    floornmap->setImage(floornimg);
    floornmap->setWrap(Texture::WRAP_S,Texture::REPEAT);
    floornmap->setWrap(Texture::WRAP_T,Texture::REPEAT);

    osgUtil::TriStripVisitor tri(new osgUtil::Optimizer());
                    blockGeode = new Geode();
    bg = new Geometry();
    blockGeode->addDrawable(bg);
    geometryGroup->addChild(blockGeode);
    vertices = new Vec3Array();
    normals = new Vec3Array();
    texcoords = new Vec2Array();
    for (uint32_t z = startz; z < zmax; z++)
    {
        cout << "Drawing z-level " << z << "..." << endl;
        for (uint32_t y = 0; y < ymax; y++)
        {
            for (uint32_t x = 0; x < xmax; x++)
            {
                if (!Map->isValidBlock(y,x,z)) continue;

                for (int k = 0; k <= 2; k++) //load our local 3x3 cube
                {
                    for (int i = 0; i <= 2; i++)
                    {
                        for (int j = 0; j <= 2; j++)
                        {
                            if (Map->isValidBlock(y+j-1,x+i-1,z+k-1))
                            {
                                Map->ReadBlock40d(y+j-1,x+i-1,z+k-1,&blocks[j][i][k]);
                                exists[j][i][k] = true;
                            }
                            else
                            {
                                exists[j][i][k] = false;
                            }
                        }
                    }
                }

                // Draw the geometry. x and y are passed in as tile coordinates so they are corrected by a factor of 16
                drawNorthWalls(y*16,x*16,z,&blocks[1][1][1],&blocks[1][0][1],exists[1][0][1]);
                drawSouthWalls(y*16,x*16,z,&blocks[1][1][1],&blocks[1][2][1],exists[1][2][1]);
                drawWestWalls(y*16,x*16,z,&blocks[1][1][1],&blocks[0][1][1],exists[0][1][1]);
				drawEastWalls(y*16,x*16,z,&blocks[1][1][1],&blocks[2][1][1],exists[2][1][1]);
                drawFloors(y*16,x*16,z,&blocks[1][1][1],&blocks[1][1][0],exists[1][1][0]);
                if (enableRamps) drawRamps(y*16,x*16,z,&blocks[1][1][1],&blocks[0][0][1],&blocks[1][0][1],&blocks[2][0][1],&blocks[0][1][1],&blocks[2][1][1],&blocks[0][2][1],&blocks[1][2][1],&blocks[2][2][1],exists[0][0][1],exists[1][0][1],exists[2][0][1],exists[0][1][1],exists[2][1][1],exists[0][2][1],exists[1][2][1],exists[2][2][1]);
                (*bg)[wallmat]->setVertexArray(vertices);
                (*bg)[wallmat]->setNormalArray(normals);
                if (!enableRamps)
                {
                	(*bg)[wallmat]->setTexCoordArray(0,texcoords);
                	(*bg)[wallmat]->setTexCoordArray(1,texcoords);
					blockGeode->getOrCreateStateSet()->setTextureAttributeAndModes(1,walltex,StateAttribute::ON);
					blockGeode->getOrCreateStateSet()->setTextureAttributeAndModes(0,wallnmap,StateAttribute::ON);
                }
                (*bg)[wallmat]->setNormalBinding(Geometry::BIND_PER_VERTEX);
				if (tristrip) tri.stripify(*bg);
				blockGeode = new Geode();
                bg = new Geometry();
                blockGeode->addDrawable(bg);
                geometryGroup->addChild(blockGeode);
                vertices = new Vec3Array();
                normals = new Vec3Array();
                texcoords = new Vec2Array();

				(*bg)[wallmat]->setVertexArray(vertices);
                (*bg)[wallmat]->setNormalArray(normals);
                if (!enableRamps)
                {
					(*bg)[wallmat]->setTexCoordArray(0,texcoords);
					(*bg)[wallmat]->setTexCoordArray(1,texcoords);
					blockGeode->getOrCreateStateSet()->setTextureAttributeAndModes(1,floortex,StateAttribute::ON);
					blockGeode->getOrCreateStateSet()->setTextureAttributeAndModes(0,floornmap,StateAttribute::ON);
                }
                (*bg)[wallmat]->setNormalBinding(Geometry::BIND_PER_VERTEX);
                if (tristrip) tri.stripify(*bg);
            }
        }
    }
    return true;
}*/

/*void DwarfGeometry::clean()
{

}*/

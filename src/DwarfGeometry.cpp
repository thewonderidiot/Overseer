#include "DwarfGeometry.h"
#include <osg/Node>
#include <osg/PositionAttitudeTransform>
#include <osg/TextureCubeMap>
#include <osg/TexEnv>
#include <osg/TexGen>
#include <osg/TexMat>
#include <osg/Depth>
#include <osg/Drawable>
#include <osg/ShapeDrawable>
#include <osgUtil/CullVisitor>

using namespace std;
using namespace osg;

class MoveEarthySkyWithEyePointTransform : public Transform
{
public:
    /** Get the transformation matrix which moves from local coords to world coords.*/
    virtual bool computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor* nv) const
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv)
        {
            Vec3 eyePointLocal = cv->getEyeLocal();
            matrix.preMultTranslate(eyePointLocal);
        }
        return true;
    }

    /** Get the transformation matrix which moves from world coords to local coords.*/
    virtual bool computeWorldToLocalMatrix(Matrix& matrix,NodeVisitor* nv) const
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv)
        {
            Vec3 eyePointLocal = cv->getEyeLocal();
            matrix.postMultTranslate(-eyePointLocal);
        }
        return true;
    }
};
struct TexMatCallback : public NodeCallback
{
public:

    TexMatCallback(TexMat& tm) :
        _texMat(tm)
    {
    }

    virtual void operator()(Node* node, NodeVisitor* nv)
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv)
        {
            const Matrix& MV = *(cv->getModelViewMatrix());
            const Matrix R = Matrix::rotate(DegreesToRadians(112.0f), 0.0f,0.0f,1.0f)*
                                  Matrix::rotate(DegreesToRadians(90.0f), 1.0f,0.0f,0.0f);

            Quat q = MV.getRotate();
            const Matrix C = Matrix::rotate( q.inverse() );

            _texMat.setMatrix( C*R );
        }

        traverse(node,nv);
    }

    TexMat& _texMat;
};


DwarfGeometry::DwarfGeometry()
{
}
DwarfGeometry::DwarfGeometry(DFHack::Maps *m, DFHack::Materials *mt, DFHack::Constructions *cns, DFHack::Vegetation *vgs, osg::Group *g, int sz, bool ts)
{
    tristrip = ts;
    Map = m;
    geometryGroup = g;
    startz = sz;
    geomax = 0;
    ceilingHeight = 0.01;
    Mats = mt;
    Cons = cns;
    Vegs = vgs;
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
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
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
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z));
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
                    (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
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
                (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
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

        /*ref_ptr<Node> tree = osgDB::readNodeFile("models/test.obj");
        PositionAttitudeTransform *treeplace = new PositionAttitudeTransform();
        treeplace->setPosition(Vec3(10,10,geomax));
        treeplace->addChild(tree.get());
        geometryGroup->addChild(treeplace);*/
    return true;
}

bool DwarfGeometry::drawVegetation()
{
    uint32_t numVegs = 0;
    Vegs->Start(numVegs);
    for (uint32_t i = 0; i < numVegs; i++)
    {
        DFHack::t_tree tree;
        Vegs->Read(i,tree);
        string matstring = Mats->organic[tree.material].id;
        for (uint32_t i = 0; i < matstring.length(); i++)
        {
            if (matstring[i]==' ') matstring[i]='_';
            else matstring[i] = tolower(matstring[i]);
        }
        switch (tree.type)
        {
        case 0:
        case 1:
            matstring += "_tree.ive";
            break;
        case 2:
        case 3:
            matstring += "_shrub.ive";
            break;
        default:
            break;
        }
        ref_ptr<Group> treenode = NULL;
        if (vegNodes[tree.material|(1<<(31-tree.type))] == NULL)
        {
            treenode = dynamic_cast<Group*>(osgDB::readNodeFile("models/"+matstring));
            if (treenode==NULL) continue;
            vegNodes[tree.material|(1<<(31-tree.type))] = treenode;
            cout << matstring << endl;
        }
        else treenode = vegNodes[tree.material|(1<<(31-tree.type))];
        //cout << "Acacia model drawn" << endl;
        ref_ptr<PositionAttitudeTransform> treeplace = new PositionAttitudeTransform();
        treeplace->setPosition(Vec3(tree.x,tree.y,tree.z));
        treeplace->setAttitude(Quat(inDegrees(-90.0),Vec3d(1,0,0)));
        treeplace->setScale(Vec3d(.5,.5,.5));
        treeplace->addChild(treenode.get());
        geometryGroup->addChild(treeplace.get());
    }
    Vegs->Finish();
    return true;
}

void DwarfGeometry::drawSkybox()
{
    TextureCubeMap *skybox = new TextureCubeMap;
    Image *imageUp = osgDB::readImageFile("materials/images/skybox/entropic_up.bmp");
    Image *imageDown = osgDB::readImageFile("materials/images/skybox/entropic_down.bmp");
    Image *imageNorth = osgDB::readImageFile("materials/images/skybox/entropic_west.bmp");
    Image *imageSouth = osgDB::readImageFile("materials/images/skybox/entropic_east.bmp");
    Image *imageWest = osgDB::readImageFile("materials/images/skybox/entropic_south.bmp");
    Image *imageEast = osgDB::readImageFile("materials/images/skybox/entropic_north.bmp");
    if (imageUp && imageDown && imageNorth && imageSouth && imageWest && imageEast)
    {
        skybox->setImage(TextureCubeMap::POSITIVE_X,imageNorth);
        skybox->setImage(TextureCubeMap::NEGATIVE_X,imageSouth);
        skybox->setImage(TextureCubeMap::POSITIVE_Y,imageDown);
        skybox->setImage(TextureCubeMap::NEGATIVE_Y,imageUp);
        skybox->setImage(TextureCubeMap::POSITIVE_Z,imageWest);
        skybox->setImage(TextureCubeMap::NEGATIVE_Z,imageEast);
        skybox->setWrap(Texture::WRAP_R, Texture::CLAMP_TO_EDGE);
        skybox->setWrap(Texture::WRAP_S, Texture::CLAMP_TO_EDGE);
        skybox->setWrap(Texture::WRAP_T, Texture::CLAMP_TO_EDGE);
        skybox->setFilter(Texture::MIN_FILTER, Texture::LINEAR_MIPMAP_LINEAR);
        skybox->setFilter(Texture::MAG_FILTER, Texture::LINEAR);
    }
    StateSet *ss = new osg::StateSet();
    TexEnv *te = new TexEnv;
    te->setMode(TexEnv::REPLACE);

    TexGen *tg = new TexGen;
    tg->setMode(TexGen::NORMAL_MAP);
    ss->setTextureAttributeAndModes(0, tg, StateAttribute::ON);

    TexMat *tm = new TexMat;
    ss->setTextureAttribute(0, tm);

    ss->setTextureAttributeAndModes(0, skybox, StateAttribute::ON);

    ss->setMode(GL_LIGHTING, StateAttribute::OFF);
    ss->setMode(GL_CULL_FACE, StateAttribute::OFF);

    Depth* depth = new Depth;
    depth->setFunction(Depth::ALWAYS);
    depth->setRange(1.0,1.0);
    ss->setAttributeAndModes(depth, StateAttribute::ON);

    ss->setRenderBinDetails(-1,"RenderBin");

    Drawable* drawable = new ShapeDrawable(new Sphere(Vec3(0.0f,0.0f,0.0f),1));

    Geode* geode = new osg::Geode;
    geode->setCullingActive(false);
    geode->setStateSet(ss);
    geode->addDrawable(drawable);

    Transform* transform = new MoveEarthySkyWithEyePointTransform;
    transform->setCullingActive(false);
    transform->addChild(geode);

    ClearNode* clearNode = new ClearNode;
//  clearNode->setRequiresClear(false);
    clearNode->setCullCallback(new TexMatCallback(*tm));
    clearNode->addChild(transform);
    geometryGroup->addChild(clearNode);
}





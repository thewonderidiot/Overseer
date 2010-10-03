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

#include <osg/Billboard>

#include "depends/tinyxml.h"
#include <iostream>

#include <osg/Program>
#include <osg/Shader>
#include <osg/Material>
#include <osgUtil/TangentSpaceGenerator>


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
DwarfGeometry::DwarfGeometry(DFHack::Maps *m, DFHack::Materials *mt, DFHack::Constructions *cns, DFHack::Vegetation *vgs, Group *g, int sz, double ch, bool ts, bool dc, int is, bool us, bool imgs)
{
    tristrip = ts;
    Map = m;
    geometryGroup = g;
    startz = sz;
    imageSize = is;
    geomax = 0;
    ceilingHeight = ch;
    doCulling = dc;
    Mats = mt;
    Cons = cns;
    Vegs = vgs;
    useShaders = us;
    doImageScaling = imgs;
    if (!loadColors()) cout << "Error loading colors. Make sure the colors directory is intact." << endl;
    if (doImageScaling)
    {
        ref_ptr<GraphicsContext::Traits> traits = new GraphicsContext::Traits;
        traits->x = 250;
        traits->y = 200;
        traits->width = 1;
        traits->height = 1;
        traits->windowDecoration = false;
        traits->doubleBuffer = false;
        traits->sharedContext = 0;

        gc = GraphicsContext::createGraphicsContext(traits.get());
        gc->realize();
        gc->makeCurrent();
    }
    cracktex = constex = 0;
    ref_ptr<Image> crackimg = osgDB::readImageFile("materials/images/rough.png");
    if (crackimg)
    {
        if (imageSize != 512 && doImageScaling) crackimg->scaleImage(imageSize,imageSize,1);
        cracktex = new Texture2D;
        cracktex->setDataVariance(Object::DYNAMIC);
        cracktex->setImage(crackimg.get());
        cracktex->setWrap(Texture::WRAP_S,Texture::REPEAT);
        cracktex->setWrap(Texture::WRAP_T,Texture::REPEAT);
        cracktex->setFilter(Texture::MIN_FILTER, Texture::NEAREST_MIPMAP_NEAREST);
        cracktex->setFilter(Texture::MAG_FILTER, Texture::NEAREST);
    }
    ref_ptr<Image> consimg = osgDB::readImageFile("materials/images/constructed.png");
    if (consimg)
    {
        if (imageSize != 512 && doImageScaling) consimg->scaleImage(imageSize,imageSize,1);
        constex = new Texture2D;
        constex->setDataVariance(Object::DYNAMIC);
        constex->setImage(consimg.get());
        constex->setWrap(Texture::WRAP_S,Texture::REPEAT);
        constex->setWrap(Texture::WRAP_T,Texture::REPEAT);
        constex->setFilter(Texture::MIN_FILTER, Texture::NEAREST_MIPMAP_NEAREST);
        constex->setFilter(Texture::MAG_FILTER, Texture::NEAREST);
    }
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
    uint16_t walltype = 0;
    for (uint32_t x = 0; x < xmax; x++)
    {
        for (uint32_t y = 0; y < ymax; y++)
        {
            if (tiles[z][y][x].ramptype == NONE) continue;
            walltype = tiles[z][y][x].material.type;
            wallmat = (walltype<<15) | tiles[z][y][x].material.index;
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
                drawNorthRampEastBoundaries(x,y,z,wallmat,walltype);
                drawNorthRampWestBoundaries(x,y,z,wallmat,walltype);
                drawNorthRampSouthBoundaries(x,y,z,wallmat,walltype);
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
                drawSouthRampWestBoundaries(x,y,z,wallmat,walltype);
                drawSouthRampEastBoundaries(x,y,z,wallmat,walltype);
                drawSouthRampNorthBoundaries(x,y,z,wallmat,walltype);
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
                drawWestRampNorthBoundaries(x,y,z,wallmat,walltype);
                drawWestRampSouthBoundaries(x,y,z,wallmat,walltype);
                drawWestRampEastBoundaries(x,y,z,wallmat,walltype);
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
                drawEastRampNorthBoundaries(x,y,z,wallmat,walltype);
                drawEastRampSouthBoundaries(x,y,z,wallmat,walltype);
                drawEastRampWestBoundaries(x,y,z,wallmat,walltype);
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
                drawWestRampNorthBoundaries(x,y,z,wallmat,walltype);
                drawWestRampEastBoundaries(x,y,z,wallmat,walltype);
                drawNorthRampWestBoundaries(x,y,z,wallmat,walltype);
                drawNorthRampSouthBoundaries(x,y,z,wallmat,walltype);
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
                drawNorthRampEastBoundaries(x,y,z,wallmat,walltype);
                drawNorthRampSouthBoundaries(x,y,z,wallmat,walltype);
                drawEastRampNorthBoundaries(x,y,z,wallmat,walltype);
                drawEastRampWestBoundaries(x,y,z,wallmat,walltype);
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
                drawSouthRampWestBoundaries(x,y,z,wallmat,walltype);
                drawSouthRampNorthBoundaries(x,y,z,wallmat,walltype);
                drawWestRampSouthBoundaries(x,y,z,wallmat,walltype);
                drawWestRampEastBoundaries(x,y,z,wallmat,walltype);
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
                drawSouthRampEastBoundaries(x,y,z,wallmat,walltype);
                drawSouthRampNorthBoundaries(x,y,z,wallmat,walltype);
                drawEastRampSouthBoundaries(x,y,z,wallmat,walltype);
                drawEastRampWestBoundaries(x,y,z,wallmat,walltype);
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
                drawSouthRampWestBoundaries(x,y,z,wallmat,walltype);
                drawEastRampNorthBoundaries(x,y,z,wallmat,walltype);
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
                drawWestRampNorthBoundaries(x,y,z,wallmat,walltype);
                drawSouthRampEastBoundaries(x,y,z,wallmat,walltype);
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
                drawNorthRampWestBoundaries(x,y,z,wallmat,walltype);
                drawEastRampSouthBoundaries(x,y,z,wallmat,walltype);
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
                drawWestRampSouthBoundaries(x,y,z,wallmat,walltype);
                drawNorthRampEastBoundaries(x,y,z,wallmat,walltype);
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
                drawNorthRampSouthBoundaries(x,y,z,wallmat,walltype);
                drawSouthRampNorthBoundaries(x,y,z,wallmat,walltype);
                drawEastRampWestBoundaries(x,y,z,wallmat,walltype);
                drawWestRampEastBoundaries(x,y,z,wallmat,walltype);
                break;
            default:
                break;
            }

        }
    }
    return true;
}
void DwarfGeometry::drawNorthRampWestBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat, uint16_t walltype)
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
        else if (DFHack::isFloorTerrain(tiles[z][y-1][x].tiletype) || isRampTopOrOpenTerrain(tiles[z][y-1][x].tiletype) || tiles[z][y-1][x].ramptype==WEST || tiles[z][y-1][x].ramptype==NW_UP || tiles[z][y-1][x].ramptype==SW_UP || tiles[z][y-1][x].ramptype==HILL) //open cases
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
void DwarfGeometry::drawNorthRampEastBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat, uint16_t walltype)
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
        else if (DFHack::isFloorTerrain(tiles[z][y+1][x].tiletype) || isRampTopOrOpenTerrain(tiles[z][y+1][x].tiletype) || tiles[z][y+1][x].ramptype==EAST || tiles[z][y+1][x].ramptype==NE_UP || tiles[z][y+1][x].ramptype==SE_UP || tiles[z][y+1][x].ramptype==HILL) //open cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
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
        else if (tiles[z][y+1][x].ramptype==SOUTH || tiles[z][y+1][x].ramptype==SW_UP || tiles[z][y+1][x].ramptype==NW_DOWN) //intersect cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
            (*vertices)[wallmat]->push_back(Vec3(x+.5,y+1,z+.5));
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
void DwarfGeometry::drawNorthRampSouthBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat, uint16_t walltype)
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

void DwarfGeometry::drawSouthRampWestBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat, uint16_t walltype)
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
        else if (DFHack::isFloorTerrain(tiles[z][y-1][x].tiletype) || isRampTopOrOpenTerrain(tiles[z][y-1][x].tiletype) || tiles[z][y-1][x].ramptype==WEST || tiles[z][y-1][x].ramptype==NW_UP || tiles[z][y-1][x].ramptype==SW_UP || tiles[z][y-1][x].ramptype==HILL) //open cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
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
void DwarfGeometry::drawSouthRampEastBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat, uint16_t walltype)
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
        else if (DFHack::isFloorTerrain(tiles[z][y+1][x].tiletype) || isRampTopOrOpenTerrain(tiles[z][y+1][x].tiletype) || tiles[z][y+1][x].ramptype==EAST || tiles[z][y+1][x].ramptype==NE_UP || tiles[z][y+1][x].ramptype==SE_UP || tiles[z][y+1][x].ramptype==HILL) //open cases
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
void DwarfGeometry::drawSouthRampNorthBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat, uint16_t walltype)
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

void DwarfGeometry::drawWestRampNorthBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat, uint16_t walltype)
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
        else if (DFHack::isFloorTerrain(tiles[z][y][x-1].tiletype) || isRampTopOrOpenTerrain(tiles[z][y][x-1].tiletype) || tiles[z][y][x-1].ramptype==NORTH || tiles[z][y][x-1].ramptype==NW_UP || tiles[z][y][x-1].ramptype==NE_UP || tiles[z][y][x-1].ramptype==HILL) //open cases
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
void DwarfGeometry::drawWestRampSouthBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat, uint16_t walltype)
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
        else if (DFHack::isFloorTerrain(tiles[z][y][x+1].tiletype) || isRampTopOrOpenTerrain(tiles[z][y][x+1].tiletype) || tiles[z][y][x+1].ramptype==SOUTH || tiles[z][y][x+1].ramptype==SW_UP || tiles[z][y][x+1].ramptype==SE_UP || tiles[z][y][x+1].ramptype==HILL) //open cases
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
void DwarfGeometry::drawWestRampEastBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat, uint16_t walltype)
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

void DwarfGeometry::drawEastRampNorthBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat, uint16_t walltype)
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
        else if (DFHack::isFloorTerrain(tiles[z][y][x-1].tiletype) || isRampTopOrOpenTerrain(tiles[z][y][x-1].tiletype) || tiles[z][y][x-1].ramptype==NORTH || tiles[z][y][x-1].ramptype==NW_UP || tiles[z][y][x-1].ramptype==NE_UP || tiles[z][y][x-1].ramptype==HILL) //open cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
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
        else if (tiles[z][y][x-1].ramptype==WEST || tiles[z][y][x-1].ramptype==SW_UP || tiles[z][y][x-1].ramptype==SE_DOWN) //intersect cases
        {
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z+1));
            (*vertices)[wallmat]->push_back(Vec3(x,y+.5,z+.5));
            (*vertices)[wallmat]->push_back(Vec3(x,y+1,z));
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
void DwarfGeometry::drawEastRampSouthBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat, uint16_t walltype)
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
        else if (DFHack::isFloorTerrain(tiles[z][y][x+1].tiletype) || isRampTopOrOpenTerrain(tiles[z][y][x+1].tiletype) || tiles[z][y][x+1].ramptype==SOUTH || tiles[z][y][x+1].ramptype==SW_UP || tiles[z][y][x+1].ramptype==SE_UP || tiles[z][y][x+1].ramptype==HILL) //open cases
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
void DwarfGeometry::drawEastRampWestBoundaries(uint32_t x, uint32_t y, uint32_t z, uint32_t wallmat, uint16_t walltype)
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
    uint16_t walltype = 0;
    short length = 0;
    for (uint32_t x = 0; x < xmax; x++)
    {
        for (uint32_t y = 0; y < ymax; y++)
        {
            if (!(DFHack::isWallTerrain(tiles[z][y][x].tiletype) || DFHack::isRampTerrain(tiles[z][y][x].tiletype)) && ((x>0 && DFHack::isWallTerrain(tiles[z][y][x-1].tiletype)) || (x==0 && tiles[z][y][x].designation.bits.subterranean)))
            {
                uint16_t northtype = tiles[z][y][x==0?x:x-1].material.type;
                uint32_t northmat = (northtype<<15) | tiles[z][y][(x==0?x:x-1)].material.index;
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
                    walltype = northtype;
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
    uint16_t walltype = 0;

    short length = 0;
    for (uint32_t x = 0; x < xmax; x++)
    {
        for (uint32_t y = 0; y < ymax; y++)
        {
            uint16_t southtype = tiles[z][y][x==xmax-1?x:x+1].material.type;
            uint32_t southmat = (southtype<<15) | tiles[z][y][(x==xmax-1?x:x+1)].material.index;
            if (!(DFHack::isWallTerrain(tiles[z][y][x].tiletype) || DFHack::isRampTerrain(tiles[z][y][x].tiletype)) && ((x<xmax-1 && DFHack::isWallTerrain(tiles[z][y][x+1].tiletype)) || (x==xmax-1 && tiles[z][y][x].designation.bits.subterranean)))
            {
                if (wallStarted && wallmat != southmat)
                {
                    wallStarted = false;
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
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
                    walltype = southtype;
                    if ((*vertices)[wallmat]==NULL) (*vertices)[wallmat] = new Vec3Array();
                    if ((*normals)[wallmat]==NULL) (*normals)[wallmat] = new Vec3Array();
                    if ((*texcoords)[wallmat]==NULL) (*texcoords)[wallmat] = new Vec2Array();
                    if ((*bg)[wallmat]==NULL) (*bg)[wallmat] = new Geometry();
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
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
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y+1,z+1));
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
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
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
    uint16_t walltype = 0;

    short length = 0;
    for (uint32_t y = 0; y < ymax; y++)
    {
        for (uint32_t x = 0; x < xmax; x++)
        {
            uint16_t westtype = tiles[z][y==0?y:y-1][x].material.type;
            uint32_t westmat = (westtype<<15) |  tiles[z][(y==0?y:y-1)][x].material.index;
            if (!(DFHack::isWallTerrain(tiles[z][y][x].tiletype) || DFHack::isRampTerrain(tiles[z][y][x].tiletype)) && ((y>0 && DFHack::isWallTerrain(tiles[z][y-1][x].tiletype)) || (y==0 && tiles[z][y][x].designation.bits.subterranean)))
            {
                if (wallStarted && wallmat != westmat)
                {
                    wallStarted = false;
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
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
                    walltype = westtype;
                    if ((*vertices)[wallmat]==NULL) (*vertices)[wallmat] = new Vec3Array();
                    if ((*normals)[wallmat]==NULL) (*normals)[wallmat] = new Vec3Array();
                    if ((*texcoords)[wallmat]==NULL) (*texcoords)[wallmat] = new Vec2Array();
                    if ((*bg)[wallmat]==NULL) (*bg)[wallmat] = new Geometry();
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
                    (*vertices)[wallmat]->push_back(Vec3(x,y,z));
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
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z));
                    (*vertices)[wallmat]->push_back(Vec3(x+1,y,z+1));
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
                (*vertices)[wallmat]->push_back(Vec3(x,y,z));
                (*vertices)[wallmat]->push_back(Vec3(x,y,z+1));
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
    uint16_t walltype = 0;

    short length = 0;
    for (uint32_t y = 0; y < ymax; y++)
    {
        for (uint32_t x = 0; x < xmax; x++)
        {
            uint16_t easttype = tiles[z][y==ymax-1?y:y+1][x].material.type;
            uint32_t eastmat = (easttype<<15) | tiles[z][y==ymax-1?y:y+1][x].material.index;
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
                    walltype = easttype;
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
    uint16_t walltype = 0;

    short length = 0;
    for (uint32_t x = 0; x < xmax; x++)
    {
        for (uint32_t y = 0; y < ymax; y++)
        {
            uint16_t downtype = tiles[z][y][x].material.type;
            uint32_t downmat = (downtype<<15) | tiles[z][y][x].material.index;
            if (DFHack::isFloorTerrain(tiles[z][y][x].tiletype))
            {
                if (wallStarted && wallmat != downmat)
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
                    walltype = downtype;
                    wallmat = downmat;
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
    uint16_t walltype = 0;

    short length = 0;
    for (uint32_t x = 0; x < xmax; x++)
    {
        for (uint32_t y = 0; y < ymax; y++)
        {
            if (z < zmax-1 && isCeiling(tiles[z][y][x].tiletype,tiles[z+1][y][x].tiletype))
            {
                uint16_t uptype = tiles[z+1][y][x].material.type;
                uint32_t upmat = (uptype<<15) | tiles[z+1][y][x].material.index;
                tiles[z][y][x].ceiling = true;
                if (wallStarted && wallmat != upmat)
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
                    wallmat = upmat;
                    walltype = uptype;
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
    if (z==zmax) return;
    uint32_t wallmat = 0;
    uint16_t walltype = 0;

    for (uint32_t x = 0; x < xmax; x++)
    {
        for (uint32_t y = 0; y < ymax; y++)
        {
            if (tiles[z][y][x].ceiling)
            {
                walltype = tiles[z+1][y][x].material.type;
                wallmat = (walltype<<15) | tiles[z+1][y][x].material.index;
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
        tiles[z].resize(16*xmax);
        for (uint32_t x = 0; x < 16*xmax; x++)
        {
            tiles[z][x].resize(16*ymax);
        }
    }
    bool hasgeo;
    cout << "Reading embark data:";
    for (uint32_t z=startz; z<zmax; z++)
    {
        hasgeo = false;
        for (uint32_t x=0; x<xmax; x++)
        {
            for (uint32_t y=0; y<ymax; y++)
            {
                if (Map->isValidBlock(x,y,z))
                {
                    Map->ReadBlock40d(x,y,z,&block);
                    Map->ReadRegionOffsets(x,y,z,&offsets);
                    veins.clear();
                    ices.clear();
                    splatter.clear();
                    Map->ReadVeins(x,y,z,&veins,&ices,&splatter);
                    for (int j=0; j<16; j++)
                    {
                        for (int i=0; i<16; i++)
                        {
                            int geolayer = block.designation[i][j].bits.geolayer_index;
                            int biome = block.designation[i][j].bits.biome;
                            tiles[z][16*x+i][16*y+j].material.type = 0;
                            tiles[z][16*x+i][16*y+j].material.index = geology[offsets[biome]][geolayer];
                            tiles[z][16*x+i][16*y+j].tiletype = block.tiletypes[i][j];
                            tiles[z][16*x+i][16*y+j].occupancy = block.occupancy[i][j];
                            tiles[z][16*x+i][16*y+j].designation = block.designation[i][j];
                            tiles[z][16*x+i][16*y+j].ceiling = false;
                            for (uint32_t v = 0; v < veins.size(); v++)
                            {
                                if (veins[v].assignment[j] &(1<<i))
                                {
                                    tiles[z][16*x+i][16*y+j].material.type = 0;
                                    tiles[z][16*x+i][16*y+j].material.index = veins[v].type;
                                }
                            }
                            if (block.global_feature != -1 && uint16_t(block.global_feature) < global_features.size() && block.designation[i][j].bits.feature_global && global_features[block.global_feature].main_material == 0)
                            {
                                tiles[z][16*x+i][16*y+j].material.type = 0;
                                tiles[z][16*x+i][16*y+j].material.index = global_features[block.global_feature].sub_material;
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
                                            tiles[z][16*x+i][16*y+j].material.type = 0;
                                            tiles[z][16*x+i][16*y+j].material.index = vectr[block.local_feature]->sub_material;
                                        }
                                    }
                                }
                            }
                            switch (DFHack::tileTypeTable[block.tiletypes[i][j]].m)
                            {
                            case DFHack::GRASS:
                                tiles[z][16*x+i][16*y+j].material.index = MAT_GRASS;
                                break;
                            case DFHack::GRASS2:
                                tiles[z][16*x+i][16*y+j].material.index = MAT_GRASS2;
                                break;
                            case DFHack::GRASS_DEAD:
                                tiles[z][16*x+i][16*y+j].material.index = MAT_GRASS_DEAD;
                                break;
                            case DFHack::GRASS_DRY:
                                tiles[z][16*x+i][16*y+j].material.index = MAT_GRASS_DRY;
                                break;
                            case DFHack::MAGMA:
                                tiles[z][16*x+i][16*y+j].material.index = MAT_MAGMA;
                                break;
                            case DFHack::ICE:
                                tiles[z][16*x+i][16*y+j].material.index = MAT_ICE;
                                break;
                            case DFHack::OBSIDIAN:
                                tiles[z][16*x+i][16*y+j].material.index = MAT_OBSIDIAN;
                                break;
                            default:
                                break;
                            }
                            if (!isRampTopOrOpenTerrain(block.tiletypes[i][j])) hasgeo = true;
                        }
                    }
                }
                else
                {
                    for (int i=0; i<16; i++)
                    {
                        for (int j=0; j<16; j++)
                        {
                            tiles[z][16*x+i][16*y+j].tiletype = -1;
                            tiles[z][16*x+i][16*y+j].ceiling = false;
                        }
                    }
                }
            }
        }
        if (hasgeo && geomax!=0) geomax = 0;
        if (!hasgeo && geomax == 0) geomax = z;
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
        tiles[c.z][c.x][c.y].material.type = c.mat_type;
        tiles[c.z][c.x][c.y].material.index = c.mat_idx | CONSTRUCTED;
    }
    Cons->Finish();
    cout << " done." << endl;
    uint32_t temp = ymax;
    ymax = xmax;
    xmax = temp; //conert to overseer coords
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

    for (uint32_t z = startz; z < zmax; z++)
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
            (*bg)[it->first]->setTexCoordArray(0,(*texcoords)[it->first].get());
            //Vec2Array *test = (*colorcoords)[it->first].get();
            getMaterial((*bg)[it->first].get(),it->first);

            zgroup->addChild(blockGeode.get());
            //cout << "Geode of " << Mats->inorganic[it->first].id << " created." << endl;

        }
        geometryGroup->addChild(zgroup.get());
        //geometryGroup->addChild(zgroup.get());

        //if (tristrip) tsv.stripify(*bg.get());
        //(*bg)[wallmat]->setVertexArray(vertices.get());
        //(*bg)[wallmat]->setNormalArray(normals.get());
        //(*bg)[wallmat]->setNormalBinding(Geometry::BIND_PER_VERTEX);
    }
    drawStairs();
    drawFortifications();
    drawFluids();
        /*ref_ptr<Node> tree = osgDB::readNodeFile("models/test.obj");
        PositionAttitudeTransform *treeplace = new PositionAttitudeTransform();
        treeplace->setPosition(Vec3(10,10,geomax));
        treeplace->addChild(tree.get());
        geometryGroup->addChild(treeplace);*/
    return true;
}

/*bool DwarfGeometry::drawVegetation()
{
    uint32_t numVegs = 0;
    Vegs->Start(numVegs);
    ref_ptr<Group> treeGroup = new Group;
    geometryGroup->addChild(treeGroup);
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
        bool usetree = false;
        switch (tree.type)
        {
        case 0:
        case 1:
            matstring += "_tree.ive";
            usetree = true;
            break;
        case 2:
        case 3:
            matstring += "_shrub.ive";
            break;
        default:
            break;
        }
        ref_ptr<Node> treenode = NULL;
        //if (vegNodes[tree.material|(1<<(31-tree.type))] == NULL && usetree)
        if (vegNodes[0] == NULL && usetree)
        {
            //treenode = dynamic_cast<Group*>(osgDB::readNodeFile("models/"+matstring));
            treenode = osgDB::readNodeFile("models/tree.ive");
            //cout << matstring << endl;
            if (treenode==NULL) continue;
            //vegNodes[tree.material|(1<<(31-tree.type))] = treenode;
            vegNodes[0] = treenode;

            osgSim::InsertImpostorsVisitor ov;
            treenode->accept(ov);
            ov.insertImpostors();
        }
        //else treenode = vegNodes[tree.material|(1<<(31-tree.type))];
        else treenode = vegNodes[0];
        //cout << "Acacia model drawn" << endl;
        ref_ptr<PositionAttitudeTransform> treeplace = new PositionAttitudeTransform();
        treeplace->setPosition(Vec3(tree.x+.5,tree.y+.5,tree.z));
        //treeplace->setAttitude(Quat(inDegrees(-90.0),Vec3d(0,0,1)));
        //treeplace->setScale(Vec3d(treeSize,treeSize,treeSize));
        treeplace->setScale(Vec3d(.1,.1,.1));

        //ref_ptr<osgSim::Impostor> impostor = new osgSim::Impostor;
        //impostor->addChild(treenode.get());
        //impostor->setRange(0,0.0,1e7f);

        treeplace->addChild(treenode.get());
        treeGroup->addChild(treeplace.get());
    }
    //cout << "made it out" << endl;
    //osgSim::InsertImpostorsVisitor ov;
    //treeGroup->accept(ov);
    //cout << "okay" << endl;
    //ov.insertImpostors();
    //cout << "dead" << endl;
    Vegs->Finish();
    return true;
}*/

bool DwarfGeometry::drawVegetation()
{
    cout << "Drawing vegetation...";
    uint32_t numVegs = 0;
    Vegs->Start(numVegs);
    ref_ptr<Group> treeGroup = new Group;
    geometryGroup->addChild(treeGroup);
    ref_ptr<Image> img = osgDB::readImageFile("materials/images/plants/tree.png");
    ref_ptr<Texture2D> tex = new Texture2D;
    tex->setImage(img.get());
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
        bool usetree = false;
        switch (tree.type)
        {
        case 0:
        case 1:
            matstring += "_tree.ive";
            usetree = true;
            break;
        case 2:
        case 3:
            matstring += "_shrub.ive";
            break;
        default:
            break;
        }
        if (usetree && !tiles[tree.z][tree.x][tree.y].designation.bits.subterranean)
        {
            ref_ptr<Billboard> treeboard = new Billboard;
            treeboard->setMode(Billboard::AXIAL_ROT);
            treeboard->setAxis(Vec3(0,0,1));
            treeboard->setNormal(Vec3(0,-1,0));
            ref_ptr<Geometry> treegeo = new Geometry;
            ref_ptr<Vec3Array> treev = new Vec3Array();
            treev->push_back(Vec3(.5,0,0));
            treev->push_back(Vec3(.5,0,1));
            treev->push_back(Vec3(-.5,0,1));
            treev->push_back(Vec3(-.5,0,0));
            treegeo->setVertexArray(treev.get());
            ref_ptr<Vec2Array> treet = new Vec2Array();
            treet->push_back(Vec2(0,0));
            treet->push_back(Vec2(0,1));
            treet->push_back(Vec2(1,1));
            treet->push_back(Vec2(1,0));
            treegeo->setTexCoordArray(0,treet.get());
            treegeo->addPrimitiveSet(new DrawArrays(PrimitiveSet::QUADS,0,4));
            ref_ptr<StateSet> ss = treegeo->getOrCreateStateSet();
            ss->setTextureAttributeAndModes(0,tex,StateAttribute::ON);
            ss->setRenderingHint(StateSet::TRANSPARENT_BIN);
            ss->setMode(GL_LIGHTING, StateAttribute::OFF);
            ss->setMode(GL_LIGHT0, StateAttribute::OFF);
            treeboard->addDrawable(treegeo.get(),Vec3(tree.y+.5,tree.x+.5,tree.z));
            //treeboard->addDrawable(treegeo.get());
            geometryGroup->addChild(treeboard.get());
        }
    }
    cout << "done" << endl;
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

//Thanks to Japa for letting me use stonesense stuff for colors!
bool DwarfGeometry::loadColors()
{
    ifstream colorIndex("colors/index.txt");
    if (!colorIndex.is_open()) return false;
    string line;
    while (!colorIndex.eof())
    {
        getline(colorIndex,line);
        line = "colors/"+line;
        TiXmlDocument *doc = new TiXmlDocument(line.c_str());
        if (!doc->LoadFile()) return false;
        TiXmlElement *root = doc->FirstChildElement("colors");
        int r=0,g=0,b=0;
        for (TiXmlElement *color = root->FirstChildElement(); color; color = color->NextSiblingElement())
        {
            color->QueryIntAttribute("red",&r);
            color->QueryIntAttribute("green",&g);
            color->QueryIntAttribute("blue",&b);
            for (TiXmlElement *type = color->FirstChildElement(); type; type = type->NextSiblingElement())
            {
                for (TiXmlElement *mat = type->FirstChildElement(); mat; mat = mat->NextSiblingElement())
                {
                    colors[mat->Attribute("value")] = new Vec4(r/255.0,g/255.0,b/255.0,1);
                }
            }
        }
    }
    return true;
}

void DwarfGeometry::drawStairs()
{
    ref_ptr<Group> stairGroup = new Group();
    ref_ptr<Group> modelGroup;
    ref_ptr<Geode> stairGeode;
    ref_ptr<Geometry> g;
    uint32_t in;
    for (uint32_t z=0; z < zmax; z++)
    {
        for (uint32_t y=0; y < ymax; y++)
        {
            for (uint32_t x=0; x < xmax; x++)
            {
                in = tiles[z][y][x].tiletype | tiles[z][y][x].material.index << 15 | tiles[z][y][x].material.type<<20;
                if (DFHack::isStairTerrain(tiles[z][y][x].tiletype))
                {
                    if (stairs[in] == NULL)
                    {
                        if (DFHack::tileTypeTable[tiles[z][y][x].tiletype].c==DFHack::STAIR_UP || DFHack::tileTypeTable[tiles[z][y][x].tiletype].c==DFHack::STAIR_UPDOWN) modelGroup = dynamic_cast<Group*>(osgDB::readNodeFile("models/stairs.ive"));
                        else continue;
                        stairGeode = dynamic_cast<Geode*>(modelGroup->getChild(0));
                        stairs[in] = stairGeode;
                        g = dynamic_cast<Geometry*>(stairGeode->getDrawable(0));
                        getMaterial(g, (tiles[z][y][x].material.type<<15) | tiles[z][y][x].material.index);
                        ref_ptr<PositionAttitudeTransform> pat = new PositionAttitudeTransform();
                        pat->setPosition(Vec3(x+.5,y+.5,z));
                        pat->addChild(stairGeode.get());
                        geometryGroup->addChild(pat.get());
                    }
                    else
                    {
                        ref_ptr<PositionAttitudeTransform> pat = new PositionAttitudeTransform();
                        pat->setPosition(Vec3(x+.5,y+.5,z));
                        pat->addChild(stairs[in].get());
                        geometryGroup->addChild(pat.get());
                    }
                }
            }
        }
    }
}

void DwarfGeometry::drawFortifications()
{
    ref_ptr<Group> fortGroup = new Group();
    ref_ptr<Group> modelGroup;
    ref_ptr<Geode> fortGeode;
    ref_ptr<Geometry> g;
    uint32_t index;
    uint32_t numConstr;
    Cons->Start(numConstr);
    for (uint32_t i=0; i<numConstr; i++)
    {
        DFHack::t_construction c;
        Cons->Read(i,c);
        index = (c.mat_type<<15) | c.mat_idx;
        if ((DFHack::tileTypeTable[tiles[c.z][c.x][c.y].tiletype].c==DFHack::FORTIFICATION) && c.z < zmax && DFHack::isOpenTerrain(tiles[c.z+1][c.x][c.y].tiletype))
        {
            if (forts[index] == NULL)
            {
                modelGroup = dynamic_cast<Group*>(osgDB::readNodeFile("models/fortification.ive"));
                fortGeode = dynamic_cast<Geode*>(modelGroup->getChild(0));
                forts[index] = fortGeode;
                g = dynamic_cast<Geometry*>(fortGeode->getDrawable(0));
                getMaterial(g, index | CONSTRUCTED);
                ref_ptr<PositionAttitudeTransform> pat = new PositionAttitudeTransform();
                pat->setPosition(Vec3(c.y+.5,c.x+.5,c.z+1));
                pat->addChild(fortGeode.get());
                geometryGroup->addChild(pat.get());
            }
            else
            {
                ref_ptr<PositionAttitudeTransform> pat = new PositionAttitudeTransform();
                pat->setPosition(Vec3(c.y+.5,c.x+.5,c.z+1));
                pat->addChild(forts[index].get());
                geometryGroup->addChild(pat.get());
            }
        }
    }
    Cons->Finish();
}


void DwarfGeometry::getMaterial(Geometry *g, uint32_t index)
{
    if (gc==NULL && doImageScaling)
    {
        ref_ptr<GraphicsContext::Traits> traits = new GraphicsContext::Traits;
        traits->x = 250;
        traits->y = 200;
        traits->width = 1;
        traits->height = 1;
        traits->windowDecoration = false;
        traits->doubleBuffer = false;
        traits->sharedContext = 0;

        gc = GraphicsContext::createGraphicsContext(traits.get());
        gc->realize();
        gc->makeCurrent();
    }
    ref_ptr<StateSet> ss = g->getOrCreateStateSet();
    ss->setMode(GL_LIGHTING, StateAttribute::ON);
    ss->setMode(GL_LIGHT0, StateAttribute::ON);
    //ss->setMode(GL_LIGHT1, StateAttribute::ON);

    if (doCulling)
    {
        CullFace *cf = new osg::CullFace(CullFace::BACK);
        ss->setAttributeAndModes(cf,StateAttribute::ON);
    }

    string matstring;
    uint32_t type = (index&0x1ffffff)>>15;
    if ((index&(7<<29))==MAT_GRASS) matstring = "grass";
    else if ((index&(7<<29))==MAT_GRASS2) matstring = "grass2";
    else if ((index&(7<<29))==MAT_GRASS_DEAD) matstring = "grass_dead";
    else if ((index&(7<<29))==MAT_GRASS_DRY) matstring = "grass_dry";
    else if ((index&(7<<29))==MAT_MAGMA) matstring = "magma";
    else if ((index&(7<<29))==MAT_ICE) matstring = "ice";
    else if ((index&(7<<29))==MAT_OBSIDIAN) matstring = "obsidian";
    else if (type==420) matstring = Mats->organic[index&0x7fff].id;
    else matstring = Mats->inorganic[index&0x7fff].id;
    for (uint32_t i = 0; i < matstring.length(); i++)
    {
        if (matstring[i]==' ') matstring[i]='_';
        else matstring[i] = tolower(matstring[i]);
    }
    //cout << matstring << endl;
    ref_ptr<Image> wallimg;
    ref_ptr<Image> wallnmap;
    ref_ptr<Program> program=0;
    ref_ptr<Shader> vs=0,fs=0;
    ref_ptr<Vec2Array> texcs = dynamic_cast<Vec2Array*>(g->getTexCoordArray(0));
    if (textures[index]!=NULL) wallimg = textures[index];
    else
    {
        wallimg = osgDB::readImageFile("materials/images/"+matstring+".bmp");
        if (imageSize != 512 && wallimg!=NULL && doImageScaling) wallimg->scaleImage(imageSize,imageSize,1);
        textures[index] = wallimg;
    }

    if (wallimg!=NULL && nmaps[index]!=NULL) wallnmap = nmaps[index];
    else
    {
        wallnmap = osgDB::readImageFile("materials/normals/"+matstring+"n.bmp");
        if (imageSize != 512 && wallnmap!=NULL && doImageScaling) wallnmap->scaleImage(imageSize,imageSize,1);
        nmaps[index] = wallnmap;
    }

    if (programs[index]!=NULL) program = programs[index];
    else
    {
        if (osgDB::findDataFile("materials/programs/"+matstring+".vert").length() != 0)
        {
            vs = Shader::readShaderFile(Shader::VERTEX,"materials/programs/"+matstring+".vert");
            fs = Shader::readShaderFile(Shader::FRAGMENT,"materials/programs/"+matstring+".frag");
        }
        else if (osgDB::findDataFile("materials/programs/bump.vert").length() != 0)
        {
            vs = Shader::readShaderFile(Shader::VERTEX,"materials/programs/bump.vert");
            fs = Shader::readShaderFile(Shader::FRAGMENT,"materials/programs/bump.frag");
        }
        if (vs && fs)
        {
            program = new Program();
            program->addShader(vs);
            program->addShader(fs);
            programs[index] = program;
        }
    }
   // ref_ptr<Image> wallimg = osgDB::readImageFile("materials/images/"+matstring+".bmp");

    Vec4 *color;
    if (type==0 && colors[Mats->inorganic[index&0x7fff].id]!=NULL) color = (colors[Mats->inorganic[index&0x7fff].id]);
    else if (type==420 && colors[Mats->organic[index&0x7fff].id]!=NULL) color = (colors[Mats->organic[index&0x7fff].id]);
    else color = NULL;

    if (wallimg != NULL)
    {
        ref_ptr<Texture2D> walltex = new Texture2D;
        walltex->setDataVariance(Object::DYNAMIC);
        walltex->setImage(wallimg.get());
        walltex->setWrap(Texture::WRAP_S,Texture::REPEAT);
        walltex->setWrap(Texture::WRAP_T,Texture::REPEAT);
        walltex->setFilter(Texture::MIN_FILTER, Texture::NEAREST_MIPMAP_NEAREST);
        walltex->setFilter(Texture::MAG_FILTER, Texture::NEAREST);
        ss->setTextureAttributeAndModes(0,walltex.get(),StateAttribute::ON);

        if (wallnmap != NULL && program != NULL && texcs != NULL && useShaders)
        {
            g->setTexCoordArray(1,texcs);
            ref_ptr<Texture2D> wallntex = new Texture2D;
            wallntex->setDataVariance(Object::DYNAMIC);
            wallntex->setImage(wallnmap.get());
            wallntex->setWrap(Texture::WRAP_S,Texture::REPEAT);
            wallntex->setWrap(Texture::WRAP_T,Texture::REPEAT);
            wallntex->setFilter(Texture::MIN_FILTER, Texture::NEAREST_MIPMAP_NEAREST);
            wallntex->setFilter(Texture::MAG_FILTER, Texture::NEAREST_MIPMAP_NEAREST);
            ss->setTextureAttributeAndModes(1,wallntex.get(),StateAttribute::ON);
            ref_ptr<Material> m = new Material();
            m->setDiffuse(Material::FRONT, Vec4(1,1,1,1));
            m->setAmbient(Material::FRONT, Vec4(1,1,1,1));
            m->setSpecular(Material::FRONT, Vec4(.2,.2,.2,1));
            m->setShininess(Material::FRONT, 60.0f);
            m->setColorMode(Material::AMBIENT_AND_DIFFUSE);
            ss->setAttributeAndModes(m, StateAttribute::ON);
            ref_ptr<osgUtil::TangentSpaceGenerator> tsg = new osgUtil::TangentSpaceGenerator;
            tsg->generate(g,1);
            g->setVertexAttribData(Geometry::ATTRIBUTE_6,Geometry::ArrayData(tsg->getTangentArray(),Geometry::BIND_PER_VERTEX,GL_FALSE));
            program->addBindAttribLocation("tangent",Geometry::ATTRIBUTE_6);
            ss->addUniform(new Uniform("diffuseTexture",0));
            ss->addUniform(new Uniform("normalTexture",1));
            ss->setAttributeAndModes(program.get(), StateAttribute::ON);
        }
        else
        {
            ref_ptr<Vec4Array> whitearray = new Vec4Array();
            whitearray->push_back(Vec4(1,1,1,1));
            g->setColorArray(whitearray);
            g->setColorBinding(Geometry::BIND_OVERALL);
        }
    }
    else if (color != NULL)
    {
        Material *m = new Material();
        m->setDiffuse(Material::FRONT, (*color));
        m->setAmbient(Material::FRONT, (*color)*.6);
        m->setSpecular(Material::FRONT, Vec4(0,0,0,1));
        ss->setAttribute(m);
        if (constex&&cracktex)
        {
            if (index&CONSTRUCTED && constex) ss->setTextureAttributeAndModes(0,constex.get(),StateAttribute::ON);
            else if (cracktex) ss->setTextureAttributeAndModes(0,cracktex.get(),StateAttribute::ON);
            ref_ptr<TexEnv> blendTexEnv = new TexEnv;
            blendTexEnv->setMode(TexEnv::DECAL);
            ss->setTextureAttribute(0,blendTexEnv);
        }

    }
    else
    {
        ref_ptr<Vec4Array> grayarray = new Vec4Array();
        grayarray->push_back(Vec4(.5,.5,.5,1));
        g->setColorArray(grayarray);
        g->setColorBinding(Geometry::BIND_OVERALL);
    }
}

void DwarfGeometry::drawFluids()
{
    ref_ptr<Group> fluidGroup = new Group();
    geometryGroup->addChild(fluidGroup);
    for (uint32_t z=0; z < zmax; z++)
    {
        cout << "Fluids z=" << z << endl;
        map<DFHack::e_liquidtype, ref_ptr<Geode> > lg;
        map<DFHack::e_liquidtype, ref_ptr<Geometry> > lgeo;
        map<DFHack::e_liquidtype, ref_ptr<Vec3Array> > lvert;
        map<DFHack::e_liquidtype, ref_ptr<Vec3Array> > lnorm;
        map<DFHack::e_liquidtype, ref_ptr<Vec2Array> > ltex;

        for (uint32_t y=0; y < ymax; y++)
        {
            for (uint32_t x=0; x < xmax; x++)
            {
                if (tiles[z][y][x].designation.bits.flow_size>0)
                {
                    DFHack::e_liquidtype ltype = tiles[z][y][x].designation.bits.liquid_type;
                    if (lg[ltype]==NULL)
                    {
                        lg[ltype] = new Geode();
                        lgeo[ltype] = new Geometry();
                        lvert[ltype] = new Vec3Array();
                        lnorm[ltype] = new Vec3Array();
                        ltex[ltype] = new Vec2Array();
                    }
                    double heights[5];
                    heights[0] = (tiles[z][y>0?y-1:0][x>0?x-1:0].designation.bits.flow_size + tiles[z][y>0?y-1:0][x].designation.bits.flow_size + tiles[z][y][x>0?x-1:0].designation.bits.flow_size + tiles[z][y][x].designation.bits.flow_size)/(4.0*7.0);
                    heights[1] = (tiles[z][y>0?y-1:0][x<xmax-1?x+1:xmax-1].designation.bits.flow_size + tiles[z][y>0?y-1:0][x].designation.bits.flow_size + tiles[z][y][x<xmax-1?x+1:xmax-1].designation.bits.flow_size + tiles[z][y][x].designation.bits.flow_size)/(4.0*7.0);
                    heights[2] = (tiles[z][y<ymax-1?y+1:ymax-1][x>0?x-1:0].designation.bits.flow_size + tiles[z][y<ymax-1?y+1:ymax-1][x].designation.bits.flow_size + tiles[z][y][x>0?x-1:0].designation.bits.flow_size + tiles[z][y][x].designation.bits.flow_size)/(4.0*7.0);
                    heights[3] = (tiles[z][y<ymax-1?y+1:ymax-1][x<xmax-1?x+1:xmax-1].designation.bits.flow_size + tiles[z][y<ymax-1?y+1:ymax-1][x].designation.bits.flow_size + tiles[z][y][x<xmax-1?x+1:xmax-1].designation.bits.flow_size + tiles[z][y][x].designation.bits.flow_size)/(4.0*7.0);
                    double havg = (heights[0]+heights[1]+heights[2]+heights[3])/4.0;
                    heights[4] = (heights[0] == heights[1] && heights[0] == heights[2] && heights[0] == heights[3])? -1 : havg;
                    lvert[ltype]->push_back(Vec3(x,y,z+heights[0]));
                    lvert[ltype]->push_back(Vec3(x+1,y,z+heights[1]));
                    lvert[ltype]->push_back(Vec3(x,y+1,z+heights[2]));
                    lvert[ltype]->push_back(Vec3(x+1,y+1,z+heights[3]));
                    if (heights[4]>=0)
                    {
                        lvert[ltype]->push_back(Vec3(x+.5,y+.5,z+heights[4]));
                        int s = lvert[ltype]->size()-1;
                        face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                        face->push_back(s);
                        face->push_back(s-2);
                        face->push_back(s-1);
                        lgeo[ltype]->addPrimitiveSet(face);
                        face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                        face->push_back(s);
                        face->push_back(s-4);
                        face->push_back(s-2);
                        lgeo[ltype]->addPrimitiveSet(face);
                        face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                        face->push_back(s);
                        face->push_back(s-3);
                        face->push_back(s-4);
                        lgeo[ltype]->addPrimitiveSet(face);
                        face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                        face->push_back(s);
                        face->push_back(s-3);
                        face->push_back(s-1);
                        lgeo[ltype]->addPrimitiveSet(face);
                    }
                    else
                    {
                        int s = lvert[ltype]->size()-1;
                        face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                        face->push_back(s);
                        face->push_back(s-1);
                        face->push_back(s-3);
                        face->push_back(s-2);
                        lgeo[ltype]->addPrimitiveSet(face);
                    }
                }
            }
        }
        for (uint32_t i=0; i<2; i++)
        {
            DFHack::e_liquidtype t = (DFHack::e_liquidtype)i;
            if (lg[t]==NULL) continue;
            lg[t]->addDrawable(lgeo[t].get());
            lgeo[t]->setVertexArray(lvert[t].get());
            fluidGroup->addChild(lg[t].get());
        }
    }
}



void DwarfGeometry::clean()
{
    if (gc!=NULL)
    {
        gc->releaseContext();
        gc->close();
    }
}

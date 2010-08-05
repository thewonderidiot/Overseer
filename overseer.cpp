#include <osgViewer/Viewer>
#include <osg/Drawable>
#include <osg/Geometry>
#include <osgGA/StateSetManipulator>

#include "DwarfManipulator.h"

#include <osg/Material>
#include <osg/Texture2D>
#include <osgFX/BumpMapping>
#include <osgDB/ReadFile>

#include <DFHack.h>
#include <dfhack/DFTileTypes.h>
#include <iostream>
#include <vector>

using namespace std;
using namespace osg;

Group *root;
Geode *blockGeode;
Geometry *bg;
Vec3Array *vertices;
Vec3Array *normals;
Vec2Array *texcoords;
DrawElementsUInt* face;

bool drawNorthWalls(int y, int x, int z, DFHack::mapblock40d *block, DFHack::mapblock40d *northblock, bool doNorthBoundary)
{
    bool wallStarted = false;
    short length = 0;
    for (int j = 0; j < 16; j++)
    {
        for (int i = 0; i < 16; i++)
        {
            if (!DFHack::isWallTerrain(block->tiletypes[i][j]) && ((j>0 && DFHack::isWallTerrain(block->tiletypes[i][j-1])) || (doNorthBoundary && j==0 && DFHack::isWallTerrain(northblock->tiletypes[i][15]))))
            {
                if (!wallStarted)
                {
                    vertices->push_back(Vec3(y+i,16-j-x,z));
                    vertices->push_back(Vec3(y+i,16-j-x,z+1));
                    texcoords->push_back(Vec2(0,0));
                    texcoords->push_back(Vec2(0,1));
                    normals->push_back(Vec3(0,-1,0));
                    normals->push_back(Vec3(0,-1,0));
                    wallStarted = true;
                    length = 0;
                }
                else length++;
                if (wallStarted && i==15)
                {
                    vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
                    vertices->push_back(Vec3(y+i+1,16-j-x,z));
                    normals->push_back(Vec3(0,-1,0));
                    normals->push_back(Vec3(0,-1,0));
                    texcoords->push_back(Vec2(length,1));
                    texcoords->push_back(Vec2(length,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face);
                    wallStarted = false;
                }

            }
            else if (wallStarted)
            {
                vertices->push_back(Vec3(y+i,16-j-x,z+1));
                vertices->push_back(Vec3(y+i,16-j-x,z));
                normals->push_back(Vec3(0,-1,0));
                normals->push_back(Vec3(0,-1,0));
                texcoords->push_back(Vec2(length,1));
				texcoords->push_back(Vec2(length,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = vertices->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                bg->addPrimitiveSet(face);
                wallStarted = false;
            }
        }
    }
    return true;
}

bool drawSouthWalls(int y, int x, int z, DFHack::mapblock40d *block, DFHack::mapblock40d *southblock, bool doSouthBoundary)
{
    bool wallStarted = false;
    short length = 0;
    for (int j = 0; j < 16; j++)
    {
        for (int i = 0; i < 16; i++)
        {
            if (!DFHack::isWallTerrain(block->tiletypes[i][j]) && ((j<15 && DFHack::isWallTerrain(block->tiletypes[i][j+1])) || (doSouthBoundary && j==15 && DFHack::isWallTerrain(southblock->tiletypes[i][0]))))
            {
                if (!wallStarted)
                {
                    vertices->push_back(Vec3(y+i,15-j-x,z));
					vertices->push_back(Vec3(y+i,15-j-x,z+1));
					texcoords->push_back(Vec2(0,0));
					texcoords->push_back(Vec2(0,1));
                    normals->push_back(Vec3(0,1,0));
                    normals->push_back(Vec3(0,1,0));
                    wallStarted = true;
                    length = 0;
                }
                else length++;
                if (wallStarted && i==15)
                {
                    vertices->push_back(Vec3(y+i+1,15-j-x,z+1));
                    vertices->push_back(Vec3(y+i+1,15-j-x,z));
                    normals->push_back(Vec3(0,1,0));
                    normals->push_back(Vec3(0,1,0));
                    texcoords->push_back(Vec2(length,1));
					texcoords->push_back(Vec2(length,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face);
                    wallStarted = false;
                }

            }
            else if (wallStarted)
            {
                vertices->push_back(Vec3(y+i,15-j-x,z+1));
                vertices->push_back(Vec3(y+i,15-j-x,z));
                normals->push_back(Vec3(0,1,0));
                normals->push_back(Vec3(0,1,0));
                texcoords->push_back(Vec2(length,1));
				texcoords->push_back(Vec2(length,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = vertices->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                bg->addPrimitiveSet(face);
                wallStarted = false;
            }
        }
    }
    return true;
}

bool drawWestWalls(int y, int x, int z, DFHack::mapblock40d *block, DFHack::mapblock40d *westblock, bool doWestBoundary)
{
    bool wallStarted = false;
    short length = 0;
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            if (!DFHack::isWallTerrain(block->tiletypes[i][j]) && ((i>0 && DFHack::isWallTerrain(block->tiletypes[i-1][j])) || (doWestBoundary && i==0 && DFHack::isWallTerrain(westblock->tiletypes[15][j]))))
            {
                if (!wallStarted)
                {
                    vertices->push_back(Vec3(y+i,16-j-x,z+1));
					vertices->push_back(Vec3(y+i,16-j-x,z));
                    normals->push_back(Vec3(1,0,0));
                    normals->push_back(Vec3(1,0,0));
                    texcoords->push_back(Vec2(0,1));
					texcoords->push_back(Vec2(0,0));
                    wallStarted = true;
                    length = 0;
                }
                else length++;
                if (wallStarted && j==15)
                {
                    vertices->push_back(Vec3(y+i,15-j-x,z));
                    vertices->push_back(Vec3(y+i,15-j-x,z+1));
                    normals->push_back(Vec3(1,0,0));
                    normals->push_back(Vec3(1,0,0));
                    texcoords->push_back(Vec2(length,0));
					texcoords->push_back(Vec2(length,1));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face);
                    wallStarted = false;
                }

            }
            else if (wallStarted)
            {
                vertices->push_back(Vec3(y+i,16-j-x,z));
                vertices->push_back(Vec3(y+i,16-j-x,z+1));
                normals->push_back(Vec3(1,0,0));
                normals->push_back(Vec3(1,0,0));
                texcoords->push_back(Vec2(length,0));
				texcoords->push_back(Vec2(length,1));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = vertices->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                bg->addPrimitiveSet(face);
                wallStarted = false;
            }
        }
    }
    return true;
}
bool drawEastWalls(int y, int x, int z, DFHack::mapblock40d *block, DFHack::mapblock40d *eastblock, bool doEastBoundary)
{
    bool wallStarted = false;
    short length = 0;
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            if (!DFHack::isWallTerrain(block->tiletypes[i][j]) && ((i<15 && DFHack::isWallTerrain(block->tiletypes[i+1][j])) || (doEastBoundary && i==15 && DFHack::isWallTerrain(eastblock->tiletypes[0][j]))))
            {
                if (!wallStarted)
                {
                    vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
					vertices->push_back(Vec3(y+i+1,16-j-x,z));
					texcoords->push_back(Vec2(0,1));
					texcoords->push_back(Vec2(0,0));
                    normals->push_back(Vec3(-1,0,0));
                    normals->push_back(Vec3(-1,0,0));
                    wallStarted = true;
                    length = 0;
                }
                else length++;
                if (wallStarted && j==15)
                {
                    vertices->push_back(Vec3(y+i+1,15-j-x,z));
					vertices->push_back(Vec3(y+i+1,15-j-x,z+1));
					texcoords->push_back(Vec2(length,0));
					texcoords->push_back(Vec2(length,1));
                    normals->push_back(Vec3(-1,0,0));
                    normals->push_back(Vec3(-1,0,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face);
                    wallStarted = false;
                }

            }
            else if (wallStarted)
            {
                vertices->push_back(Vec3(y+i+1,16-j-x,z));
				vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
                normals->push_back(Vec3(-1,0,0));
                normals->push_back(Vec3(-1,0,0));
                texcoords->push_back(Vec2(length,0));
				texcoords->push_back(Vec2(length,1));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = vertices->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                bg->addPrimitiveSet(face);
                wallStarted = false;
            }
        }
    }
    return true;
}

bool drawFloors(int y, int x, int z, DFHack::mapblock40d *block, DFHack::mapblock40d *downblock, bool doDownBoundary)
{
    bool floorStarted = false;
    short length = 0;
    for (int j = 0; j < 16; j++)
    {
        for (int i = 0; i < 16; i++)
        {
            if (DFHack::isFloorTerrain(block->tiletypes[i][j]) || (doDownBoundary && DFHack::isWallTerrain(block->tiletypes[i][j]) && (DFHack::isFloorTerrain(downblock->tiletypes[i][j]) || DFHack::isOpenTerrain(downblock->tiletypes[i][j]))))
            {
                if (!floorStarted)
                {
                    vertices->push_back(Vec3(y+i,16-j-x,z));
					vertices->push_back(Vec3(y+i,15-j-x,z));
					texcoords->push_back(Vec2(0,1));
					texcoords->push_back(Vec2(0,0));
                    normals->push_back(Vec3(0,0,1));
                    normals->push_back(Vec3(0,0,1));
                    floorStarted = true;
                    length = 0;
                }
                else length++;
                if (floorStarted && i==15)
                {
                    vertices->push_back(Vec3(y+i+1,15-j-x,z));
					vertices->push_back(Vec3(y+i+1,16-j-x,z));
					texcoords->push_back(Vec2(length,0));
					texcoords->push_back(Vec2(length,1));
                    normals->push_back(Vec3(0,0,1));
                    normals->push_back(Vec3(0,0,1));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face);
                    floorStarted = false;
                }

            }
            else if (floorStarted)
            {
                vertices->push_back(Vec3(y+i,15-j-x,z));
				vertices->push_back(Vec3(y+i,16-j-x,z));
                normals->push_back(Vec3(0,0,1));
                normals->push_back(Vec3(0,0,1));
                texcoords->push_back(Vec2(length,0));
				texcoords->push_back(Vec2(length,1));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = vertices->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                bg->addPrimitiveSet(face);
                floorStarted = false;
            }
        }
    }
    return true;
}

void drawNorthRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *northblock, DFHack::mapblock40d *southblock, bool doNorthBoundary, bool doSouthBoundary)
{
	vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
	vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
	vertices->push_back(Vec3(y+i,16-j-x,z+1));
	vertices->push_back(Vec3(y+i,16-j-x,z+1));
	vertices->push_back(Vec3(y+i,15-j-x,z));
	vertices->push_back(Vec3(y+i,15-j-x,z));
	vertices->push_back(Vec3(y+i+1,15-j-x,z));
	vertices->push_back(Vec3(y+i+1,15-j-x,z));
	vertices->push_back(Vec3(y+i+1,16-j-x,z));
	vertices->push_back(Vec3(y+i,16-j-x,z));
	normals->push_back(Vec3(0,-1,1));
	normals->push_back(Vec3(1,0,0));
	normals->push_back(Vec3(0,-1,1));
	normals->push_back(Vec3(-1,0,0));
	normals->push_back(Vec3(0,-1,1));
	normals->push_back(Vec3(-1,0,0));
	normals->push_back(Vec3(0,-1,1));
	normals->push_back(Vec3(1,0,0));
	normals->push_back(Vec3(1,0,0));
	normals->push_back(Vec3(-1,0,0));
	int s = vertices->size()-1;
	face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
	face->push_back(s-3);
	face->push_back(s-5);
	face->push_back(s-7);
	face->push_back(s-9);
	bg->addPrimitiveSet(face);
	face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
	face->push_back(s-1);
	face->push_back(s-2);
	face->push_back(s-8);
	bg->addPrimitiveSet(face);
	face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
	face->push_back(s);
	face->push_back(s-4);
	face->push_back(s-6);
	bg->addPrimitiveSet(face);
}

void drawSouthRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *southblock, DFHack::mapblock40d *northblock, bool doSouthBoundary, bool doNorthBoundary)
{
	vertices->push_back(Vec3(y+i,15-j-x,z+1));
	vertices->push_back(Vec3(y+i,15-j-x,z+1));
	vertices->push_back(Vec3(y+i+1,15-j-x,z+1));
	vertices->push_back(Vec3(y+i+1,15-j-x,z+1));
	vertices->push_back(Vec3(y+i+1,16-j-x,z));
	vertices->push_back(Vec3(y+i+1,16-j-x,z));
	vertices->push_back(Vec3(y+i,16-j-x,z));
	vertices->push_back(Vec3(y+i,16-j-x,z));
	vertices->push_back(Vec3(y+i,15-j-x,z));
	vertices->push_back(Vec3(y+i+1,15-j-x,z));
	normals->push_back(Vec3(0,1,1));
	normals->push_back(Vec3(-1,0,0));
	normals->push_back(Vec3(0,1,1));
	normals->push_back(Vec3(1,0,0));
	normals->push_back(Vec3(0,1,1));
	normals->push_back(Vec3(1,0,0));
	normals->push_back(Vec3(0,1,1));
	normals->push_back(Vec3(-1,0,0));
	normals->push_back(Vec3(-1,0,0));
	normals->push_back(Vec3(1,0,0));
	int s = vertices->size()-1;
	face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
	face->push_back(s-3);
	face->push_back(s-5);
	face->push_back(s-7);
	face->push_back(s-9);
	bg->addPrimitiveSet(face);
	face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
	face->push_back(s-1);
	face->push_back(s-2);
	face->push_back(s-8);
	bg->addPrimitiveSet(face);
	face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
	face->push_back(s);
	face->push_back(s-4);
	face->push_back(s-6);
	bg->addPrimitiveSet(face);
}

void drawWestRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *westblock, DFHack::mapblock40d *eastblock, bool doWestBoundary, bool doEastBoundary)
{
	vertices->push_back(Vec3(y+i,16-j-x,z+1));
	vertices->push_back(Vec3(y+i,16-j-x,z+1));
	vertices->push_back(Vec3(y+i,15-j-x,z+1));
	vertices->push_back(Vec3(y+i,15-j-x,z+1));
	vertices->push_back(Vec3(y+i+1,15-j-x,z));
	vertices->push_back(Vec3(y+i+1,15-j-x,z));
	vertices->push_back(Vec3(y+i+1,16-j-x,z));
	vertices->push_back(Vec3(y+i+1,16-j-x,z));
	vertices->push_back(Vec3(y+i,16-j-x,z));
	vertices->push_back(Vec3(y+i,15-j-x,z));
	normals->push_back(Vec3(1,0,1));
	normals->push_back(Vec3(0,1,0));
	normals->push_back(Vec3(1,0,1));
	normals->push_back(Vec3(0,-1,0));
	normals->push_back(Vec3(1,0,1));
	normals->push_back(Vec3(0,-1,0));
	normals->push_back(Vec3(1,0,1));
	normals->push_back(Vec3(0,1,0));
	normals->push_back(Vec3(0,1,0));
	normals->push_back(Vec3(0,-1,0));
	int s = vertices->size()-1;
	face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
	face->push_back(s-3);
	face->push_back(s-5);
	face->push_back(s-7);
	face->push_back(s-9);
	bg->addPrimitiveSet(face);
	face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
	face->push_back(s-1);
	face->push_back(s-2);
	face->push_back(s-8);
	bg->addPrimitiveSet(face);
	face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
	face->push_back(s);
	face->push_back(s-4);
	face->push_back(s-6);
	bg->addPrimitiveSet(face);
}

void drawEastRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *eastblock, DFHack::mapblock40d *westblock, bool doEastBoundary, bool doWestBoundary)
{
	vertices->push_back(Vec3(y+i+1,15-j-x,z+1));
	vertices->push_back(Vec3(y+i+1,15-j-x,z+1));
	vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
	vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
	vertices->push_back(Vec3(y+i,16-j-x,z));
	vertices->push_back(Vec3(y+i,16-j-x,z));
	vertices->push_back(Vec3(y+i,15-j-x,z));
	vertices->push_back(Vec3(y+i,15-j-x,z));
	vertices->push_back(Vec3(y+i+1,15-j-x,z));
	vertices->push_back(Vec3(y+i+1,16-j-x,z));
	normals->push_back(Vec3(-1,0,1));
	normals->push_back(Vec3(0,-1,0));
	normals->push_back(Vec3(-1,0,1));
	normals->push_back(Vec3(0,1,0));
	normals->push_back(Vec3(-1,0,1));
	normals->push_back(Vec3(0,1,0));
	normals->push_back(Vec3(-1,0,1));
	normals->push_back(Vec3(0,-1,0));
	normals->push_back(Vec3(0,-1,0));
	normals->push_back(Vec3(0,1,0));
	int s = vertices->size()-1;
	face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
	face->push_back(s-3);
	face->push_back(s-5);
	face->push_back(s-7);
	face->push_back(s-9);
	bg->addPrimitiveSet(face);
	face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
	face->push_back(s-1);
	face->push_back(s-2);
	face->push_back(s-8);
	bg->addPrimitiveSet(face);
	face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
	face->push_back(s);
	face->push_back(s-4);
	face->push_back(s-6);
	bg->addPrimitiveSet(face);
}

bool drawRamps(int y, int x, int z, DFHack::mapblock40d *block, //these should be changed to accept an array, so blocks[][][] and exists[][][] need to be reformatted for z,y,z
			DFHack::mapblock40d *northwestblock, DFHack::mapblock40d *northblock, DFHack::mapblock40d *northeastblock, DFHack::mapblock40d *westblock, DFHack::mapblock40d *eastblock,
			DFHack::mapblock40d *southwestblock, DFHack::mapblock40d *southblock, DFHack::mapblock40d *southeastblock,
			bool doNorthwestBoundary, bool doNorthBoundary, bool doNortheastBoundary, bool doWestBoundary, bool doEastBoundary, bool doSouthwestBoundary, bool doSouthBoundary, bool doSoutheastBoundary)
{
	for (int j = 0; j < 16; j++)
	{
		for (int i = 0; i < 16; i++)
		{
			if (DFHack::isRampTerrain(block->tiletypes[i][j])) //I sincerely apologize for this. This NEEDS to be changed. I am disappoint with myself.
			{
				//starting off with corner situations
				//northwest
				if (((j>0 && DFHack::isRampTerrain(block->tiletypes[i][j-1]))||(j==0 && doNorthBoundary && DFHack::isRampTerrain(northblock->tiletypes[i][15]))) //north is a ramp
				&& ((i>0 && DFHack::isRampTerrain(block->tiletypes[i-1][j]))||(i==0 && doWestBoundary && DFHack::isRampTerrain(westblock->tiletypes[15][j])))) //west is a ramp
				{
					if ((i>0 && j>0 && DFHack::isWallTerrain(block->tiletypes[i-1][j-1]))||(i==0 && j>0 && doWestBoundary && DFHack::isWallTerrain(westblock->tiletypes[15][j-1]))
					|| (i>0 && j==0 && doNorthBoundary && DFHack::isWallTerrain(northblock->tiletypes[i-1][15])) || (i==0 && j==0 && doNorthwestBoundary && DFHack::isWallTerrain(northwestblock->tiletypes[15][15]))) //northwest is a wall
					{
						vertices->push_back(Vec3(y+i,16-j-x,z+1)); //top of crease
						vertices->push_back(Vec3(y+i,16-j-x,z+1));
						vertices->push_back(Vec3(y+i,16-j-x,z+1));
						vertices->push_back(Vec3(y+i,16-j-x,z+1));


						vertices->push_back(Vec3(y+i+1,15-j-x,z)); //bottom of crease
						vertices->push_back(Vec3(y+i+1,15-j-x,z));

						vertices->push_back(Vec3(y+i,16-j-x,z)); //below top of crease
						vertices->push_back(Vec3(y+i,16-j-x,z));

						vertices->push_back(Vec3(y+i+1,16-j-x,z)); //right wing
						vertices->push_back(Vec3(y+i+1,16-j-x,z));

						vertices->push_back(Vec3(y+i,15-j-x,z)); //left wing
						vertices->push_back(Vec3(y+i,15-j-x,z));

						normals->push_back(Vec3(1,0,1));
						normals->push_back(Vec3(0,-1,1));
						normals->push_back(Vec3(0,1,0));
						normals->push_back(Vec3(-1,0,0));

						normals->push_back(Vec3(1,0,1));
						normals->push_back(Vec3(0,-1,1));

						normals->push_back(Vec3(0,1,0));
						normals->push_back(Vec3(-1,0,0));

						normals->push_back(Vec3(1,0,1));
						normals->push_back(Vec3(0,1,0));

						normals->push_back(Vec3(0,-1,1));
						normals->push_back(Vec3(-1,0,0));

						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						int s = vertices->size()-1;
						face->push_back(s-3);
						face->push_back(s-7);
						face->push_back(s-11);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						face->push_back(s-1);
						face->push_back(s-6);
						face->push_back(s-10);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						face->push_back(s-2);
						face->push_back(s-5);
						face->push_back(s-9);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						face->push_back(s);
						face->push_back(s-4);
						face->push_back(s-8);
						bg->addPrimitiveSet(face);
						continue;
					}
					else if (((j<15 && DFHack::isWallTerrain(block->tiletypes[i][j+1]))||(j==15 && doSouthBoundary && DFHack::isWallTerrain(southblock->tiletypes[i][0]))) //south is a wall
					&& ((i<15 && DFHack::isWallTerrain(block->tiletypes[i+1][j]))||(i==15 && doEastBoundary && DFHack::isWallTerrain(eastblock->tiletypes[0][j])))) //east is a wall
					{
						vertices->push_back(Vec3(y+i,16-j-x,z)); //bottom of crease
						vertices->push_back(Vec3(y+i,16-j-x,z));
						vertices->push_back(Vec3(y+i,16-j-x,z));
						vertices->push_back(Vec3(y+i,16-j-x,z));

						vertices->push_back(Vec3(y+i+1,15-j-x,z+1)); //top of crease
						vertices->push_back(Vec3(y+i+1,15-j-x,z+1));
						vertices->push_back(Vec3(y+i+1,15-j-x,z+1));
						vertices->push_back(Vec3(y+i+1,15-j-x,z+1));

						vertices->push_back(Vec3(y+i+1,16-j-x,z+1)); //top of left wing
						vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
						vertices->push_back(Vec3(y+i+1,16-j-x,z+1));

						vertices->push_back(Vec3(y+i,15-j-x,z+1)); //top of right wing
						vertices->push_back(Vec3(y+i,15-j-x,z+1));
						vertices->push_back(Vec3(y+i,15-j-x,z+1));

						vertices->push_back(Vec3(y+i+1,15-j-x,z)); //below top of crease
						vertices->push_back(Vec3(y+i+1,15-j-x,z));

						vertices->push_back(Vec3(y+i,15-j-x,z)); //sub-right wing
						vertices->push_back(Vec3(y+i,15-j-x,z));

						vertices->push_back(Vec3(y+i+1,16-j-x,z)); //sub-left wing
						vertices->push_back(Vec3(y+i+1,16-j-x,z));

						normals->push_back(Vec3(-1,0,1));
						normals->push_back(Vec3(0,1,1));
						normals->push_back(Vec3(-1,0,0));
						normals->push_back(Vec3(0,1,0));

						normals->push_back(Vec3(-1,0,1));
						normals->push_back(Vec3(0,1,1));
						normals->push_back(Vec3(0,-1,0));
						normals->push_back(Vec3(1,0,0));

						normals->push_back(Vec3(-1,0,1));
						normals->push_back(Vec3(0,1,0));
						normals->push_back(Vec3(1,0,0));

						normals->push_back(Vec3(0,1,1));
						normals->push_back(Vec3(-1,0,0));
						normals->push_back(Vec3(0,-1,0));

						normals->push_back(Vec3(0,-1,0));
						normals->push_back(Vec3(1,0,0));

						normals->push_back(Vec3(-1,0,0));
						normals->push_back(Vec3(0,-1,0));

						normals->push_back(Vec3(0,1,0));
						normals->push_back(Vec3(1,0,0));

						int s = vertices->size()-1;
						face = new DrawElementsUInt(PrimitiveSet::QUADS,0); //right square wall
						face->push_back(s-2);
						face->push_back(s-5);
						face->push_back(s-13);
						face->push_back(s-6);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::QUADS,0); //left square wall
						face->push_back(s);
						face->push_back(s-4);
						face->push_back(s-12);
						face->push_back(s-9);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //right tri wall
						face->push_back(s-3);
						face->push_back(s-7);
						face->push_back(s-17);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //left tri wall
						face->push_back(s-1);
						face->push_back(s-10);
						face->push_back(s-16);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //right ramp wall
						face->push_back(s-8);
						face->push_back(s-14);
						face->push_back(s-18);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //left ramp wall
						face->push_back(s-11);
						face->push_back(s-15);
						face->push_back(s-19);
						bg->addPrimitiveSet(face);
						continue;
					}
				}
				//northeast
				if (((j>0 && DFHack::isRampTerrain(block->tiletypes[i][j-1]))||(j==0 && doNorthBoundary && DFHack::isRampTerrain(northblock->tiletypes[i][15]))) //north is a ramp
				&& ((i<15 && DFHack::isRampTerrain(block->tiletypes[i+1][j]))||(i==15 && doEastBoundary && DFHack::isRampTerrain(eastblock->tiletypes[0][j])))) //east is a ramp
				{
					if ((i<15 && j>0 && DFHack::isWallTerrain(block->tiletypes[i+1][j-1]))||(i==15 && j>0 && doEastBoundary && DFHack::isWallTerrain(eastblock->tiletypes[0][j-1]))
					|| (i<15 && j==0 && doNorthBoundary && DFHack::isWallTerrain(northblock->tiletypes[i+1][15])) || (i==15 && j==0 && doNortheastBoundary && DFHack::isWallTerrain(northeastblock->tiletypes[0][15]))) //northeast is a wall
					{
						vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
						vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
						vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
						vertices->push_back(Vec3(y+i+1,16-j-x,z+1));

						vertices->push_back(Vec3(y+i,15-j-x,z));
						vertices->push_back(Vec3(y+i,15-j-x,z));

						vertices->push_back(Vec3(y+i+1,16-j-x,z));
						vertices->push_back(Vec3(y+i+1,16-j-x,z));

						vertices->push_back(Vec3(y+i+1,15-j-x,z));
						vertices->push_back(Vec3(y+i+1,15-j-x,z));

						vertices->push_back(Vec3(y+i,16-j-x,z));
						vertices->push_back(Vec3(y+i,16-j-x,z));

						normals->push_back(Vec3(0,-1,1));
						normals->push_back(Vec3(-1,0,1));
						normals->push_back(Vec3(1,0,0));
						normals->push_back(Vec3(0,1,0));

						normals->push_back(Vec3(0,-1,1));
						normals->push_back(Vec3(-1,0,1));

						normals->push_back(Vec3(1,0,0));
						normals->push_back(Vec3(0,1,0));

						normals->push_back(Vec3(0,-1,1));
						normals->push_back(Vec3(1,0,0));

						normals->push_back(Vec3(-1,0,1));
						normals->push_back(Vec3(0,1,0));

						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						int s = vertices->size()-1;
						face->push_back(s-3);
						face->push_back(s-7);
						face->push_back(s-11);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						face->push_back(s-1);
						face->push_back(s-6);
						face->push_back(s-10);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						face->push_back(s-2);
						face->push_back(s-5);
						face->push_back(s-9);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						face->push_back(s);
						face->push_back(s-4);
						face->push_back(s-8);
						bg->addPrimitiveSet(face);
						continue;
					}
					else if (((j<15 && DFHack::isWallTerrain(block->tiletypes[i][j+1]))||(j==15 && doSouthBoundary && DFHack::isWallTerrain(southblock->tiletypes[i][0]))) //south is a wall
					&& ((i>0 && DFHack::isWallTerrain(block->tiletypes[i-1][j]))||(i==0 && doWestBoundary && DFHack::isWallTerrain(westblock->tiletypes[15][j])))) //west is a wall
					{
						vertices->push_back(Vec3(y+i+1,16-j-x,z)); //bottom of crease
						vertices->push_back(Vec3(y+i+1,16-j-x,z));
						vertices->push_back(Vec3(y+i+1,16-j-x,z));
						vertices->push_back(Vec3(y+i+1,16-j-x,z));

						vertices->push_back(Vec3(y+i,15-j-x,z+1)); //top of crease
						vertices->push_back(Vec3(y+i,15-j-x,z+1));
						vertices->push_back(Vec3(y+i,15-j-x,z+1));
						vertices->push_back(Vec3(y+i,15-j-x,z+1));

						vertices->push_back(Vec3(y+i+1,15-j-x,z+1)); //top of left wing
						vertices->push_back(Vec3(y+i+1,15-j-x,z+1));
						vertices->push_back(Vec3(y+i+1,15-j-x,z+1));

						vertices->push_back(Vec3(y+i,16-j-x,z+1)); //top of right wing
						vertices->push_back(Vec3(y+i,16-j-x,z+1));
						vertices->push_back(Vec3(y+i,16-j-x,z+1));

						vertices->push_back(Vec3(y+i,15-j-x,z)); //below top of crease
						vertices->push_back(Vec3(y+i,15-j-x,z));

						vertices->push_back(Vec3(y+i,16-j-x,z)); //sub-right wing
						vertices->push_back(Vec3(y+i,16-j-x,z));

						vertices->push_back(Vec3(y+i+1,15-j-x,z)); //sub-left wing
						vertices->push_back(Vec3(y+i+1,15-j-x,z));

						normals->push_back(Vec3(0,1,1));
						normals->push_back(Vec3(1,0,1));
						normals->push_back(Vec3(0,1,0));
						normals->push_back(Vec3(1,0,0));

						normals->push_back(Vec3(0,1,1));
						normals->push_back(Vec3(1,0,1));
						normals->push_back(Vec3(-1,0,0));
						normals->push_back(Vec3(0,-1,0));

						normals->push_back(Vec3(0,1,1));
						normals->push_back(Vec3(1,0,0));
						normals->push_back(Vec3(0,-1,0));

						normals->push_back(Vec3(1,0,1));
						normals->push_back(Vec3(0,1,0));
						normals->push_back(Vec3(-1,0,0));

						normals->push_back(Vec3(-1,0,0));
						normals->push_back(Vec3(0,-1,0));

						normals->push_back(Vec3(0,1,0));
						normals->push_back(Vec3(-1,0,0));

						normals->push_back(Vec3(1,0,0));
						normals->push_back(Vec3(0,-1,0));

						int s = vertices->size()-1;
						face = new DrawElementsUInt(PrimitiveSet::QUADS,0); //right square wall
						face->push_back(s-2);
						face->push_back(s-5);
						face->push_back(s-13);
						face->push_back(s-6);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::QUADS,0); //left square wall
						face->push_back(s);
						face->push_back(s-4);
						face->push_back(s-12);
						face->push_back(s-9);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //right tri wall
						face->push_back(s-3);
						face->push_back(s-7);
						face->push_back(s-17);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //left tri wall
						face->push_back(s-1);
						face->push_back(s-10);
						face->push_back(s-16);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //right ramp wall
						face->push_back(s-8);
						face->push_back(s-14);
						face->push_back(s-18);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //left ramp wall
						face->push_back(s-11);
						face->push_back(s-15);
						face->push_back(s-19);
						bg->addPrimitiveSet(face);
						continue;
					}
				}
				//southwest
				if (((j<15 && DFHack::isRampTerrain(block->tiletypes[i][j+1]))||(j==15 && doSouthBoundary && DFHack::isRampTerrain(southblock->tiletypes[i][0]))) //south is a ramp
				&& ((i>0 && DFHack::isRampTerrain(block->tiletypes[i-1][j]))||(i==0 && doWestBoundary && DFHack::isRampTerrain(westblock->tiletypes[15][j])))) //west is a ramp
				{
					if ((i>0 && j<15 && DFHack::isWallTerrain(block->tiletypes[i-1][j+1]))||(i==0 && j<15 && doWestBoundary && DFHack::isWallTerrain(westblock->tiletypes[15][j+1]))
					|| (i>0 && j==15 && doSouthBoundary && DFHack::isWallTerrain(southblock->tiletypes[i-1][0])) || (i==0 && j==15 && doSouthwestBoundary && DFHack::isWallTerrain(southwestblock->tiletypes[15][0]))) //southwest is a wall
					{
						vertices->push_back(Vec3(y+i,15-j-x,z+1));
						vertices->push_back(Vec3(y+i,15-j-x,z+1));
						vertices->push_back(Vec3(y+i,15-j-x,z+1));
						vertices->push_back(Vec3(y+i,15-j-x,z+1));

						vertices->push_back(Vec3(y+i+1,16-j-x,z));
						vertices->push_back(Vec3(y+i+1,16-j-x,z));

						vertices->push_back(Vec3(y+i,15-j-x,z));
						vertices->push_back(Vec3(y+i,15-j-x,z));

						vertices->push_back(Vec3(y+i,16-j-x,z));
						vertices->push_back(Vec3(y+i,16-j-x,z));

						vertices->push_back(Vec3(y+i+1,15-j-x,z));
						vertices->push_back(Vec3(y+i+1,15-j-x,z));

						normals->push_back(Vec3(0,1,1));
						normals->push_back(Vec3(1,0,1));
						normals->push_back(Vec3(-1,0,0));
						normals->push_back(Vec3(0,-1,0));

						normals->push_back(Vec3(0,1,1));
						normals->push_back(Vec3(1,0,1));

						normals->push_back(Vec3(-1,0,0));
						normals->push_back(Vec3(0,-1,0));

						normals->push_back(Vec3(0,1,1));
						normals->push_back(Vec3(-1,0,0));

						normals->push_back(Vec3(1,0,1));
						normals->push_back(Vec3(0,-1,0));

						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						int s = vertices->size()-1;
						face->push_back(s-3);
						face->push_back(s-7);
						face->push_back(s-11);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						face->push_back(s-1);
						face->push_back(s-6);
						face->push_back(s-10);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						face->push_back(s-2);
						face->push_back(s-5);
						face->push_back(s-9);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						face->push_back(s);
						face->push_back(s-4);
						face->push_back(s-8);
						bg->addPrimitiveSet(face);
						continue;
					}
					else if (((j>0 && DFHack::isWallTerrain(block->tiletypes[i][j-1]))||(j==0 && doNorthBoundary && DFHack::isWallTerrain(northblock->tiletypes[i][15]))) //north is a wall
					&& ((i<15 && DFHack::isWallTerrain(block->tiletypes[i+1][j]))||(i==15 && doEastBoundary && DFHack::isWallTerrain(eastblock->tiletypes[0][j])))) //east is a wall
					{

						vertices->push_back(Vec3(y+i,15-j-x,z)); //bottom of crease
						vertices->push_back(Vec3(y+i,15-j-x,z));
						vertices->push_back(Vec3(y+i,15-j-x,z));
						vertices->push_back(Vec3(y+i,15-j-x,z));

						vertices->push_back(Vec3(y+i+1,16-j-x,z+1)); //top of crease
						vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
						vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
						vertices->push_back(Vec3(y+i+1,16-j-x,z+1));


						vertices->push_back(Vec3(y+i,16-j-x,z+1)); //top of left wing
						vertices->push_back(Vec3(y+i,16-j-x,z+1));
						vertices->push_back(Vec3(y+i,16-j-x,z+1));

						vertices->push_back(Vec3(y+i+1,15-j-x,z+1)); //top of right wing
						vertices->push_back(Vec3(y+i+1,15-j-x,z+1));
						vertices->push_back(Vec3(y+i+1,15-j-x,z+1));

						vertices->push_back(Vec3(y+i+1,16-j-x,z)); //below top of crease
						vertices->push_back(Vec3(y+i+1,16-j-x,z));

						vertices->push_back(Vec3(y+i+1,15-j-x,z)); //sub-right wing
						vertices->push_back(Vec3(y+i+1,15-j-x,z));

						vertices->push_back(Vec3(y+i,16-j-x,z)); //sub-left wing
						vertices->push_back(Vec3(y+i,16-j-x,z));

						normals->push_back(Vec3(0,-1,1));
						normals->push_back(Vec3(-1,0,1));
						normals->push_back(Vec3(0,-1,0));
						normals->push_back(Vec3(-1,0,0));

						normals->push_back(Vec3(0,-1,1));
						normals->push_back(Vec3(-1,0,1));
						normals->push_back(Vec3(1,0,0));
						normals->push_back(Vec3(0,1,0));

						normals->push_back(Vec3(0,-1,1));
						normals->push_back(Vec3(-1,0,0));
						normals->push_back(Vec3(0,1,0));

						normals->push_back(Vec3(-1,0,1));
						normals->push_back(Vec3(0,-1,0));
						normals->push_back(Vec3(1,0,0));

						normals->push_back(Vec3(1,0,0));
						normals->push_back(Vec3(0,1,0));

						normals->push_back(Vec3(0,-1,0));
						normals->push_back(Vec3(1,0,0));

						normals->push_back(Vec3(-1,0,0));
						normals->push_back(Vec3(0,1,0));

						int s = vertices->size()-1;
						face = new DrawElementsUInt(PrimitiveSet::QUADS,0); //right square wall
						face->push_back(s-2);
						face->push_back(s-5);
						face->push_back(s-13);
						face->push_back(s-6);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::QUADS,0); //left square wall
						face->push_back(s);
						face->push_back(s-4);
						face->push_back(s-12);
						face->push_back(s-9);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //right tri wall
						face->push_back(s-3);
						face->push_back(s-7);
						face->push_back(s-17);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //left tri wall
						face->push_back(s-1);
						face->push_back(s-10);
						face->push_back(s-16);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //right ramp wall
						face->push_back(s-8);
						face->push_back(s-14);
						face->push_back(s-18);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //left ramp wall
						face->push_back(s-11);
						face->push_back(s-15);
						face->push_back(s-19);
						bg->addPrimitiveSet(face);
						continue;
					}
				}
				if (((j<15 && DFHack::isRampTerrain(block->tiletypes[i][j+1]))||(j==15 && doSouthBoundary && DFHack::isRampTerrain(southblock->tiletypes[i][0]))) //south is a ramp
				&& ((i<15 && DFHack::isRampTerrain(block->tiletypes[i+1][j]))||(i==15 && doEastBoundary && DFHack::isRampTerrain(eastblock->tiletypes[0][j])))) //east is a ramp
				{
					if ((i<15 && j<15 && DFHack::isWallTerrain(block->tiletypes[i+1][j+1]))||(i==15 && j<15 && doEastBoundary && DFHack::isWallTerrain(eastblock->tiletypes[0][j+1]))
					|| (i<15 && j==15 && doSouthBoundary && DFHack::isWallTerrain(southblock->tiletypes[i+1][0])) || (i==15 && j==15 && doSoutheastBoundary && DFHack::isWallTerrain(southeastblock->tiletypes[0][0]))) //south-east is a wall
					{
						vertices->push_back(Vec3(y+i+1,15-j-x,z+1));
						vertices->push_back(Vec3(y+i+1,15-j-x,z+1));
						vertices->push_back(Vec3(y+i+1,15-j-x,z+1));
						vertices->push_back(Vec3(y+i+1,15-j-x,z+1));

						vertices->push_back(Vec3(y+i,16-j-x,z));
						vertices->push_back(Vec3(y+i,16-j-x,z));

						vertices->push_back(Vec3(y+i+1,15-j-x,z));
						vertices->push_back(Vec3(y+i+1,15-j-x,z));

						vertices->push_back(Vec3(y+i,15-j-x,z));
						vertices->push_back(Vec3(y+i,15-j-x,z));

						vertices->push_back(Vec3(y+i+1,16-j-x,z));
						vertices->push_back(Vec3(y+i+1,16-j-x,z));

						normals->push_back(Vec3(-1,0,1));
						normals->push_back(Vec3(0,1,1));
						normals->push_back(Vec3(0,-1,0));
						normals->push_back(Vec3(1,0,0));

						normals->push_back(Vec3(-1,0,1));
						normals->push_back(Vec3(0,1,1));

						normals->push_back(Vec3(0,-1,0));
						normals->push_back(Vec3(1,0,0));

						normals->push_back(Vec3(-1,0,1));
						normals->push_back(Vec3(0,-1,0));

						normals->push_back(Vec3(0,1,1));
						normals->push_back(Vec3(1,0,0));

						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						int s = vertices->size()-1;
						face->push_back(s-3);
						face->push_back(s-7);
						face->push_back(s-11);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						face->push_back(s-1);
						face->push_back(s-6);
						face->push_back(s-10);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						face->push_back(s-2);
						face->push_back(s-5);
						face->push_back(s-9);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
						face->push_back(s);
						face->push_back(s-4);
						face->push_back(s-8);
						bg->addPrimitiveSet(face);
						continue;
					}
					else if (((j>0 && DFHack::isWallTerrain(block->tiletypes[i][j-1]))||(j==0 && doNorthBoundary && DFHack::isWallTerrain(northblock->tiletypes[i][15]))) //north is a wall
					&& ((i>0 && DFHack::isWallTerrain(block->tiletypes[i-1][j]))||(i==0 && doWestBoundary && DFHack::isWallTerrain(westblock->tiletypes[15][j])))) //west is a wall
					{
						vertices->push_back(Vec3(y+i+1,15-j-x,z)); //bottom of crease
						vertices->push_back(Vec3(y+i+1,15-j-x,z));
						vertices->push_back(Vec3(y+i+1,15-j-x,z));
						vertices->push_back(Vec3(y+i+1,15-j-x,z));


						vertices->push_back(Vec3(y+i,16-j-x,z+1)); //top of crease
						vertices->push_back(Vec3(y+i,16-j-x,z+1));
						vertices->push_back(Vec3(y+i,16-j-x,z+1));
						vertices->push_back(Vec3(y+i,16-j-x,z+1));


						vertices->push_back(Vec3(y+i,15-j-x,z+1)); //top of left wing
						vertices->push_back(Vec3(y+i,15-j-x,z+1));
						vertices->push_back(Vec3(y+i,15-j-x,z+1));

						vertices->push_back(Vec3(y+i+1,16-j-x,z+1)); //top of right wing
						vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
						vertices->push_back(Vec3(y+i+1,16-j-x,z+1));

						vertices->push_back(Vec3(y+i,16-j-x,z)); //below top of crease
						vertices->push_back(Vec3(y+i,16-j-x,z));

						vertices->push_back(Vec3(y+i+1,16-j-x,z)); //sub-right wing
						vertices->push_back(Vec3(y+i+1,16-j-x,z));

						vertices->push_back(Vec3(y+i,15-j-x,z)); //sub-left wing
						vertices->push_back(Vec3(y+i,15-j-x,z));

						normals->push_back(Vec3(1,0,1));
						normals->push_back(Vec3(0,-1,1));
						normals->push_back(Vec3(1,0,0));
						normals->push_back(Vec3(0,-1,0));

						normals->push_back(Vec3(1,0,1));
						normals->push_back(Vec3(0,-1,1));
						normals->push_back(Vec3(0,1,0));
						normals->push_back(Vec3(-1,0,0));

						normals->push_back(Vec3(1,0,1));
						normals->push_back(Vec3(0,-1,0));
						normals->push_back(Vec3(-1,0,0));

						normals->push_back(Vec3(0,-1,1));
						normals->push_back(Vec3(1,0,0));
						normals->push_back(Vec3(0,1,0));

						normals->push_back(Vec3(0,1,0));
						normals->push_back(Vec3(-1,0,0));

						normals->push_back(Vec3(1,0,0));
						normals->push_back(Vec3(0,1,0));

						normals->push_back(Vec3(0,-1,0));
						normals->push_back(Vec3(-1,0,0));

						int s = vertices->size()-1;
						face = new DrawElementsUInt(PrimitiveSet::QUADS,0); //right square wall
						face->push_back(s-2);
						face->push_back(s-5);
						face->push_back(s-13);
						face->push_back(s-6);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::QUADS,0); //left square wall
						face->push_back(s);
						face->push_back(s-4);
						face->push_back(s-12);
						face->push_back(s-9);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //right tri wall
						face->push_back(s-3);
						face->push_back(s-7);
						face->push_back(s-17);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //left tri wall
						face->push_back(s-1);
						face->push_back(s-10);
						face->push_back(s-16);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //right ramp wall
						face->push_back(s-8);
						face->push_back(s-14);
						face->push_back(s-18);
						bg->addPrimitiveSet(face);
						face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0); //left ramp wall
						face->push_back(s-11);
						face->push_back(s-15);
						face->push_back(s-19);
						bg->addPrimitiveSet(face);
						continue;
					}
				}
				if (((j>0 && DFHack::isWallTerrain(block->tiletypes[i][j-1]))||(j==0 && doNorthBoundary && DFHack::isWallTerrain(northblock->tiletypes[i][15]))) //north is a wall
				&& ((j<15 && !DFHack::isWallTerrain(block->tiletypes[i][j+1]))||(j==15 && doSouthBoundary && !DFHack::isWallTerrain(southblock->tiletypes[i][0]))||!doSouthBoundary)) //south isn't
				{
					drawNorthRamp(y,x,z,i,j,block,northblock,southblock,doNorthBoundary,doSouthBoundary);
					continue;
				}
				if (((j<15 && DFHack::isWallTerrain(block->tiletypes[i][j+1]))||(j==15 && doSouthBoundary && DFHack::isWallTerrain(southblock->tiletypes[i][0]))) //south is a wall
				&& ((j>0 && !DFHack::isWallTerrain(block->tiletypes[i][j-1]))||(j==0 && doNorthBoundary && !DFHack::isWallTerrain(northblock->tiletypes[i][15]))||!doNorthBoundary)) //north isn't
				{
					drawSouthRamp(y,x,z,i,j,block,southblock,northblock,doSouthBoundary,doNorthBoundary);
					continue;
				}
				if (((i>0 && DFHack::isWallTerrain(block->tiletypes[i-1][j]))||(i==0 && doWestBoundary && DFHack::isWallTerrain(westblock->tiletypes[15][j]))) //west is a wall
				&& ((i<15 && !DFHack::isWallTerrain(block->tiletypes[i+1][j]))||(i==15 && doEastBoundary && !DFHack::isWallTerrain(eastblock->tiletypes[0][j]))||!doEastBoundary)) //east isn't
				{
					drawWestRamp(y,x,z,i,j,block,westblock,eastblock,doWestBoundary,doEastBoundary);
					continue;
				}
				if (((i<15 && DFHack::isWallTerrain(block->tiletypes[i+1][j]))||(i==15 && doEastBoundary && DFHack::isWallTerrain(eastblock->tiletypes[0][j]))) //east is a wall
				&& ((i>0 && !DFHack::isWallTerrain(block->tiletypes[i-1][j]))||(i==0 && doWestBoundary && !DFHack::isWallTerrain(westblock->tiletypes[15][j]))||!doWestBoundary)) //west isn't
				{
					drawEastRamp(y,x,z,i,j,block,eastblock,westblock,doEastBoundary,doWestBoundary);
					continue;
				}
				if ((j>0 && DFHack::isWallTerrain(block->tiletypes[i][j-1]))||(j==0 && doNorthBoundary && DFHack::isWallTerrain(northblock->tiletypes[i][15]))||!doNorthBoundary) //north is a wall
				{
					drawNorthRamp(y,x,z,i,j,block,northblock,southblock,doNorthBoundary,doSouthBoundary);
					continue;
				}
				if ((j<15 && DFHack::isWallTerrain(block->tiletypes[i][j+1]))||(j==15 && doSouthBoundary && DFHack::isWallTerrain(southblock->tiletypes[i][0]))||!doSouthBoundary) //south is a wall
				{
					drawSouthRamp(y,x,z,i,j,block,southblock,northblock,doSouthBoundary,doNorthBoundary);
					continue;
				}
				if ((i>0 && DFHack::isWallTerrain(block->tiletypes[i-1][j]))||(i==0 && doWestBoundary && DFHack::isWallTerrain(westblock->tiletypes[15][j]))||!doWestBoundary) //west is a wall
				{
					drawWestRamp(y,x,z,i,j,block,westblock,eastblock,doWestBoundary,doEastBoundary);
					continue;
				}
				if ((i<15 && DFHack::isWallTerrain(block->tiletypes[i+1][j]))||(i==15 && doEastBoundary && DFHack::isWallTerrain(eastblock->tiletypes[0][j]))||!doEastBoundary) //east is a wall
				{
					drawEastRamp(y,x,z,i,j,block,eastblock,westblock,doEastBoundary,doWestBoundary);
					continue;
				}

				vertices->push_back(Vec3(y+i,16-j-x,z+1));
				vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
				vertices->push_back(Vec3(y+i+1,16-j-x,z));
				vertices->push_back(Vec3(y+i,16-j-x,z));
				vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
				vertices->push_back(Vec3(y+i+1,16-j-x,z+1));
				vertices->push_back(Vec3(y+i,16-j-x,z+1));
				vertices->push_back(Vec3(y+i,16-j-x,z+1));
				vertices->push_back(Vec3(y+i,15-j-x,z));
				vertices->push_back(Vec3(y+i,15-j-x,z));
				vertices->push_back(Vec3(y+i+1,15-j-x,z));
				vertices->push_back(Vec3(y+i+1,15-j-x,z));
				vertices->push_back(Vec3(y+i+1,16-j-x,z));
				vertices->push_back(Vec3(y+i,16-j-x,z));

				normals->push_back(Vec3(0,1,0));
				normals->push_back(Vec3(0,1,0));
				normals->push_back(Vec3(0,1,0));
				normals->push_back(Vec3(0,1,0));
				normals->push_back(Vec3(0,-1,1));
				normals->push_back(Vec3(1,0,0));
				normals->push_back(Vec3(0,-1,1));
				normals->push_back(Vec3(-1,0,0));
				normals->push_back(Vec3(0,-1,1));
				normals->push_back(Vec3(-1,0,0));
				normals->push_back(Vec3(0,-1,1));
				normals->push_back(Vec3(1,0,0));
				normals->push_back(Vec3(1,0,0));
				normals->push_back(Vec3(-1,0,0));

				int s = vertices->size()-1;
				face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
				face->push_back(s-3);
				face->push_back(s-5);
				face->push_back(s-7);
				face->push_back(s-9);
				bg->addPrimitiveSet(face);
				face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
				face->push_back(s-1);
				face->push_back(s-2);
				face->push_back(s-8);
				bg->addPrimitiveSet(face);
				face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
				face->push_back(s);
				face->push_back(s-4);
				face->push_back(s-6);
				bg->addPrimitiveSet(face);
				face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
				face->push_back(s-10);
				face->push_back(s-11);
				face->push_back(s-12);
				face->push_back(s-13);
				bg->addPrimitiveSet(face);
			}
		}
	}
	return true;
}

/*


    }*/




int main(int argc, char **argv)
{
    osgViewer::Viewer viewer;
    root = new Group();
    osgFX::BumpMapping *bump = new osgFX::BumpMapping();
    root->addChild(bump);

    DFHack::ContextManager *DFMgr = new DFHack::ContextManager("Memory.xml");
	DFHack::Context *DF;

    try
    {
        DF = DFMgr->getSingleContext();
        DF->Attach();
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
        return 1;
    }

    DFHack::Maps *Maps = DF->getMaps();
    if(!Maps->Start())
    {
        cerr << "Can't init map." << endl;
        return 1;
    }
    cout << "Connected to Dwarf Fortress!" << endl;

    uint32_t xmax,ymax,zmax;
    Maps->getSize(xmax,ymax,zmax);
    cout << "Embark size: " << xmax << "x" << ymax << "x" << zmax << endl;

    cout << "mapblock40d size: " << sizeof(DFHack::mapblock40d) << endl;

    DFHack::mapblock40d blocks[3][3][3];
    bool exists[3][3][3] = {{false,false,false},{false,false,false},{false,false,false}};

    Texture2D *walltex = new Texture2D;
    walltex->setDataVariance(Object::DYNAMIC);
    Image *wallimg = osgDB::readImageFile("Test.dds");
    walltex->setImage(wallimg);
    walltex->setWrap(Texture::WRAP_S,Texture::REPEAT);
    walltex->setWrap(Texture::WRAP_T,Texture::REPEAT);

    Texture2D *wallnmap = new Texture2D;
    wallnmap->setDataVariance(Object::DYNAMIC);
    Image *wallnimg = osgDB::readImageFile("TestN.dds");
    wallnmap->setImage(wallnimg);
    wallnmap->setWrap(Texture::WRAP_S,Texture::REPEAT);
    wallnmap->setWrap(Texture::WRAP_T,Texture::REPEAT);

    for (int z = 0; z < zmax; z++)
    {
        cout << "Drawing z-level " << z << "..." << endl;
        for (int y = 0; y < ymax; y++)
        {
            for (int x = 0; x < xmax; x++)
            {
                if (!Maps->isValidBlock(y,x,z)) continue;

                for (int k = 0; k <= 2; k++) //load our local 3x3 cube
                {
                    for (int i = 0; i <= 2; i++)
                    {
                        for (int j = 0; j <= 2; j++)
                        {
                            if (Maps->isValidBlock(y+j-1,x+i-1,z+k-1))
                            {
                                Maps->ReadBlock40d(y+j-1,x+i-1,z+k-1,&blocks[j][i][k]);
                                exists[j][i][k] = true;
                            }
                            else
                            {
                                exists[j][i][k] = false;
                            }
                        }
                    }
                }
                blockGeode = new Geode();
                bg = new Geometry();
                blockGeode->addDrawable(bg);
                //root->addChild(blockGeode);
                bump->addChild(blockGeode);
                vertices = new Vec3Array();
                normals = new Vec3Array();
                texcoords = new Vec2Array();
                // Draw the geometry. x and y are passed in as tile coordinates so they are corrected by a factor of 16
                drawNorthWalls(y*16,x*16,z,&blocks[1][1][1],&blocks[1][0][1],exists[1][0][1]);
                drawSouthWalls(y*16,x*16,z,&blocks[1][1][1],&blocks[1][2][1],exists[1][2][1]);
                drawWestWalls(y*16,x*16,z,&blocks[1][1][1],&blocks[0][1][1],exists[0][1][1]);
				drawEastWalls(y*16,x*16,z,&blocks[1][1][1],&blocks[2][1][1],exists[2][1][1]);
                drawFloors(y*16,x*16,z,&blocks[1][1][1],&blocks[1][1][0],exists[1][1][0]);
                //drawRamps(y*16,x*16,z,&blocks[1][1][1],&blocks[0][0][1],&blocks[1][0][1],&blocks[2][0][1],&blocks[0][1][1],&blocks[2][1][1],&blocks[0][2][1],&blocks[1][2][1],&blocks[2][2][1],exists[0][0][1],exists[1][0][1],exists[2][0][1],exists[0][1][1],exists[2][1][1],exists[0][2][1],exists[1][2][1],exists[2][2][1]);
                bg->setVertexArray(vertices);
                bg->setNormalArray(normals);
                bg->setTexCoordArray(0,texcoords);
                bg->setTexCoordArray(1,texcoords);
                bg->setNormalBinding(Geometry::BIND_PER_VERTEX);
                blockGeode->getOrCreateStateSet()->setTextureAttributeAndModes(1,walltex,StateAttribute::ON);
				blockGeode->getOrCreateStateSet()->setTextureAttributeAndModes(0,wallnmap,StateAttribute::ON);
            }
        }
    }
	bump->prepareChildren();
    viewer.setSceneData(root);
    viewer.setUpViewInWindow(20, 20, 1044, 788);
    viewer.realize();
    viewer.setCameraManipulator(new DwarfManipulator());
    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);
    for (osgViewer::Viewer::Windows::iterator itr = windows.begin(); itr!=windows.end(); itr++) (*itr)->useCursor(false);
    DF->Detach();
    viewer.run();
}

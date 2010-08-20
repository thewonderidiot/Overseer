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

#include "DwarfGeometry.h"
using namespace std;
using namespace osg;

DwarfGeometry::DwarfGeometry()
{
}
DwarfGeometry::DwarfGeometry(DFHack::Maps *m, osg::Group *g, int sz, bool er, bool ts)
{
    enableRamps = er;
    tristrip = ts;
    Map = m;
    geometryGroup = g;
    startz = sz;
}


/*void DwarfGeometry::drawNorthRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *northblock, DFHack::mapblock40d *southblock, bool doNorthBoundary, bool doSouthBoundary)
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

void DwarfGeometry::drawSouthRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *southblock, DFHack::mapblock40d *northblock, bool doSouthBoundary, bool doNorthBoundary)
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

void DwarfGeometry::drawWestRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *westblock, DFHack::mapblock40d *eastblock, bool doWestBoundary, bool doEastBoundary)
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

void DwarfGeometry::drawEastRamp(int y, int x, int z, int i, int j, DFHack::mapblock40d *block, DFHack::mapblock40d *eastblock, DFHack::mapblock40d *westblock, bool doEastBoundary, bool doWestBoundary)
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

bool DwarfGeometry::drawRamps(int y, int x, int z, DFHack::mapblock40d *block, //these should be changed to accept an array, so blocks[][][] and exists[][][] need to be reformatted for z,y,z
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
}*/

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
                        else if (x < xmax-1 && y < ymax-1 && DFHack::isRampTerrain(tiles[z][y][x+1].tiletype) && DFHack::isRampTerrain(tiles[z][y+1][x].tiletype))
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
                        else if (x < xmax-1 && y > 0 && DFHack::isRampTerrain(tiles[z][y][x+1].tiletype) && DFHack::isRampTerrain(tiles[z][y-1][x].tiletype))
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
                        else if (x > 0 && y < ymax-1 && DFHack::isRampTerrain(tiles[z][y][x-1].tiletype) && DFHack::isRampTerrain(tiles[z][y+1][x].tiletype))
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
                        else if (x>0 && y>0 && DFHack::isRampTerrain(tiles[z][y][x-1].tiletype) && DFHack::isRampTerrain(tiles[z][y-1][x].tiletype))
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
                tiles[z][y][x].ramptype = NONE;
            }
        }
    }
    cout << "done." << endl;
}


bool DwarfGeometry::drawRamps(int x, int y, int z)
{
    for (int i = x; i < x+16; i++)
    {
        for (int j = y; j < y+16; j++)
        {
            if (DFHack::isRampTerrain(tiles[z][j][i].tiletype))
            {
            }
        }
    }
    return true;
}

void DwarfGeometry::drawBlock(int x, int y, int z)
{
    cout << endl;
    for (int i = x; i < x+16; i++)
    {
        for (int j = y; j < y+16; j++)
        {
            if (DFHack::isWallTerrain(tiles[z][j][i].tiletype)) cout << "O";
            else if (DFHack::isWallTerrain(tiles[z][j][i-1].tiletype)) cout << "!";
            else if (DFHack::isRampTerrain(tiles[z][j][i].tiletype)) cout << "R";
            else if (DFHack::isFloorTerrain(tiles[z][j][i].tiletype)) cout << "+";
            else if (DFHack::isOpenTerrain(tiles[z][j][i].tiletype)) cout << " ";
            else if (DFHack::isStairTerrain(tiles[z][j][i].tiletype)) cout << "X";
        }
        cout << endl;
    }
    cout << endl;
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
            if (!DFHack::isWallTerrain(tiles[z][y][x].tiletype) && (x>0 && DFHack::isWallTerrain(tiles[z][y][x-1].tiletype)))
            {
                if (wallStarted && wallmat != tiles[z][y][x-1].material.index)
                {
                    wallStarted = false;
                    vertices->push_back(Vec3(x,y,z+1));
                    vertices->push_back(Vec3(x,y,z));
                    normals->push_back(Vec3(1,0,0));
                    normals->push_back(Vec3(1,0,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face.get());
                }
                if (!wallStarted)
                {
                    wallStarted = true;
                    wallmat = tiles[z][y][x-1].material.index;
                    vertices->push_back(Vec3(x,y,z));
                    vertices->push_back(Vec3(x,y,z+1));
                    normals->push_back(Vec3(1,0,0));
                    normals->push_back(Vec3(1,0,0));
                    length = 1;
                }
                else length++;
                if (y == ymax-1 && wallStarted)
                {
                    wallStarted = false;
                    vertices->push_back(Vec3(x,y+1,z+1));
                    vertices->push_back(Vec3(x,y+1,z));
                    normals->push_back(Vec3(1,0,0));
                    normals->push_back(Vec3(1,0,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face.get());
                }
            }
            else if (wallStarted)
            {
                wallStarted = false;
                vertices->push_back(Vec3(x,y,z+1));
                vertices->push_back(Vec3(x,y,z));
                normals->push_back(Vec3(1,0,0));
                normals->push_back(Vec3(1,0,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = vertices->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                bg->addPrimitiveSet(face.get());
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
            if (!DFHack::isWallTerrain(tiles[z][y][x].tiletype) && (x<xmax-1 && DFHack::isWallTerrain(tiles[z][y][x+1].tiletype)))
            {
                if (wallStarted && wallmat != tiles[z][y][x+1].material.index)
                {
                    wallStarted = false;
                    vertices->push_back(Vec3(x+1,y,z+1));
                    vertices->push_back(Vec3(x+1,y,z));
                    normals->push_back(Vec3(-1,0,0));
                    normals->push_back(Vec3(-1,0,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face.get());
                }
                if (!wallStarted)
                {
                    wallStarted = true;
                    wallmat = tiles[z][y][x+1].material.index;
                    vertices->push_back(Vec3(x+1,y,z));
                    vertices->push_back(Vec3(x+1,y,z+1));
                    normals->push_back(Vec3(-1,0,0));
                    normals->push_back(Vec3(-1,0,0));
                    length = 1;
                }
                else length++;
                if (y == ymax-1 && wallStarted)
                {
                    wallStarted = false;
                    vertices->push_back(Vec3(x+1,y+1,z+1));
                    vertices->push_back(Vec3(x+1,y+1,z));
                    normals->push_back(Vec3(-1,0,0));
                    normals->push_back(Vec3(-1,0,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face.get());
                }
            }
            else if (wallStarted)
            {
                wallStarted = false;
                vertices->push_back(Vec3(x+1,y,z+1));
                vertices->push_back(Vec3(x+1,y,z));
                normals->push_back(Vec3(-1,0,0));
                normals->push_back(Vec3(-1,0,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = vertices->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                bg->addPrimitiveSet(face.get());
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
            if (!DFHack::isWallTerrain(tiles[z][y][x].tiletype) && (y>0 && DFHack::isWallTerrain(tiles[z][y-1][x].tiletype)))
            {
                if (wallStarted && wallmat != tiles[z][y-1][x].material.index)
                {
                    wallStarted = false;
                    vertices->push_back(Vec3(x,y,z+1));
                    vertices->push_back(Vec3(x,y,z));
                    normals->push_back(Vec3(0,1,0));
                    normals->push_back(Vec3(0,1,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face.get());
                }
                if (!wallStarted)
                {
                    wallStarted = true;
                    wallmat = tiles[z][y-1][x].material.index;
                    vertices->push_back(Vec3(x,y,z));
                    vertices->push_back(Vec3(x,y,z+1));
                    normals->push_back(Vec3(0,1,0));
                    normals->push_back(Vec3(0,1,0));
                    length = 1;
                }
                else length++;
                if (x == xmax-1 && wallStarted)
                {
                    wallStarted = false;
                    vertices->push_back(Vec3(x+1,y,z+1));
                    vertices->push_back(Vec3(x+1,y,z));
                    normals->push_back(Vec3(0,1,0));
                    normals->push_back(Vec3(0,1,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face.get());
                }
            }
            else if (wallStarted)
            {
                wallStarted = false;
                vertices->push_back(Vec3(x,y,z+1));
                vertices->push_back(Vec3(x,y,z));
                normals->push_back(Vec3(0,1,0));
                normals->push_back(Vec3(0,1,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = vertices->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                bg->addPrimitiveSet(face.get());
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
            if (!DFHack::isWallTerrain(tiles[z][y][x].tiletype) && (y<ymax-1 && DFHack::isWallTerrain(tiles[z][y+1][x].tiletype)))
            {
                if (wallStarted && wallmat != tiles[z][y+1][x].material.index)
                {
                    wallStarted = false;
                    vertices->push_back(Vec3(x,y+1,z+1));
                    vertices->push_back(Vec3(x,y+1,z));
                    normals->push_back(Vec3(0,-1,0));
                    normals->push_back(Vec3(0,-1,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face.get());
                }
                if (!wallStarted)
                {
                    wallStarted = true;
                    wallmat = tiles[z][y+1][x].material.index;
                    vertices->push_back(Vec3(x,y+1,z));
                    vertices->push_back(Vec3(x,y+1,z+1));
                    normals->push_back(Vec3(0,-1,0));
                    normals->push_back(Vec3(0,-1,0));
                    length = 1;
                }
                else length++;
                if (x == xmax-1 && wallStarted)
                {
                    wallStarted = false;
                    vertices->push_back(Vec3(x+1,y+1,z+1));
                    vertices->push_back(Vec3(x+1,y+1,z));
                    normals->push_back(Vec3(0,-1,0));
                    normals->push_back(Vec3(0,-1,0));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face.get());
                }
            }
            else if (wallStarted)
            {
                wallStarted = false;
                vertices->push_back(Vec3(x,y+1,z+1));
                vertices->push_back(Vec3(x,y+1,z));
                normals->push_back(Vec3(0,-1,0));
                normals->push_back(Vec3(0,-1,0));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = vertices->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                bg->addPrimitiveSet(face.get());
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
                    vertices->push_back(Vec3(x+1,y,z));
                    vertices->push_back(Vec3(x,y,z));
                    normals->push_back(Vec3(0,0,1));
                    normals->push_back(Vec3(0,0,1));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face.get());
                }
                if (!wallStarted)
                {
                    wallStarted = true;
                    wallmat = tiles[z][y][x].material.index;
                    vertices->push_back(Vec3(x,y,z));
                    vertices->push_back(Vec3(x+1,y,z));
                    normals->push_back(Vec3(0,0,1));
                    normals->push_back(Vec3(0,0,1));
                    length = 1;
                }
                else length++;
                if (y == ymax-1 && wallStarted)
                {
                    wallStarted = false;
                    vertices->push_back(Vec3(x+1,y+1,z));
                    vertices->push_back(Vec3(x,y+1,z));
                    normals->push_back(Vec3(0,0,1));
                    normals->push_back(Vec3(0,0,1));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face.get());
                }
            }
            else if (wallStarted)
            {
                wallStarted = false;
                vertices->push_back(Vec3(x+1,y,z));
                vertices->push_back(Vec3(x,y,z));
                normals->push_back(Vec3(0,0,1));
                normals->push_back(Vec3(0,0,1));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = vertices->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                bg->addPrimitiveSet(face.get());
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
            if (z < zmax-1 && !DFHack::isWallTerrain(tiles[z][y][x].tiletype) && !DFHack::isOpenTerrain(tiles[z+1][y][x].tiletype))
            {
                if (wallStarted && wallmat != tiles[z+1][y][x].material.index)
                {
                    wallStarted = false;
                    vertices->push_back(Vec3(x+1,y,z+.99));
                    vertices->push_back(Vec3(x,y,z+.99));
                    normals->push_back(Vec3(0,0,-1));
                    normals->push_back(Vec3(0,0,-1));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face.get());
                }
                if (!wallStarted)
                {
                    wallStarted = true;
                    wallmat = tiles[z+1][y][x].material.index;
                    vertices->push_back(Vec3(x,y,z+.99));
                    vertices->push_back(Vec3(x+1,y,z+.99));
                    normals->push_back(Vec3(0,0,-1));
                    normals->push_back(Vec3(0,0,-1));
                    length = 1;
                }
                else length++;
                if (y == ymax-1 && wallStarted)
                {
                    wallStarted = false;
                    vertices->push_back(Vec3(x+1,y+1,z+.99));
                    vertices->push_back(Vec3(x,y+1,z+.99));
                    normals->push_back(Vec3(0,0,-1));
                    normals->push_back(Vec3(0,0,-1));
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    int s = vertices->size()-1;
                    face->push_back(s);
                    face->push_back(s-1);
                    face->push_back(s-2);
                    face->push_back(s-3);
                    bg->addPrimitiveSet(face.get());
                }
            }
            else if (wallStarted)
            {
                wallStarted = false;
                vertices->push_back(Vec3(x+1,y,z+.99));
                vertices->push_back(Vec3(x,y,z+.99));
                normals->push_back(Vec3(0,0,-1));
                normals->push_back(Vec3(0,0,-1));
                face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                int s = vertices->size()-1;
                face->push_back(s);
                face->push_back(s-1);
                face->push_back(s-2);
                face->push_back(s-3);
                bg->addPrimitiveSet(face.get());
            }
        }
    }
    return true;
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
    Map->getSize(xmax,ymax,zmax);
    tiles.resize(zmax);
    for (uint32_t z = 0; z < zmax; z++)
    {
        tiles[z].resize(16*ymax);
        for (uint32_t y = 0; y < 16*ymax; y++)
        {
            tiles[z][y].resize(16*xmax);
        }
    }

    cout << "Reading embark data...";
    for (uint32_t z=startz; z<zmax; z++)
    {
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
                            tiles[z][16*y+j][16*x+i].material.type = DFHack::STONE;
                            tiles[z][16*y+j][16*x+i].material.index = geology[offsets[biome]][geolayer];
                            tiles[z][16*y+j][16*x+i].tiletype = block.tiletypes[i][j];
                            tiles[z][16*y+j][16*x+i].occupancy = block.occupancy[i][j];
                            tiles[z][16*y+j][16*x+i].designation = block.designation[i][j];
                            for (uint32_t v = 0; v < veins.size(); v++)
                            {
                                if (veins[v].assignment[j] &(1<<i))
                                {
                                    tiles[z][16*y+j][16*x+i].material.type = DFHack::VEIN;
                                    tiles[z][16*y+j][16*x+i].material.index = veins[v].type;
                                }
                            }
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
                        }
                    }
                }
            }
        }
    }
    xmax*=16;
    ymax*=16;
    cout << " done." << endl;
    processRamps();
    return true;
}


bool DwarfGeometry::drawGeometry()
{
    for (uint32_t z = 0; z < zmax; z++)
    {
        cout << "Drawing z-level " << z << endl;
        blockGeode = new Geode();
        bg = new Geometry();
        blockGeode->addDrawable(bg.get());
        geometryGroup->addChild(blockGeode.get());
        vertices = new Vec3Array();
        normals = new Vec3Array();
        drawNorthWalls(z);
        drawSouthWalls(z);
        drawWestWalls(z);
        drawEastWalls(z);
        drawFloors(z);
        bg->setVertexArray(vertices.get());
        bg->setNormalArray(normals.get());
        bg->setNormalBinding(Geometry::BIND_PER_VERTEX);
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
                bg->setVertexArray(vertices);
                bg->setNormalArray(normals);
                if (!enableRamps)
                {
                	bg->setTexCoordArray(0,texcoords);
                	bg->setTexCoordArray(1,texcoords);
					blockGeode->getOrCreateStateSet()->setTextureAttributeAndModes(1,walltex,StateAttribute::ON);
					blockGeode->getOrCreateStateSet()->setTextureAttributeAndModes(0,wallnmap,StateAttribute::ON);
                }
                bg->setNormalBinding(Geometry::BIND_PER_VERTEX);
				if (tristrip) tri.stripify(*bg);
				blockGeode = new Geode();
                bg = new Geometry();
                blockGeode->addDrawable(bg);
                geometryGroup->addChild(blockGeode);
                vertices = new Vec3Array();
                normals = new Vec3Array();
                texcoords = new Vec2Array();

				bg->setVertexArray(vertices);
                bg->setNormalArray(normals);
                if (!enableRamps)
                {
					bg->setTexCoordArray(0,texcoords);
					bg->setTexCoordArray(1,texcoords);
					blockGeode->getOrCreateStateSet()->setTextureAttributeAndModes(1,floortex,StateAttribute::ON);
					blockGeode->getOrCreateStateSet()->setTextureAttributeAndModes(0,floornmap,StateAttribute::ON);
                }
                bg->setNormalBinding(Geometry::BIND_PER_VERTEX);
                if (tristrip) tri.stripify(*bg);
            }
        }
    }
    return true;
}*/

/*void DwarfGeometry::clean()
{

}*/

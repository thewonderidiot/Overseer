#include <osgViewer/Viewer>
#include <osg/Drawable>
#include <osg/Geometry>
#include <osgGA/GUIEventHandler>
#include <osgGA/StateSetManipulator>
#include <osgUtil/Simplifier>
#include <osgUtil/Optimizer>
#include <osgUtil/TriStripVisitor>
#include <osgUtil/SmoothingVisitor>

#include "DwarfManipulator.h"

#include <osg/Material>
#include <DFHack.h>
#include <dfhack/DFTileTypes.h>
#include <iostream>
#include <vector>

using namespace std;
using namespace osg;

enum {VWALL, HWALL, FLOOR};

int main(int argc, char **argv)
{
    osgViewer::Viewer viewer;
    Group *root = new Group();
    Geode *blockGeode;
    Geometry *bg;
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
    DFHack::mapblock40d block, upblock, downblock, leftblock, rightblock, frontblock, backblock,backleftblock,backrightblock,frontleftblock,frontrightblock;
    bool hasup, hasdown, hasleft, hasright, hasback, hasfront, hasbackleft, hasbackright, hasfrontleft, hasfrontright;
    osgUtil::TriStripVisitor tri(new osgUtil::Optimizer());
    for (int z = 161; z < 164; z++)
    {
        cout << "Loading z-level " << z << endl;
        for (int y = 11; y < 12; y++)
        {
            for (int x = 11; x < 12; x++)
            {
                if (!Maps->isValidBlock(y,x,z)) continue;
                int tx = x*16;
                int ty = y*16;
                int tz = z;
                blockGeode = new Geode();
                bg = new Geometry();
                blockGeode->addDrawable(bg);
                root->addChild(blockGeode);

                Maps->ReadBlock40d(y,x,z,&block);
                hasup = hasdown = hasleft = hasright = hasback = hasfront = hasbackleft = hasbackright = hasfrontleft = hasfrontright = true;
                if (Maps->isValidBlock(y-1,x,z)) Maps->ReadBlock40d(y-1,x,z,&leftblock); else hasleft = false;
                if (Maps->isValidBlock(y+1,x,z)) Maps->ReadBlock40d(y+1,x,z,&rightblock); else hasright = false;
                if (Maps->isValidBlock(y,x-1,z)) Maps->ReadBlock40d(y,x-1,z,&backblock); else hasback = false;
                if (Maps->isValidBlock(y,x+1,z)) Maps->ReadBlock40d(y,x+1,z,&frontblock); else hasfront = false;
                if (Maps->isValidBlock(y,x,z-1)) Maps->ReadBlock40d(y,x,z-1,&downblock); else hasdown = false;
                if (Maps->isValidBlock(y,x,z+1)) Maps->ReadBlock40d(y,x,z+1,&upblock); else hasup = false;
                if (Maps->isValidBlock(y-1,x-1,z)) Maps->ReadBlock40d(y-1,x-1,z,&backleftblock); else hasbackleft = false;
                if (Maps->isValidBlock(y+1,x-1,z)) Maps->ReadBlock40d(y+1,x-1,z,&backrightblock); else hasbackright = false;
                if (Maps->isValidBlock(y-1,x+1,z)) Maps->ReadBlock40d(y-1,x+1,z,&frontleftblock); else hasfrontleft = false;
                if (Maps->isValidBlock(y+1,x+1,z)) Maps->ReadBlock40d(y+1,x+1,z,&frontrightblock); else hasfrontright = false;
                Vec3Array *bvu = new Vec3Array();
                Vec3Array *bvr = new Vec3Array();
                Vec3Array *bvd = new Vec3Array();
                Vec3Array *bvl = new Vec3Array();
                Vec3Array *bvf = new Vec3Array();
                Vec3Array *bvulramps = new Vec3Array();
                Vec3Array *bvurramps = new Vec3Array();
                Vec3Array *bvdlramps = new Vec3Array();
                Vec3Array *bvdrramps = new Vec3Array();
                Vec3Array *bvuramps = new Vec3Array();
                Vec3Array *bvframps = new Vec3Array();
                Vec3Array *bvlramps = new Vec3Array();
                Vec3Array *bvrramps = new Vec3Array();
                int uwalls = 0, dwalls=0, rwalls=0, lwalls=0, floors=0, ulramps=0, urramps=0, uramps=0, framps=0;

                Vec3Array *normals = new Vec3Array();
                Vec3Array *rampnormals = new Vec3Array();
                bool wallAStarted = false, wallBStarted = false, floorStarted = false;

                for (int j = 0; j < 16; j++)
                {
                    for (int i = 0; i < 16; i++)
                    {
                        //if (DFHack::isWallTerrain(block.tiletypes[i][j])) cout << "O";
                       // else if (DFHack::isFloorTerrain(block.tiletypes[i][j])) cout << "+";
                        //else if (DFHack::isRampTerrain(block.tiletypes[i][j])) cout << "R";
                        //else if (DFHack::isStairTerrain(block.tiletypes[i][j])) cout << "X";
                        //else if (DFHack::isOpenTerrain(block.tiletypes[i][j])) cout << " ";
                        //else cout << "?";

                        if (!DFHack::isWallTerrain(block.tiletypes[i][j]) && ((j>0 && DFHack::isWallTerrain(block.tiletypes[i][j-1])) || (hasback && j==0 && DFHack::isWallTerrain(backblock.tiletypes[i][15]))))
                        {
                            if (!wallAStarted)
                            {
                                bvu->push_back(Vec3(ty+i,16-j-tx,tz));
                                bvu->push_back(Vec3(ty+i,16-j-tx,tz+1));
                                uwalls++;
                                wallAStarted = true;
                            }
                            if (wallAStarted && i==15)
                            {
                                bvu->push_back(Vec3(ty+i+1,16-j-tx,tz+1));
                                bvu->push_back(Vec3(ty+i+1,16-j-tx,tz));
                                wallAStarted = false;
                            }

                        }
                        else if (wallAStarted)
                        {
                            bvu->push_back(Vec3(ty+i,16-j-tx,tz+1));
                            bvu->push_back(Vec3(ty+i,16-j-tx,tz));
                            wallAStarted = false;
                        }

                        if (!DFHack::isWallTerrain(block.tiletypes[i][j]) && ((j<15 && DFHack::isWallTerrain(block.tiletypes[i][j+1])) || (hasfront && j==15 && DFHack::isWallTerrain(frontblock.tiletypes[i][0]))))
                        {
                            if (!wallBStarted)
                            {
                                bvd->push_back(Vec3(ty+i,15-j-tx,tz));
                                bvd->push_back(Vec3(ty+i,15-j-tx,tz+1));
                                dwalls++;
                                wallBStarted = true;
                            }
                            if (wallBStarted && i==15)
                            {
                                bvd->push_back(Vec3(ty+i+1,15-j-tx,tz+1));
                                bvd->push_back(Vec3(ty+i+1,15-j-tx,tz));
                                wallBStarted = false;
                            }

                        }
                        else if (wallBStarted)
                        {
                            bvd->push_back(Vec3(ty+i,15-j-tx,tz+1));
                            bvd->push_back(Vec3(ty+i,15-j-tx,tz));
                            wallBStarted = false;
                        }
                        //Floor in horizontal strips. Optimize later!
                        if (DFHack::isFloorTerrain(block.tiletypes[i][j]) || (hasdown && DFHack::isWallTerrain(block.tiletypes[i][j]) && (DFHack::isFloorTerrain(downblock.tiletypes[i][j]) || DFHack::isOpenTerrain(downblock.tiletypes[i][j]))))
                        {
                            if (!floorStarted)
                            {
                                bvf->push_back(Vec3(ty+i,16-j-tx,tz));
                                bvf->push_back(Vec3(ty+i,15-j-tx,tz));
                                floors++;
                                floorStarted=true;
                            }
                            if (floorStarted && i==15)
                            {
                                bvf->push_back(Vec3(ty+i+1,15-j-tx,tz));
                                bvf->push_back(Vec3(ty+i+1,16-j-tx,tz));
                                floorStarted=false;
                            }
                        }
                        else if (floorStarted)
                        {
                            bvf->push_back(Vec3(ty+i,15-j-tx,tz));
                            bvf->push_back(Vec3(ty+i,16-j-tx,tz));
                            floorStarted=false;
                        }
                        //ramps are even worse --completely unoptimized! For most fortresses this is okay.
                        if (DFHack::isRampTerrain(block.tiletypes[i][j])) //I sincerely apologize for this. This NEEDS to be changed. I am disappoint with myself.
                        {
                            //starting off with corner situations
                            //top-left
                            if (((j>0 && DFHack::isRampTerrain(block.tiletypes[i][j-1]))||(j==0 && hasback && DFHack::isRampTerrain(backblock.tiletypes[i][15]))) //up is a ramp
                            && ((i>0 && DFHack::isRampTerrain(block.tiletypes[i-1][j]))||(i==0 && hasleft && DFHack::isRampTerrain(leftblock.tiletypes[15][j])))) //left is a ramp
                            {
                                ulramps++;
                                if ((i>0 && j>0 && DFHack::isWallTerrain(block.tiletypes[i-1][j-1]))||(i==0 && j>0 && hasleft && DFHack::isWallTerrain(leftblock.tiletypes[15][j-1]))
                                || (i>0 && j==0 && hasback && DFHack::isWallTerrain(backblock.tiletypes[i-1][15])) || (i==0 && j==0 && hasbackleft && DFHack::isWallTerrain(backleftblock.tiletypes[15][15]))) //up-left is a wall
                                {
                                    cout << "Q";
                                    bvulramps->push_back(Vec3(ty+i,16-j-tx,tz+1));
                                    bvulramps->push_back(Vec3(ty+i+1,15-j-tx,tz));
                                    bvulramps->push_back(Vec3(ty+i+1,16-j-tx,tz));
                                    bvulramps->push_back(Vec3(ty+i,15-j-tx,tz));
                                }
                                else //up-left is a floor
                                {
                                    cout << "q";
                                    bvulramps->push_back(Vec3(ty+i,16-j-tx,tz));
                                    bvulramps->push_back(Vec3(ty+i+1,15-j-tx,tz+1));
                                    bvulramps->push_back(Vec3(ty+i+1,16-j-tx,tz+1));
                                    bvulramps->push_back(Vec3(ty+i,15-j-tx,tz+1));
                                }
                            }
                            //top-right
                            else if (((j>0 && DFHack::isRampTerrain(block.tiletypes[i][j-1]))||(j==0 && hasback && DFHack::isRampTerrain(backblock.tiletypes[i][15]))) //up is a ramp
                            && ((i<15 && DFHack::isRampTerrain(block.tiletypes[i+1][j]))||(i==15 && hasright && DFHack::isRampTerrain(rightblock.tiletypes[0][j])))) //right is a ramp
                            {
                                urramps++;
                                if ((i<15 && j>0 && DFHack::isWallTerrain(block.tiletypes[i+1][j-1]))||(i==15 && j>0 && hasright && DFHack::isWallTerrain(rightblock.tiletypes[0][j-1]))
                                || (i<15 && j==0 && hasback && DFHack::isWallTerrain(backblock.tiletypes[i+1][15])) || (i==15 && j==0 && hasbackright && DFHack::isWallTerrain(backrightblock.tiletypes[0][15]))) //up-right is a wall
                                {
                                    cout << "E";
                                    bvurramps->push_back(Vec3(ty+i+1,16-j-tx,tz+1));
                                    bvurramps->push_back(Vec3(ty+i,15-j-tx,tz));
                                    bvurramps->push_back(Vec3(ty+i+1,15-j-tx,tz));
                                    bvurramps->push_back(Vec3(ty+i,16-j-tx,tz));
                                }
                                else //up-right is a floor
                                {
                                    cout << "e";
                                    bvurramps->push_back(Vec3(ty+i+1,16-j-tx,tz));
                                    bvurramps->push_back(Vec3(ty+i,15-j-tx,tz+1));
                                    bvurramps->push_back(Vec3(ty+i+1,15-j-tx,tz+1));
                                    bvurramps->push_back(Vec3(ty+i,16-j-tx,tz+1));
                                }
                            }
                            //bottom-left
                            else if (((j<15 && DFHack::isRampTerrain(block.tiletypes[i][j+1]))||(j==15 && hasfront && DFHack::isRampTerrain(frontblock.tiletypes[i][0]))) //front is a ramp
                            && ((i>0 && DFHack::isRampTerrain(block.tiletypes[i-1][j]))||(i==0 && hasleft && DFHack::isRampTerrain(leftblock.tiletypes[15][j])))) //left is a ramp
                            {
                                if ((i>0 && j<15 && DFHack::isWallTerrain(block.tiletypes[i-1][j+1]))||(i==0 && j<15 && hasleft && DFHack::isWallTerrain(leftblock.tiletypes[15][j+1]))
                                || (i>0 && j==15 && hasfront && DFHack::isWallTerrain(frontblock.tiletypes[i-1][0])) || (i==0 && j==15 && hasfrontleft && DFHack::isWallTerrain(frontleftblock.tiletypes[15][0]))) //front-left is a wall
                                {
                                    cout << "Z";
                                }
                                else //front-left is a floor
                                {
                                    cout << "z";
                                }
                            }
                            else if (((j<15 && DFHack::isRampTerrain(block.tiletypes[i][j+1]))||(j==15 && hasfront && DFHack::isRampTerrain(frontblock.tiletypes[i][0]))) //front is a ramp
                            && ((i<15 && DFHack::isRampTerrain(block.tiletypes[i+1][j]))||(i==15 && hasright && DFHack::isRampTerrain(rightblock.tiletypes[0][j])))) //right is a ramp
                            {
                                if ((i<15 && j<15 && DFHack::isWallTerrain(block.tiletypes[i+1][j+1]))||(i==15 && j<15 && hasright && DFHack::isWallTerrain(rightblock.tiletypes[0][j+1]))
                                || (i<15 && j==15 && hasfront && DFHack::isWallTerrain(frontblock.tiletypes[i+1][0])) || (i==15 && j==15 && hasfrontright && DFHack::isWallTerrain(frontrightblock.tiletypes[0][0]))) //front-right is a wall
                                {
                                    cout << "C";
                                }
                                else//front-right is a floor
                                {
                                    cout << "c";
                                }
                            }
                            else if (((j>0 && DFHack::isWallTerrain(block.tiletypes[i][j-1]))||(j==0 && hasback && DFHack::isWallTerrain(backblock.tiletypes[i][15]))) //back is a wall
                            && ((j<15 && !DFHack::isWallTerrain(block.tiletypes[i][j+1]))||(j==15 && hasfront && !DFHack::isWallTerrain(frontblock.tiletypes[i][0])))) //front isn't
                            {
                                cout << "W";
                                uramps++;
                                bvuramps->push_back(Vec3(ty+i+1,16-j-tx,tz+1));
                                bvuramps->push_back(Vec3(ty+i,16-j-tx,tz+1));
                                bvuramps->push_back(Vec3(ty+i,15-j-tx,tz));
                                bvuramps->push_back(Vec3(ty+i+1,15-j-tx,tz));
                                bvuramps->push_back(Vec3(ty+i+1,16-j-tx,tz));
                                bvuramps->push_back(Vec3(ty+i,16-j-tx,tz));
                            }
                            else if (((j<15 && DFHack::isWallTerrain(block.tiletypes[i][j+1]))||(j==15 && hasfront && DFHack::isWallTerrain(frontblock.tiletypes[i][0]))) //front is a wall
                            && ((j>0 && !DFHack::isWallTerrain(block.tiletypes[i][j-1]))||(j==0 && hasback && !DFHack::isWallTerrain(frontblock.tiletypes[i][15])))) //back isn't
                            {
                                cout << "X";
                                framps++;
                                bvframps->push_back(Vec3(ty+i,15-j-tx,tz+1));
                                bvframps->push_back(Vec3(ty+i+1,15-j-tx,tz+1));
                                bvframps->push_back(Vec3(ty+i+1,16-j-tx,tz));
                                bvframps->push_back(Vec3(ty+i,16-j-tx,tz));
                                bvframps->push_back(Vec3(ty+i,15-j-tx,tz));
                                bvframps->push_back(Vec3(ty+i+1,15-j-tx,tz));
                            }
                            else if (((i>0 && DFHack::isWallTerrain(block.tiletypes[i-1][j]))||(i==0 && hasleft && DFHack::isWallTerrain(leftblock.tiletypes[15][j]))) //left is a wall
                            && ((i<15 && !DFHack::isWallTerrain(block.tiletypes[i+1][j]))||(i==15 && hasright && !DFHack::isWallTerrain(rightblock.tiletypes[0][j])))) //right isn't
                            {
                                cout << "A";
                            }
                            else if (((i<15 && DFHack::isWallTerrain(block.tiletypes[i+1][j]))||(i==15 && hasright && DFHack::isWallTerrain(rightblock.tiletypes[0][j]))) //right is a wall
                            && ((i>0 && !DFHack::isWallTerrain(block.tiletypes[i-1][j]))||(i==0 && hasleft && !DFHack::isWallTerrain(leftblock.tiletypes[15][j])))) //left isn't
                            {
                                cout << "D";
                            }
                            else //stumped...
                            {
                                cout << "?";
                            }

                        }
                        else
                        {
                            if (DFHack::isWallTerrain(block.tiletypes[i][j])) cout << "O";
                            else if (DFHack::isFloorTerrain(block.tiletypes[i][j])) cout << "+";
                            else if (DFHack::isStairTerrain(block.tiletypes[i][j])) cout << "X";
                            else if (DFHack::isOpenTerrain(block.tiletypes[i][j])) cout << " ";
                        }
                    }
                   cout << endl;
                }

                for (int i = 0; i < 16; i++)
                {
                    for (int j = 0; j < 16; j++)
                    {
                        if (!DFHack::isWallTerrain(block.tiletypes[i][j]) && ((i>0 && DFHack::isWallTerrain(block.tiletypes[i-1][j])) || (hasleft && i==0 && DFHack::isWallTerrain(leftblock.tiletypes[15][j]))))
                        {
                            if (!wallAStarted)
                            {
                                bvl->push_back(Vec3(ty+i,16-j-tx,tz+1));
                                bvl->push_back(Vec3(ty+i,16-j-tx,tz));
                                lwalls++;
                                wallAStarted = true;
                            }
                            if (wallAStarted && j==15)
                            {
                                bvl->push_back(Vec3(ty+i,15-j-tx,tz));
                                bvl->push_back(Vec3(ty+i,15-j-tx,tz+1));
                                wallAStarted = false;
                            }

                        }
                        else if (wallAStarted)
                        {
                            bvl->push_back(Vec3(ty+i,16-j-tx,tz));
                            bvl->push_back(Vec3(ty+i,16-j-tx,tz+1));
                            wallAStarted = false;
                        }

                        if (!DFHack::isWallTerrain(block.tiletypes[i][j]) && ((i<15 && DFHack::isWallTerrain(block.tiletypes[i+1][j])) || (hasright && i==15 && DFHack::isWallTerrain(rightblock.tiletypes[0][j]))))
                        {
                            if (!wallBStarted)
                            {
                                bvr->push_back(Vec3(ty+i+1,16-j-tx,tz+1));
                                bvr->push_back(Vec3(ty+i+1,16-j-tx,tz));
                                rwalls++;
                                wallBStarted = true;
                            }
                            if (wallBStarted && j==15)
                            {
                                bvr->push_back(Vec3(ty+i+1,15-j-tx,tz));
                                bvr->push_back(Vec3(ty+i+1,15-j-tx,tz+1));
                                wallBStarted = false;
                            }

                        }
                        else if (wallBStarted)
                        {
                            bvr->push_back(Vec3(ty+i+1,16-j-tx,tz));
                            bvr->push_back(Vec3(ty+i+1,16-j-tx,tz+1));
                            wallBStarted = false;
                        }
                    }
                }

                bvu->insert(bvu->end(),bvd->begin(),bvd->end());
                bvd->clear();
                bvu->insert(bvu->end(),bvf->begin(),bvf->end());
                bvf->clear();
                bvu->insert(bvu->end(),bvr->begin(),bvr->end());
                bvr->clear();
                bvu->insert(bvu->end(),bvl->begin(),bvl->end());
                bvl->clear();

                bvulramps->insert(bvulramps->end(),bvurramps->begin(),bvurramps->end());
                bvurramps->clear();
                //bvu->insert(bvu->end(),bvulramps->begin(),bvulramps->end());
                bvuramps->insert(bvuramps->end(),bvframps->begin(),bvframps->end());
                bvu->insert(bvu->end(),bvuramps->begin(),bvuramps->end());
                bg->setVertexArray(bvu);
                for (int i = 0; i < 4*uwalls; i++) normals->push_back(Vec3(0,-1,0));
                for (int i = 0; i < 4*dwalls; i++) normals->push_back(Vec3(0,1,0));
                for (int i = 0; i < 4*floors; i++) normals->push_back(Vec3(0,0,1));
                for (int i = 0; i < 4*rwalls; i++) normals->push_back(Vec3(-1,0,0));
                for (int i = 0; i < 4*lwalls; i++) normals->push_back(Vec3(1,0,0));
                /*for (int i = 0; i < ulramps; i++)
                {
                    normals->push_back(Vec3(.57735,-.57735,.57735));
                    normals->push_back(Vec3(.57735,-.57735,.57735));
                    normals->push_back(Vec3(0,-.70711,.70711));
                    normals->push_back(Vec3(.70711,0,.70711));
                }
                for (int i = 0; i < urramps; i++)
                {
                    normals->push_back(Vec3(-.57735,-.57735,.57735));
                    normals->push_back(Vec3(-.57735,-.57735,.57735));
                    normals->push_back(Vec3(-.70711,0,.70711));
                    normals->push_back(Vec3(0,.70711,.70711));
                }*/
                for (int i=0; i < uramps; i++)
                {
                    normals->push_back(Vec3(0,-1,1));
                    normals->push_back(Vec3(0,-1,1));
                    normals->push_back(Vec3(0,-1,1));
                    normals->push_back(Vec3(0,-1,1));
                    normals->push_back(Vec3(-1,0,0));
                    normals->push_back(Vec3(1,0,0));
                }
                for (int i=0; i < uramps; i++)
                {
                    normals->push_back(Vec3(0,1,1));
                    normals->push_back(Vec3(0,1,1));
                    normals->push_back(Vec3(0,1,1));
                    normals->push_back(Vec3(0,1,1));
                    normals->push_back(Vec3(1,0,0));
                    normals->push_back(Vec3(-1,0,0));
                }
                DrawElementsUInt* face;
                for (int i = 3; i < bvu->size()-bvuramps->size(); i+=4)
                {
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    face->push_back(i);
                    face->push_back(i-1);
                    face->push_back(i-2);
                    face->push_back(i-3);
                    bg->addPrimitiveSet(face);
                }
                for (int i=bvu->size()-bvuramps->size(); i<bvu->size(); i+=6)
                {
                    /*face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                    face->push_back(i);
                    face->push_back(i+1);
                    face->push_back(i+3);
                    bg->addPrimitiveSet(face);
                    face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                    face->push_back(i);
                    face->push_back(i+2);
                    face->push_back(i+1);
                    bg->addPrimitiveSet(face);*/
                    face = new DrawElementsUInt(PrimitiveSet::QUADS,0);
                    face->push_back(i);
                    face->push_back(i+1);
                    face->push_back(i+2);
                    face->push_back(i+3);
                    bg->addPrimitiveSet(face);
                    face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                    face->push_back(i+1);
                    face->push_back(i+5);
                    face->push_back(i+2);
                    bg->addPrimitiveSet(face);
                    face = new DrawElementsUInt(PrimitiveSet::TRIANGLES,0);
                    face->push_back(i);
                    face->push_back(i+3);
                    face->push_back(i+4);
                    bg->addPrimitiveSet(face);
                }

                if (normals->size() > 0)
                {
                   bg->setNormalArray(normals);
                   bg->setNormalBinding(Geometry::BIND_PER_VERTEX);
                }

               // cout << bg->getVertexArray()->getNumElements() << " vs. " << normals->size() << endl;
                //osgUtil::SmoothingVisitor sv;
                //blockGeode->accept(sv);
                //tri.stripify(*bg);
                //blockGeode->accept(tri);
            }
        }
    }


    //viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
    viewer.setSceneData(root);
    viewer.setUpViewInWindow(20, 20, 1044, 788);
    viewer.realize();
    viewer.setCameraManipulator(new DwarfManipulator());
    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);
    for (osgViewer::Viewer::Windows::iterator itr = windows.begin(); itr!=windows.end(); itr++) (*itr)->useCursor(false);
    viewer.run();
    DF->Detach();
}

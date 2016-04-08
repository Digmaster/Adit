#include "ChunkManager.h"

#include <list>
#include <algorithm>
#include <string>
#include <iostream>

#include <PolyVoxCore\ConstVolumeProxy.h>
#include <PolyVoxCore\SurfaceMesh.h>
#include <PolyVoxCore\CubicSurfaceExtractorWithNormals.h>

#include <osg/Geode>

#include "BlockGrid.h"
#include "OSGRenderer.h"


ChunkManager::ChunkManager(BlockGrid* container):_gridContainer(container), _center(-100,-100,-100)
{
	_visibility = 4;
}


ChunkManager::~ChunkManager()
{
}

void ChunkManager::updateChunks()
{

	//rebuild one dirty chunk at a time
	if (_dirtyChunks.size() > 0)
	{
		_dirtyChunks.back()->rebuild(_gridContainer);
		_dirtyChunks.pop_back();
	}
}

Chunk& ChunkManager::getChunk(Coords coords)
{
	/*chunkMap_iterator it = _chunkMap.find(coords);
	if (it != _chunkMap.end())
		return *(it->second);
	else*/
		return Chunk(coords, _gridContainer->getBaseNode());
	//TODO: Better "no chunk" logic
}

void ChunkManager::rebuildChunks()
{
	for (chunkMap_type::iterator it = _chunkMap.begin(); it != _chunkMap.end(); it++)
	{
		delete it->second;
	}
	_chunkMap.clear();

	for (dirtyChunks_type::iterator it = _dirtyChunks.begin(); it != _dirtyChunks.end(); it++)
	{
		
	}
	_dirtyChunks.clear();
	setCenterChunk(_center, true);
}

void ChunkManager::setCenterChunk(Coords center, bool force)
{
	if (center == _center && !force) return;
	_center = center;
	//Delete out-of-range chunks
	for (chunkMap_type::iterator it = _chunkMap.begin(); it != _chunkMap.end(); /* No increment */)
	{
		if (it->first.x() > center.x() + _visibility || it->first.x() < center.x() - _visibility || it->first.y() > center.y() + _visibility || it->first.y() < center.y() - _visibility)
		{
			//if (it->first.z() > center.z() + _visibility && it->first.z() < center.z() - _visibility)
			{
				delete it->second;
				it = _chunkMap.erase(it); //returns next element
			}
		}
		else
		{
			it++;
		}
	}

	//load missing chunks
	for (int x = center.x() - _visibility; x <= center.x() + _visibility; x++)
	{
		for (int y = center.y() - _visibility; y <= center.y() + _visibility; y++)
		{
			//for (int z = center.z() - _visibility; z <= center.z() + _visibility && z <= BlockGrid::gridHeight/Chunk::chunkHeight; z++)
			//for (int z = 0; z <= BlockGrid::gridHeight / Chunk::chunkHeight; z++)
			{
				int z = 0;
				if (z < 0) z = 0; //ensure we don't go too low
				Coords coords(x, y, z);
				chunkMap_type::iterator it = _chunkMap.find(coords);
				if (it == _chunkMap.end())
				{
					Chunk* chunk = new Chunk(coords, _gridContainer->getBaseNode());
					_chunkMap.emplace(coords, chunk);
					_dirtyChunks.push_front(_chunkMap.at(coords));
				}
			}
		}
	}
}

void ChunkManager::moveCenterChunk(Coords movement)
{
	movement = movement + _center;
	std::cout << _center << "=>" << movement << std::endl;
	setCenterChunk(movement);
}


void ChunkManager::loadRegion(const PolyVox::ConstVolumeProxy<CompositeBlock::blockDataType>& volume, const PolyVox::Region& reg)
{
	std::cout << "warning loading region: " << reg.getLowerCorner() << " -> " << reg.getUpperCorner() << std::endl;
	LandGenerator gen;
	gen.fillVolume(volume, reg);
}

void ChunkManager::unloadRegion(const PolyVox::ConstVolumeProxy<CompositeBlock::blockDataType>& /*vol*/, const PolyVox::Region& reg)
{
	std::cout << "warning unloading region: " << reg.getLowerCorner() << " -> " << reg.getUpperCorner() << std::endl;
}
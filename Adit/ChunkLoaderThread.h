#pragma once

#include "Coords.h"
#include <queue>
#include <thread>
#include <mutex>
#include <list>

class Chunk;

class ChunkLoaderThread
{
public:
	ChunkLoaderThread();
	~ChunkLoaderThread();

	void requestLoadChunk(Coords location, bool front=false);
	Chunk* getLoadedChunk();

	void removeChunk(Coords location);

	void stopThread() { stopFlag = true; }

	void waitUntilEmpty();

	void operator()();

	ChunkLoaderThread(const ChunkLoaderThread& origin) {} // add this line

	void loop();
	void priorityLoop();
private:
	std::list<Coords> toLoad;
	std::list<Coords> priorityToLoad;
	std::list<Chunk*> loaded;
	std::mutex toLoadMutex;
	std::mutex priorityToLoadMutex;
	std::mutex loadedMutex;
	bool stopFlag;

	std::list<std::thread*> loadThreads;
};

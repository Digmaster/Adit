#include "ChunkLoaderThread.h"

#include <algorithm>

#include <windows.h>

#include "Chunk.h"

#include <iostream>

#include "GameEngine.h"

void ChunkLoaderThread::operator()()
{
	loop();
}

ChunkLoaderThread::ChunkLoaderThread()
{
	stopFlag = false;
	for (int i = 0; i < 1; i++)
	{
		loadThreads.push_back(new std::thread([=] { loop(); }));
	}
	for (int i = 0; i < 1; i++)
	{
		loadThreads.push_back(new std::thread([=] { priorityLoop(); }));
	}
}


ChunkLoaderThread::~ChunkLoaderThread()
{
	stopFlag = true;
	while(!loadThreads.empty())
	{
		loadThreads.front()->join();
		loadThreads.pop_front();
	}
	std::cout << "chunkloading done" << std::endl;
}

void ChunkLoaderThread::requestLoadChunk(Coords location, bool front)
{
	std::lock_guard<std::mutex> mutexlock(loadedMutex);
	if (front)
		priorityToLoad.push_back(location);
	else
		toLoad.push_back(location);
}

void ChunkLoaderThread::waitUntilEmpty()
{
	while (true)
	{
		toLoadMutex.lock();
		if (toLoad.empty())
		{
			toLoadMutex.unlock();
			return;
		}
		toLoadMutex.unlock();
		Sleep(500);
	}
}

Chunk * ChunkLoaderThread::getLoadedChunk()
{
	std::lock_guard<std::mutex> mutexlock(loadedMutex);
	if (loaded.size() == 0)
		return nullptr;
	else
	{
		Chunk* returnChunk = loaded.front();
		loaded.pop_front();
		return returnChunk;
	}
}

void ChunkLoaderThread::removeChunk(Coords location)
{
}

void ChunkLoaderThread::loop()
{
	stopFlag = false;
	while (!stopFlag)
	{
		toLoadMutex.lock();
		if (toLoad.size() == 0)
		{
			toLoadMutex.unlock();
			Sleep(500); //nothing to load? sleep for a bit
		}
		else
		{
			//Retrieve oldest load request
			Coords coords = toLoad.front();
			toLoad.pop_front();
			toLoadMutex.unlock();

			Chunk* chunk = new Chunk(coords);
			chunk->rebuild(GameEngine::inst().getGrid());

			loadedMutex.lock();
			loaded.push_back(chunk);
			loadedMutex.unlock();
		}
	}
}

void ChunkLoaderThread::priorityLoop()
{
	stopFlag = false;
	while (!stopFlag)
	{
		priorityToLoadMutex.lock();
		if (priorityToLoad.size() == 0)
		{
			priorityToLoadMutex.unlock();
			Sleep(500); //nothing to load? sleep for a bit
		}
		else
		{
			//Retrieve oldest load request
			Coords coords = priorityToLoad.front();
			priorityToLoad.pop_front();
			priorityToLoadMutex.unlock();

			Chunk* chunk = new Chunk(coords);
			chunk->rebuild(GameEngine::inst().getGrid());

			loadedMutex.lock();
			loaded.push_front(chunk);
			loadedMutex.unlock();
		}
	}
}
#include "FileManager.h"

#include "ThreadPool.h"

using namespace std;

namespace file_manager
{
	FileManager::FileManager(uint32_t threadsCount) :
		threadPool(make_unique<threading::ThreadPool>(threadsCount))
	{

	}

	FileManager& FileManager::getInstance(uint32_t threadsCount)
	{
		static FileManager instance(threadsCount);

		if (instance.threadPool->getThreadsCount() != threadsCount)
		{
			// TODO: sync

			instance.threadPool->reinit(true, threadsCount);
		}

		return instance;
	}
}

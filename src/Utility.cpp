#include "Utility.h"

#include "FileManager.h"

using namespace std;

namespace file_manager
{
	namespace size_literals
	{
		size_t operator "" _kib(size_t count)
		{
			return count * 1024;
		}

		size_t operator "" _mib(size_t count)
		{
			return count * static_cast<size_t>(pow(1024, 2));
		}

		size_t operator "" _gib(size_t count)
		{
			return count * static_cast<size_t>(pow(1024, 3));
		}
	}

	namespace utility
	{
		size_t pathHash::operator () (const filesystem::path& pathToFile) const noexcept
		{
			return hash<string>()(pathToFile.string());
		}
	}

	namespace _utility
	{
		void addCache(filesystem::path&& pathToFile, string&& data)
		{
			Cache& cache = FileManager::getInstance().getCache();
			unique_lock<mutex> lock(cache.cacheDataMutex);

			if (cache.currentCacheSize + data.size() > cache.cacheSize)
			{
				return;
			}

			cache.cacheData.emplace(move(pathToFile), move(data));
		}
	}
}

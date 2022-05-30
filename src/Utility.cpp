#include "Utility.h"

#include "FileManager.h"

using namespace std;

namespace file_manager
{
	namespace size_literals
	{
		uint64_t operator "" _kib(uint64_t count)
		{
			return count * 1024;
		}

		uint64_t operator "" _mib(uint64_t count)
		{
			return count * static_cast<uint64_t>(pow(1024, 2));
		}

		uint64_t operator "" _gib(uint64_t count)
		{
			return count * static_cast<uint64_t>(pow(1024, 3));
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

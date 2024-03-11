#include "Utility.h"

#include "FileManager.h"

using namespace std;

namespace file_manager
{
	namespace utility
	{
		size_t pathHash::operator () (const filesystem::path& filePath) const noexcept
		{
			return hash<string>()(filePath.string());
		}
	}

	namespace _utility
	{
		void addCache(filesystem::path&& filePath, string&& data)
		{
			Cache& cache = FileManager::getInstance().getCache();
			unique_lock<mutex> lock(cache.cacheDataMutex);

			if (cache.currentCacheSize + data.size() > cache.cacheSize)
			{
				return;
			}

			cache.cacheData.emplace(move(filePath), move(data));
		}
	}
}

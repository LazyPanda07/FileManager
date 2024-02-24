#include "Utility.h"

#include "FileManager.h"

using namespace std;

namespace file_manager
{
	namespace size_literals
	{
		unsigned long long operator "" _kib(unsigned long long count)
		{
			return count * 1024;
		}

		unsigned long long operator "" _mib(unsigned long long count)
		{
			return count * static_cast<unsigned long long>(pow(1024, 2));
		}

		unsigned long long operator "" _gib(unsigned long long count)
		{
			return count * static_cast<unsigned long long>(pow(1024, 3));
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

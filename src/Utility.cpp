#include "Utility.h"

#include "FileManager.h"

namespace file_manager
{
	namespace utility
	{
		size_t PathHash::operator () (const std::filesystem::path& filePath) const noexcept
		{
			return std::hash<std::string>()(filePath.string());
		}
	}

	namespace _utility
	{
		void addCache(std::filesystem::path&& filePath, std::string&& data)
		{
			Cache& cache = FileManager::getInstance().getCache();
			std::lock_guard<std::mutex> lock(cache.cacheDataMutex);

			if (cache.currentCacheSize + data.size() > cache.cacheSize)
			{
				return;
			}

			cache.cacheData.try_emplace(std::move(filePath), std::move(data));
		}
	}
}

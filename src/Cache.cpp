#include "Cache.h"

#include "FileManager.h"
#include "Exceptions/FileDoesNotExistException.h"

namespace std
{
	template<>
	struct less<pair<uint64_t, filesystem::path>>
	{
		constexpr bool operator()(const pair<uint64_t, filesystem::path>& left, const pair<uint64_t, filesystem::path>& right) const
		{
			return left.first > right.first;
		}
	};
}

namespace file_manager
{
	void Cache::updateCache()
	{
		std::priority_queue<std::pair<uint64_t, std::filesystem::path>> paths;
		std::vector<std::pair<uint64_t, const std::filesystem::path*>> notExistingPaths;
		std::lock_guard<std::mutex> cacheDataLock(cacheDataMutex);

		for (const auto& [path, _] : cacheData)
		{
			uint64_t size = std::filesystem::file_size(path);

			if (!std::filesystem::exists(path))
			{
				notExistingPaths.emplace_back(size, &path);

				continue;
			}

			paths.emplace(size, path);
		}

		for (const auto& [size, path] : notExistingPaths)
		{
			currentCacheSize -= size;

			cacheData.erase(*path);
		}

		while (currentCacheSize < cacheSize && paths.size())
		{
			const auto& [size, path] = paths.top();

			currentCacheSize -= size;

			cacheData.erase(path);

			paths.pop();
		}
	}

	Cache& Cache::getCache()
	{
		return FileManager::getInstance().getCache();
	}

	Cache::Cache() :
		cacheSize(0),
		currentCacheSize(0)
	{

	}

	Cache::CacheResultCodes Cache::addCache(const std::filesystem::path& filePath, std::ios_base::openmode mode)
	{
		if (!std::filesystem::exists(filePath))
		{
			return CacheResultCodes::fileDoesNotExist;
		}
		else if (currentCacheSize + std::filesystem::file_size(filePath) > cacheSize)
		{
			return CacheResultCodes::notEnoughCacheSize;
		}

		std::lock_guard<std::mutex> dataLock(cacheDataMutex);

		if (cacheData.contains(filePath))
		{
			return CacheResultCodes::noError;
		}

		std::string data = (std::ostringstream() << std::ifstream(filePath, mode).rdbuf()).str();

		currentCacheSize += data.size();

		cacheData.try_emplace(filePath, move(data));

		return CacheResultCodes::noError;
	}

	Cache::CacheResultCodes Cache::appendCache(const std::filesystem::path& filePath, const std::vector<char>& data)
	{
		return this->appendCache(filePath, data.data());
	}

	Cache::CacheResultCodes Cache::appendCache(const std::filesystem::path& filePath, std::string_view data)
	{
		if (currentCacheSize + data.size() > cacheSize)
		{
			return CacheResultCodes::notEnoughCacheSize;
		}

		std::lock_guard<std::mutex> dataLock(cacheDataMutex);

		if (auto it = cacheData.find(filePath); it != cacheData.end())
		{
			it->second += data;
		}
		else
		{
			cacheData[filePath] = data;
		}

		currentCacheSize += data.size();

		return CacheResultCodes::noError;
	}

	bool Cache::contains(const std::filesystem::path& filePath) const
	{
		std::lock_guard<std::mutex> dataLock(cacheDataMutex);

		return cacheData.contains(filePath);
	}

	void Cache::clear()
	{
		std::lock_guard<std::mutex> dataLock(cacheDataMutex);
		
		currentCacheSize = 0;

		cacheData.clear();
	}

	void Cache::clear(const std::filesystem::path& filePath)
	{
		std::lock_guard<std::mutex> dataLock(cacheDataMutex);
		auto it = cacheData.find(filePath);

		if (it == cacheData.end())
		{
			return;
		}

		currentCacheSize -= it->second.size();

		cacheData.erase(it);
	}

	void Cache::setCacheSize(uint64_t sizeInBytes)
	{
		cacheSize = sizeInBytes;

		if (cacheSize < currentCacheSize)
		{
			this->updateCache();
		}
	}

	const std::string& Cache::getCacheData(const std::filesystem::path& filePath) const
	{
		std::lock_guard<std::mutex> dataLock(cacheDataMutex);
		auto it = cacheData.find(filePath);

		if (it == cacheData.end())
		{
			throw exceptions::FileDoesNotExistException(filePath);
		}

		return it->second;
	}

	uint64_t Cache::getCacheSize() const
	{
		return cacheSize;
	}

	uint64_t Cache::getCurrentCacheSize() const
	{
		return currentCacheSize;
	}

	const std::string& Cache::operator [] (const std::filesystem::path& filePath) const
	{
		return this->getCacheData(filePath);
	}
}

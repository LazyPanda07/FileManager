#include "Cache.h"

#include "FileManager.h"
#include "Exceptions/FileDoesNotExistException.h"

using namespace std;

template<>
struct less<pair<uint64_t, filesystem::path>>
{
	constexpr bool operator()(const pair<uint64_t, filesystem::path>& left, const pair<uint64_t, filesystem::path>& right) const
	{
		return left.first > right.first;
	}
};

namespace file_manager
{
	void Cache::updateCache()
	{
		priority_queue<pair<uint64_t, filesystem::path>> paths;
		vector<pair<uint64_t, const filesystem::path*>> notExistingPaths;
		unique_lock<mutex> cacheDataLock(cacheDataMutex);

		for (const auto& [path, _] : cacheData)
		{
			uint64_t size = filesystem::file_size(path);

			if (!filesystem::exists(path))
			{
				notExistingPaths.emplace_back(size, &path);

				continue;
			}

			paths.emplace(size, path);
		}

		unique_lock<mutex> currentSizeLock(currentSizeMutex);

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

	Cache::Cache() :
		cacheSize(0),
		currentCacheSize(0)
	{

	}

	Cache::CacheResultCodes Cache::addCache(const filesystem::path& pathToFile)
	{
		if (!filesystem::exists(pathToFile))
		{
			return CacheResultCodes::fileDoesNotExist;
		}
		else if (currentCacheSize + filesystem::file_size(pathToFile) > cacheSize)
		{
			return CacheResultCodes::notEnoughCacheSize;
		}

		unique_lock<mutex> lock(cacheDataMutex);

		if (cacheData.contains(pathToFile))
		{
			return CacheResultCodes::noError;
		}

		string data = (ostringstream() << ifstream(pathToFile).rdbuf()).str();

		{
			unique_lock<mutex> lock(currentSizeMutex);

			currentCacheSize += data.size();
		}

		cacheData.emplace(pathToFile, move(data));

		return CacheResultCodes::noError;
	}

	bool Cache::contains(const filesystem::path& pathToFile) const
	{
		return cacheData.contains(pathToFile);
	}

	void Cache::clear()
	{
		currentCacheSize = 0;

		cacheData.clear();
	}

	void Cache::clear(const filesystem::path& pathToFile)
	{
		auto it = cacheData.find(pathToFile);

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

	const string& Cache::getCacheData(const filesystem::path& pathToFile) const
	{
		auto it = cacheData.find(pathToFile);

		if (it == cacheData.end())
		{
			throw exceptions::FileDoesNotExistException(pathToFile);
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
}

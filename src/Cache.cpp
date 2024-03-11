#include "Cache.h"

#include "FileManager.h"
#include "Exceptions/FileDoesNotExistException.h"

using namespace std;

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

	Cache::CacheResultCodes Cache::addCache(const filesystem::path& filePath)
	{
		if (!filesystem::exists(filePath))
		{
			return CacheResultCodes::fileDoesNotExist;
		}
		else if (currentCacheSize + filesystem::file_size(filePath) > cacheSize)
		{
			return CacheResultCodes::notEnoughCacheSize;
		}

		unique_lock<mutex> dataLock(cacheDataMutex);

		if (cacheData.contains(filePath))
		{
			return CacheResultCodes::noError;
		}

		string data = (ostringstream() << ifstream(filePath).rdbuf()).str();

		currentCacheSize += data.size();

		cacheData.emplace(filePath, move(data));

		return CacheResultCodes::noError;
	}

	Cache::CacheResultCodes Cache::appendCache(const filesystem::path& filePath, const vector<char>& data)
	{
		return this->appendCache(filePath, data.data());
	}

	Cache::CacheResultCodes Cache::appendCache(const filesystem::path& filePath, const string_view& data)
	{
		if (currentCacheSize + data.size() > cacheSize)
		{
			return CacheResultCodes::notEnoughCacheSize;
		}

		unique_lock<mutex> dataLock(cacheDataMutex);

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

	bool Cache::contains(const filesystem::path& filePath) const
	{
		unique_lock<mutex> dataLock(cacheDataMutex);

		return cacheData.contains(filePath);
	}

	void Cache::clear()
	{
		unique_lock<mutex> dataLock(cacheDataMutex);
		
		currentCacheSize = 0;

		cacheData.clear();
	}

	void Cache::clear(const filesystem::path& filePath)
	{
		unique_lock<mutex> dataLock(cacheDataMutex);
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

	const string& Cache::getCacheData(const filesystem::path& filePath) const
	{
		unique_lock<mutex> dataLock(cacheDataMutex);
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

	const string& Cache::operator [] (const filesystem::path& filePath) const
	{
		return this->getCacheData(filePath);
	}
}

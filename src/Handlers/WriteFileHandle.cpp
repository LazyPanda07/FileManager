#include "Handlers/WriteFileHandle.h"

#include "FileManager.h"

namespace file_manager
{
	bool WriteFileHandle::CachingBuffer::increaseCacheData()
	{
		if (!isCachingAvailable)
		{
			return false;
		}

		std::string_view availableCacheData(pbase(), pptr());

		if (availableCacheData.size() + cache.getCurrentCacheSize() > cache.getCacheSize())
		{
			isCachingAvailable = false;

			_utility::changeCurrentCacheSize<std::minus>(cacheData.size());

			cacheData.clear();

			return false;
		}

		_utility::changeCurrentCacheSize<std::plus>(availableCacheData.size());

		cacheData += availableCacheData;

		return true;
	}

	WriteFileHandle::CachingBuffer::CachingBuffer(Cache& cache, const std::filesystem::path& filePath, std::ios_base::openmode mode) :
		cache(cache),
		filePath(filePath),
		isCachingAvailable(cache.getCacheSize())
	{
		open(filePath, mode);
	}

	int WriteFileHandle::CachingBuffer::sync()
	{
		this->increaseCacheData();

		return std::filebuf::sync();
	}

	WriteFileHandle::CachingBuffer::~CachingBuffer()
	{
		if (!this->increaseCacheData())
		{
			return;
		}

		_utility::addCache(std::move(filePath), std::move(cacheData));
	}

	WriteFileHandle::WriteFileHandle(const std::filesystem::path& filePath, std::ios_base::openmode mode) :
		FileHandle(filePath, mode | std::ios_base::out)
	{

	}

	void WriteFileHandle::write(const std::string& data)
	{
		file.write(data.data(), data.size()).flush();
	}

	std::ostream& WriteFileHandle::getStream()
	{
		return file.write(nullptr, 0);
	}

	WriteFileHandle::~WriteFileHandle()
	{
		if (isNotifyOnDestruction)
		{
			FileManager::getInstance().completeWriteRequest(filePath);
		}
	}
}

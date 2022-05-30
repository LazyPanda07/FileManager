#include "WriteFileHandle.h"

#include "FileManager.h"

using namespace std;

namespace file_manager
{
	bool WriteFileHandle::CachingBuffer::increaseCacheData()
	{
		if (!isCachingAvailable)
		{
			return false;
		}

		string_view availableCacheData(pbase(), pptr());

		if (availableCacheData.size() + cache.getCurrentCacheSize() > cache.getCacheSize())
		{
			isCachingAvailable = false;

			_utility::changeCurrentCacheSize<minus>(cacheData.size());

			cacheData.clear();

			return false;
		}

		_utility::changeCurrentCacheSize<plus>(availableCacheData.size());

		cacheData += availableCacheData;

		return true;
	}

	WriteFileHandle::CachingBuffer::CachingBuffer(Cache& cache, const filesystem::path& pathToFile, ios_base::openmode mode) :
		cache(cache),
		pathToFile(pathToFile),
		isCachingAvailable(cache.getCacheSize())
	{
		open(pathToFile, mode);
	}

	int WriteFileHandle::CachingBuffer::sync()
	{
		this->increaseCacheData();

		return filebuf::sync();
	}

	WriteFileHandle::CachingBuffer::~CachingBuffer()
	{
		if (!this->increaseCacheData())
		{
			return;
		}

		_utility::addCache(move(pathToFile), move(cacheData));
	}

	WriteFileHandle::WriteFileHandle(const filesystem::path& pathToFile, ios_base::openmode mode) :
		FileHandle(pathToFile, mode | ios_base::out)
	{

	}

	void WriteFileHandle::write(const string& data)
	{
		file.write(data.data(), data.size());
	}

	ostream& WriteFileHandle::getStream()
	{
		return file.write(nullptr, 0);
	}

	WriteFileHandle::~WriteFileHandle()
	{
		if (isNotifyOnDestruction)
		{
			FileManager::getInstance().completeWriteRequest(pathToFile);
		}
	}
}

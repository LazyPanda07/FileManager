#include "ReadFileHandle.h"

#include "FileManager.h"
#include "Exceptions/FileDoesNotExistException.h"

using namespace std;

namespace file_manager
{
	ReadFileHandle::ReadOnlyBuffer::ReadOnlyBuffer(string_view view)
	{
		char* data = const_cast<char*>(view.data());

		setg(data, data, data + view.size());
	}

	ReadFileHandle::ReadFileHandle(const filesystem::path& filePath, ios_base::openmode mode) :
		FileHandle(filePath, mode | ios_base::in)
	{
		Cache& cache = FileManager::getInstance().getCache();

		if (cache.contains(filePath))
		{
			buffer = make_unique<ReadOnlyBuffer>(cache.getCacheData(filePath));

			static_cast<iostream&>(file).rdbuf(buffer.get());
		}
	}

	const string& ReadFileHandle::readAllData()
	{
		Cache& cache = FileManager::getInstance().getCache();

		switch (cache.addCache(filePath, mode))
		{
		case Cache::CacheResultCodes::noError:
			return cache.getCacheData(filePath);

		case Cache::CacheResultCodes::fileDoesNotExist:
			throw exceptions::FileDoesNotExistException(filePath);

		case Cache::CacheResultCodes::notEnoughCacheSize:
			data = (ostringstream() << file.rdbuf()).str();
		}

		return data;
	}

	streamsize ReadFileHandle::readSome(string& outData, streamsize count, bool shrinkOutData, bool resizeOutData)
	{
		if (resizeOutData && outData.size() != count)
		{
			outData.resize(count);
		}

		streamsize result = file.read(outData.data(), count).gcount();

		if (shrinkOutData && outData.size() != result)
		{
			outData.resize(result);
		}

		return result;
	}

	istream& ReadFileHandle::getStream()
	{
		return file.read(nullptr, 0);
	}

	ReadFileHandle::~ReadFileHandle()
	{
		if (isNotifyOnDestruction)
		{
			FileManager::getInstance().decreaseReadRequests(filePath);
		}
	}
}

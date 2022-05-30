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

	ReadFileHandle::ReadFileHandle(const filesystem::path& pathToFile, ios_base::openmode mode) :
		FileHandle(pathToFile, mode | ios_base::in)
	{
		Cache& cache = FileManager::getInstance().getCache();

		if (cache.contains(pathToFile))
		{
			buffer = make_unique<ReadOnlyBuffer>(cache.getCacheData(pathToFile));

			file.set_rdbuf(buffer.get());
		}
	}

	const string& ReadFileHandle::readAllData()
	{
		Cache& cache = FileManager::getInstance().getCache();

		switch (cache.addCache(pathToFile))
		{
		case Cache::CacheResultCodes::noError:
			return cache.getCacheData(pathToFile);

		case Cache::CacheResultCodes::fileDoesNotExist:
			throw exceptions::FileDoesNotExistException(pathToFile);

		case Cache::CacheResultCodes::notEnoughCacheSize:
			data = (ostringstream() << file.rdbuf()).str();
		}

		return data;
	}

	streamsize ReadFileHandle::readSome(string& outData, streamsize count, bool resizeOutData)
	{
		if (resizeOutData && outData.size() != count)
		{
			outData.resize(count);
		}

		streamsize result = file.readsome(outData.data(), count);

		if (resizeOutData && outData.size() != result)
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
			FileManager::getInstance().decreaseReadRequests(pathToFile);
		}
	}
}

#include "Handlers/ReadFileHandle.h"

#include "FileManager.h"
#include "Exceptions/FileDoesNotExistException.h"

namespace file_manager
{
	ReadFileHandle::ReadOnlyBuffer::ReadOnlyBuffer(std::string_view view)
	{
		char* data = const_cast<char*>(view.data());

		setg(data, data, data + view.size());
	}

	ReadFileHandle::ReadFileHandle(const std::filesystem::path& filePath, std::ios_base::openmode mode) :
		FileHandle(filePath, mode | std::ios_base::in)
	{
		Cache& cache = FileManager::getInstance().getCache();

		if (cache.contains(filePath))
		{
			buffer = make_unique<ReadOnlyBuffer>(cache.getCacheData(filePath));

			static_cast<std::iostream& > (file).rdbuf(buffer.get());
		}
	}

	const std::string& ReadFileHandle::readAllData()
	{
		Cache& cache = FileManager::getInstance().getCache();

		switch (cache.addCache(filePath, mode))
		{
		case Cache::CacheResultCodes::noError:
			return cache.getCacheData(filePath);

		case Cache::CacheResultCodes::fileDoesNotExist:
			throw exceptions::FileDoesNotExistException(filePath);

		case Cache::CacheResultCodes::notEnoughCacheSize:
			data = (std::ostringstream() << file.rdbuf()).str();
		}

		return data;
	}

	std::streamsize ReadFileHandle::readSome(std::string& outData, std::streamsize count, bool shrinkOutData, bool resizeOutData)
	{
		if (resizeOutData && outData.size() != count)
		{
			outData.resize(count);
		}

		std::streamsize result = file.read(outData.data(), count).gcount();

		if (shrinkOutData && outData.size() != result)
		{
			outData.resize(result);
		}

		return result;
	}

	std::istream& ReadFileHandle::getStream()
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

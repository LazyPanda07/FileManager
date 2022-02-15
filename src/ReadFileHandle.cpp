#include "FileManager.h"

using namespace std;

namespace file_manager
{
	ReadFileHandle::ReadFileHandle(const filesystem::path& pathToFile, ios_base::openmode mode) :
		FileHandle(pathToFile, mode | ios_base::in)
	{
		
	}

	string ReadFileHandle::readAllData()
	{
		return (ostringstream() << file.rdbuf()).str();
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

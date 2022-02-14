#include "FileManager.h"

using namespace std;

namespace file_manager
{
	WriteFileHandle::WriteFileHandle(const filesystem::path& pathToFile, ios_base::openmode mode) :
		FileHandle(pathToFile, mode | ios_base::out)
	{
		FileManager::getInstance().changeIsWriteRequest(pathToFile, true);
	}

	void WriteFileHandle::write(const std::string& data)
	{
		file.write(data.data(), data.size());
	}

	WriteFileHandle::~WriteFileHandle()
	{
		if (isNotifyOnDestruction)
		{
			FileManager::getInstance().changeIsWriteRequest(pathToFile, false);
		}
	}
}

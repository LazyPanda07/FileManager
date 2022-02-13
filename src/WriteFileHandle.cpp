#include "FileManager.h"

using namespace std;

namespace file_manager
{
	WriteFileHandle::WriteFileHandle(const filesystem::path& pathToFile) :
		FileHandle(pathToFile, ios_base::out)
	{
		FileManager::getInstance().changeIsWriteRequest(pathToFile, true);
	}

	WriteFileHandle::~WriteFileHandle()
	{
		if (isNotifyOnDestruction)
		{
			FileManager::getInstance().changeIsWriteRequest(pathToFile, false);
		}
	}
}

#include "FileManager.h"

using namespace std;

namespace file_manager
{
	ReadFileHandle::ReadFileHandle(const filesystem::path& pathToFile) :
		FileHandle(pathToFile, ios_base::in)
	{
		FileManager::getInstance().changeReadRequests(pathToFile, 1);
	}

	string ReadFileHandle::readAllFile()
	{
		return (ostringstream() << file.rdbuf()).str();
	}

	ReadFileHandle::~ReadFileHandle()
	{
		if (isNotifyOnDestruction)
		{
			FileManager::getInstance().changeReadRequests(pathToFile, -1);
		}
	}
}

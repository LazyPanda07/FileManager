#include "FileManager.h"

using namespace std;

namespace file_manager
{
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
		// TODO: create caching ostream

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

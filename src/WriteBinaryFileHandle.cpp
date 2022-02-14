#include "FileManager.h"

using namespace std;

namespace file_manager
{
	FileManager::WriteBinaryFileHandle::WriteBinaryFileHandle(const filesystem::path& pathToFile, ios_base::openmode mode) :
		WriteFileHandle(pathToFile, mode | ios_base::binary)
	{

	}
}

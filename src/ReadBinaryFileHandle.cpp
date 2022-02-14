#include "FileManager.h"

using namespace std;

namespace file_manager
{
	FileManager::ReadBinaryFileHandle::ReadBinaryFileHandle(const filesystem::path& pathToFile) :
		ReadFileHandle(pathToFile, ios_base::binary)
	{

	}
}

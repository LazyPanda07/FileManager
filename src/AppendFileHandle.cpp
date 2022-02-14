#include "FileManager.h"

using namespace std;

namespace file_manager
{
	FileManager::AppendFileHandle::AppendFileHandle(const filesystem::path& pathToFile) :
		WriteFileHandle(pathToFile, ios_base::app)
	{

	}
}

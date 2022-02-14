#include "FileManager.h"

using namespace std;

namespace file_manager
{
	FileManager::AppendBinaryFileHandle::AppendBinaryFileHandle(const filesystem::path& pathToFile) :
		WriteBinaryFileHandle(pathToFile, ios_base::app)
	{

	}
}

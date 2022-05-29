#include "WriteBinaryFileHandle.h"

using namespace std;

namespace file_manager
{
	WriteBinaryFileHandle::WriteBinaryFileHandle(const filesystem::path& pathToFile, ios_base::openmode mode) :
		WriteFileHandle(pathToFile, mode | ios_base::binary)
	{

	}
}

#include "WriteBinaryFileHandle.h"

using namespace std;

namespace file_manager
{
	WriteBinaryFileHandle::WriteBinaryFileHandle(const filesystem::path& filePath, ios_base::openmode mode) :
		WriteFileHandle(filePath, mode | ios_base::binary)
	{

	}
}

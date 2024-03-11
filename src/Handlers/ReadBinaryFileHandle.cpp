#include "ReadBinaryFileHandle.h"

using namespace std;

namespace file_manager
{
	ReadBinaryFileHandle::ReadBinaryFileHandle(const filesystem::path& filePath) :
		ReadFileHandle(filePath, ios_base::binary)
	{

	}
}

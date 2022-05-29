#include "ReadBinaryFileHandle.h"

using namespace std;

namespace file_manager
{
	ReadBinaryFileHandle::ReadBinaryFileHandle(const filesystem::path& pathToFile) :
		ReadFileHandle(pathToFile, ios_base::binary)
	{

	}
}

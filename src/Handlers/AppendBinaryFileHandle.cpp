#include "AppendBinaryFileHandle.h"

using namespace std;

namespace file_manager
{
	AppendBinaryFileHandle::AppendBinaryFileHandle(const filesystem::path& pathToFile) :
		WriteBinaryFileHandle(pathToFile, ios_base::app)
	{

	}
}

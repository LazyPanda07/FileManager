#include "AppendBinaryFileHandle.h"

using namespace std;

namespace file_manager
{
	AppendBinaryFileHandle::AppendBinaryFileHandle(const filesystem::path& filePath) :
		WriteBinaryFileHandle(filePath, ios_base::app)
	{

	}
}

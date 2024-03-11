#include "AppendFileHandle.h"

using namespace std;

namespace file_manager
{
	AppendFileHandle::AppendFileHandle(const filesystem::path& filePath) :
		WriteFileHandle(filePath, ios_base::app)
	{

	}
}

#include "AppendFileHandle.h"

using namespace std;

namespace file_manager
{
	AppendFileHandle::AppendFileHandle(const filesystem::path& pathToFile) :
		WriteFileHandle(pathToFile, ios_base::app)
	{

	}
}

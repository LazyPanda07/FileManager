#include "Handlers/AppendFileHandle.h"

namespace file_manager
{
	AppendFileHandle::AppendFileHandle(const std::filesystem::path& filePath) :
		WriteFileHandle(filePath, std::ios_base::app)
	{

	}
}

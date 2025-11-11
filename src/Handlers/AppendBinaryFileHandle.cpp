#include "Handlers/AppendBinaryFileHandle.h"

namespace file_manager
{
	AppendBinaryFileHandle::AppendBinaryFileHandle(const std::filesystem::path& filePath) :
		WriteBinaryFileHandle(filePath, std::ios_base::app)
	{

	}
}

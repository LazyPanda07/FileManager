#include "Handlers/ReadBinaryFileHandle.h"

namespace file_manager
{
	ReadBinaryFileHandle::ReadBinaryFileHandle(const std::filesystem::path& filePath) :
		ReadFileHandle(filePath, std::ios_base::binary)
	{

	}
}

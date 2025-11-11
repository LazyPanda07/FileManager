#include "Handlers/WriteBinaryFileHandle.h"

namespace file_manager
{
	WriteBinaryFileHandle::WriteBinaryFileHandle(const std::filesystem::path& filePath, std::ios_base::openmode mode) :
		WriteFileHandle(filePath, mode | std::ios_base::binary)
	{

	}
}

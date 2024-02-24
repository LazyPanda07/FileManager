#pragma once

#include "WriteFileHandle.h"

namespace file_manager
{
	class WriteBinaryFileHandle : public WriteFileHandle
	{
	protected:
		WriteBinaryFileHandle(const std::filesystem::path& pathToFile, std::ios_base::openmode mode = std::ios_base::out | std::ios_base::binary);

	public:
		virtual ~WriteBinaryFileHandle() = default;

		friend class FileManager;
	};
}

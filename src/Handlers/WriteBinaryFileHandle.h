#pragma once

#include "WriteFileHandle.h"

namespace file_manager
{
	class WriteBinaryFileHandle : public WriteFileHandle
	{
	protected:
		WriteBinaryFileHandle(const std::filesystem::path& pathToFile, std::ios_base::openmode mode = 0);

	public:
		virtual ~WriteBinaryFileHandle() = default;

		friend class FileManager;
	};
}

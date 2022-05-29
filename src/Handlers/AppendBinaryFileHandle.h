#pragma once

#include "WriteBinaryFileHandle.h"

namespace file_manager
{
	class AppendBinaryFileHandle : public WriteBinaryFileHandle
	{
	private:
		AppendBinaryFileHandle(const std::filesystem::path& pathToFile);

	public:
		~AppendBinaryFileHandle() = default;

		friend class FileManager;
	};
}

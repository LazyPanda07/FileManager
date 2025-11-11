#pragma once

#include "WriteFileHandle.h"

namespace file_manager
{
	class AppendFileHandle : public WriteFileHandle
	{
	private:
		AppendFileHandle(const std::filesystem::path& filePath);

	public:
		~AppendFileHandle() = default;

		friend class FileManager;
	};
}

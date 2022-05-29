#pragma once

#include "WriteFileHandle.h"

namespace file_manager
{
	class AppendFileHandle : public WriteFileHandle
	{
	private:
		AppendFileHandle(const std::filesystem::path& pathToFile);

	public:
		~AppendFileHandle() = default;

		friend class FileManager;
	};
}

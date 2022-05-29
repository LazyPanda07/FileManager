#pragma once

#include "ReadFileHandle.h"

namespace file_manager
{
	class ReadBinaryFileHandle : public ReadFileHandle
	{
	private:
		ReadBinaryFileHandle(const std::filesystem::path& pathToFile);

	public:
		~ReadBinaryFileHandle() = default;

		friend class FileManager;
	};
}

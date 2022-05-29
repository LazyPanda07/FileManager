#pragma once

#include <fstream>

#include "Utility.h"

namespace file_manager
{
	class FILE_MANAGER_API FileHandle
	{
	protected:
		std::filesystem::path pathToFile;
		std::fstream file;
		std::ios_base::openmode mode;
		bool isNotifyOnDestruction;

	protected:
		FileHandle(const std::filesystem::path& pathToFile, std::ios_base::openmode mode);

		FileHandle(FileHandle&& other) noexcept;

		FileHandle& operator = (FileHandle&& other) noexcept;

	public:
		FileHandle(const FileHandle&) = delete;

		FileHandle& operator = (const FileHandle&) = delete;

		uint64_t getFileSize() const;

		const std::filesystem::path& getPathToFile() const;

		std::filesystem::path getFileName() const;

		virtual ~FileHandle();

		friend class FileManager;
	};
}

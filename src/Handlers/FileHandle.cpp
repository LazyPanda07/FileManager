#include "Handlers/FileHandle.h"

#include "FileManager.h"

namespace file_manager
{
	FileHandle::FileHandle(const std::filesystem::path& filePath, std::ios_base::openmode mode) :
		filePath(filePath),
		file(filePath, mode),
		mode(mode),
		isNotifyOnDestruction(true)
	{

	}

	FileHandle::FileHandle(FileHandle&& other) noexcept
	{
		(*this) = std::move(other);
	}

	FileHandle& FileHandle::operator = (FileHandle&& other) noexcept
	{
		filePath = std::move(other.filePath);
		file = move(other.file);
		mode = other.mode;

		isNotifyOnDestruction = other.isNotifyOnDestruction;

		other.isNotifyOnDestruction = !other.isNotifyOnDestruction;

		return *this;
	}

	uint64_t FileHandle::getFileSize() const
	{
		return std::filesystem::file_size(filePath);
	}

	const std::filesystem::path& FileHandle::getPathToFile() const
	{
		return filePath;
	}

	std::filesystem::path FileHandle::getFileName() const
	{
		return filePath.filename();
	}

	FileHandle::~FileHandle()
	{
		if (isNotifyOnDestruction)
		{
			file.close();

			FileManager::getInstance().notify(std::move(filePath));
		}
	}
}

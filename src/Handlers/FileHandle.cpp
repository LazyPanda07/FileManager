#include "FileHandle.h"

#include "FileManager.h"

using namespace std;

namespace file_manager
{
	FileHandle::FileHandle(const filesystem::path& filePath, ios_base::openmode mode) :
		filePath(filePath),
		file(filePath, mode),
		mode(mode),
		isNotifyOnDestruction(true)
	{

	}

	FileHandle::FileHandle(FileHandle&& other) noexcept
	{
		(*this) = move(other);
	}

	FileHandle& FileHandle::operator = (FileHandle&& other) noexcept
	{
		filePath = move(other.filePath);
		file = move(other.file);
		mode = other.mode;

		isNotifyOnDestruction = other.isNotifyOnDestruction;

		other.isNotifyOnDestruction = !other.isNotifyOnDestruction;

		return *this;
	}

	uint64_t FileHandle::getFileSize() const
	{
		return filesystem::file_size(filePath);
	}

	const filesystem::path& FileHandle::getPathToFile() const
	{
		return filePath;
	}

	filesystem::path FileHandle::getFileName() const
	{
		return filePath.filename();
	}

	FileHandle::~FileHandle()
	{
		if (isNotifyOnDestruction)
		{
			file.close();

			FileManager::getInstance().notify(move(filePath), mode);
		}
	}
}

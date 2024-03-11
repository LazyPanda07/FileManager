#include "FileHandle.h"

#include "FileManager.h"

using namespace std;

namespace file_manager
{
	FileHandle::FileHandle(const filesystem::path& pathToFile, ios_base::openmode mode) :
		pathToFile(pathToFile),
		file(pathToFile, mode),
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
		pathToFile = move(other.pathToFile);
		file = move(other.file);
		mode = other.mode;

		isNotifyOnDestruction = other.isNotifyOnDestruction;

		other.isNotifyOnDestruction = !other.isNotifyOnDestruction;

		return *this;
	}

	uint64_t FileHandle::getFileSize() const
	{
		return filesystem::file_size(pathToFile);
	}

	const filesystem::path& FileHandle::getPathToFile() const
	{
		return pathToFile;
	}

	filesystem::path FileHandle::getFileName() const
	{
		return pathToFile.filename();
	}

	FileHandle::~FileHandle()
	{
		if (isNotifyOnDestruction)
		{
			file.close();

			FileManager::getInstance().notify(move(pathToFile), mode);
		}
	}
}

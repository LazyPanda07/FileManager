#include "FileManager.h"

using namespace std;

namespace file_manager
{
	FileManager::FileHandle::FileHandle(const filesystem::path& pathToFile, ios_base::openmode mode) :
		pathToFile(pathToFile),
		file(pathToFile, mode),
		mode(mode),
		isNotifyOnDestruction(true)
	{

	}

	FileManager::FileHandle::FileHandle(FileHandle&& other) noexcept
	{
		(*this) = move(other);
	}

	FileManager::FileHandle& FileManager::FileHandle::operator = (FileHandle&& other) noexcept
	{
		pathToFile = move(other.pathToFile);
		file = move(other.file);
		mode = other.mode;

		isNotifyOnDestruction = true;

		other.isNotifyOnDestruction = false;

		return *this;
	}

	uintmax_t FileManager::FileHandle::getFileSize() const
	{
		return filesystem::file_size(pathToFile);
	}

	const filesystem::path& FileManager::FileHandle::getPathToFile() const
	{
		return pathToFile;
	}

	filesystem::path FileManager::FileHandle::getFileName() const
	{
		return pathToFile.filename();
	}

	FileManager::FileHandle::~FileHandle()
	{
		if (isNotifyOnDestruction)
		{
			file.close();

			FileManager::getInstance().notify(move(pathToFile), mode);
		}
	}
}

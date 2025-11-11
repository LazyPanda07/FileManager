#include "Exceptions/FileDoesNotExistException.h"

#include <format>

namespace file_manager::exceptions
{
	FileDoesNotExistException::FileDoesNotExistException(const std::filesystem::path& filePath) :
		BaseFileManagerException(std::format("File '{}' does not exist", filePath.string()))
	{

	}
}

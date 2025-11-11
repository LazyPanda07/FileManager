#include "Exceptions/NotAFileException.h"

#include <format>

namespace file_manager::exceptions
{
	NotAFileException::NotAFileException(const std::filesystem::path& path) :
		BaseFileManagerException(std::format("Path '{}' does not represent file", path.string()))
	{

	}
}

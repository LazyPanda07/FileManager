#pragma once

#include "BaseFileManagerException.h"

namespace file_manager::exceptions
{
	/// @brief Thrown if path represents not a regular file
	class FILE_MANAGER_API NotAFileException : public BaseFileManagerException
	{
	public:
		NotAFileException(const std::filesystem::path& path);

		~NotAFileException() = default;
	};
}

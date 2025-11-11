#pragma once

#include <stdexcept>
#include <filesystem>

#include "Utility.h"

namespace file_manager::exceptions
{
	/// @brief Base class for all File Manager exceptions
	class FILE_MANAGER_API BaseFileManagerException : public std::runtime_error
	{
	public:
		BaseFileManagerException(const std::string& message);

		virtual ~BaseFileManagerException() = default;
	};
}

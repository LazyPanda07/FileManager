#pragma once

#include "BaseFileManagerException.h"

namespace file_manager
{
	namespace exceptions
	{
		/// @brief Thrown if file does not exist
		class FILE_MANAGER_API FileDoesNotExistException : public BaseFileManagerException
		{
		public:
			FileDoesNotExistException(const std::filesystem::path& filePath);

			~FileDoesNotExistException() = default;
		};
	}
}

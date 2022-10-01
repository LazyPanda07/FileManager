#pragma once

#include <stdexcept>
#include <filesystem>

#ifdef FILE_MANAGER_DLL
#define FILE_MANAGER_API __declspec(dllexport)

#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
#else
#define FILE_MANAGER_API
#endif // FILE_MANAGER_DLL

namespace file_manager
{
	namespace exceptions
	{
		/// @brief Base class for all File Manager exceptions
		class FILE_MANAGER_API BaseFileManagerException : public std::runtime_error
		{
		public:
			BaseFileManagerException(const std::string& message);

			virtual ~BaseFileManagerException() = default;
		};
	}
}

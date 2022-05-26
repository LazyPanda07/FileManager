#pragma once

#include <filesystem>

#ifdef FILE_MANAGER_DLL
#define FILE_MANAGER_API __declspec(dllexport)

#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
#else
#define FILE_MANAGER_API
#endif

namespace file_manager
{
	namespace utility
	{
		struct FILE_MANAGER_API pathHash
		{
			size_t operator () (const std::filesystem::path& pathToFile) const noexcept;
		};
	}
}

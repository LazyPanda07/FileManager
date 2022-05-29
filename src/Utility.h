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
	inline namespace size_literals
	{
		/// @brief Shortcut for declaring KiB (1024 bytes)
		/// @param count Count of KiB
		/// @return Result of converting KiB to bytes
		uint64_t operator "" _kib(uint64_t count);

		/// @brief Shortcut for declaring MiB (1024 KiB)
		/// @param count Count of MiB
		/// @return Result of converting MiB to bytes
		uint64_t operator "" _mib(uint64_t count);

		/// @brief Shortcut for declaring GiB (1024 MiB)
		/// @param count Count of GiB
		/// @return Result of converting GiB to bytes
		uint64_t operator "" _gib(uint64_t count);
	}

	namespace utility
	{
		struct FILE_MANAGER_API pathHash
		{
			size_t operator () (const std::filesystem::path& pathToFile) const noexcept;
		};
	}
}

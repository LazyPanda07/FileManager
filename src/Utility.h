#pragma once

#include <filesystem>

#ifdef FILE_MANAGER_DLL
#ifdef __LINUX__
#define FILE_MANAGER_API __attribute__((visibility("default")))
#else
#define FILE_MANAGER_API __declspec(dllexport)
#endif

#define FILE_MANAGER_API_FUNCTION extern "C" FILE_MANAGER_API

#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
#else
#define FILE_MANAGER_API
#define FILE_MANAGER_API_FUNCTION
#endif

namespace file_manager
{
	inline namespace size_literals
	{
		/// @brief Shortcut for declaring KiB (1024 bytes)
		/// @param count Count of KiB
		/// @return Result of converting KiB to bytes
		FILE_MANAGER_API_FUNCTION size_t operator "" _kib(size_t count);

		/// @brief Shortcut for declaring MiB (1024 KiB)
		/// @param count Count of MiB
		/// @return Result of converting MiB to bytes
		FILE_MANAGER_API_FUNCTION size_t operator "" _mib(size_t count);

		/// @brief Shortcut for declaring GiB (1024 MiB)
		/// @param count Count of GiB
		/// @return Result of converting GiB to bytes
		FILE_MANAGER_API_FUNCTION size_t operator "" _gib(size_t count);
	}

	namespace utility
	{
		/// @brief filesystem::path hash function
		struct FILE_MANAGER_API pathHash
		{
			size_t operator () (const std::filesystem::path& pathToFile) const noexcept;
		};	
	}

	namespace _utility
	{
		template<typename OperationT>
		concept Operation = requires (uint64_t left, uint64_t right)
		{
			{ OperationT()(left, right) } -> std::convertible_to<uint64_t>;
		};

		void addCache(std::filesystem::path&& pathToFile, std::string&& data);

		template<template<typename> typename OperationT> requires _utility::Operation<OperationT<uint64_t>>
		void changeCurrentCacheSize(uint64_t amount);
	}
}

namespace threading
{
	class ThreadPool;
}

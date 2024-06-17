#pragma once

#include <filesystem>
#include <cmath>

#ifdef __LINUX__
#define FILE_MANAGER_API __attribute__((visibility("default")))
#else
#define FILE_MANAGER_API __declspec(dllexport)

#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
#endif

namespace file_manager
{
	inline namespace size_literals
	{
		/// @brief Shortcut for declaring KiB (1024 bytes)
		/// @param count Count of KiB
		/// @return Result of converting KiB to bytes
		inline unsigned long long operator "" _kib(unsigned long long count)
		{
			return count * 1024;
		}

		/// @brief Shortcut for declaring MiB (1024 KiB)
		/// @param count Count of MiB
		/// @return Result of converting MiB to bytes
		inline unsigned long long operator "" _mib(unsigned long long count)
		{
			return count * static_cast<unsigned long long>(std::pow(1024, 2));
		}

		/// @brief Shortcut for declaring GiB (1024 MiB)
		/// @param count Count of GiB
		/// @return Result of converting GiB to bytes
		inline unsigned long long operator "" _gib(unsigned long long count)
		{
			return count * static_cast<unsigned long long>(std::pow(1024, 3));
		}
	}

	namespace utility
	{
		/// @brief filesystem::path hash function
		struct FILE_MANAGER_API pathHash
		{
			size_t operator () (const std::filesystem::path& filePath) const noexcept;
		};
	}

	namespace _utility
	{
		template<typename OperationT>
		concept Operation = requires (uint64_t left, uint64_t right)
		{
			{ OperationT()(left, right) } -> std::convertible_to<uint64_t>;
		};

		void addCache(std::filesystem::path&& filePath, std::string&& data);

		template<template<typename> typename OperationT> requires _utility::Operation<OperationT<uint64_t>>
		void changeCurrentCacheSize(uint64_t amount);
	}
}

namespace threading
{
	class ThreadPool;
}

#pragma once

#include <unordered_map>
#include <map>
#include <mutex>
#include <atomic>
#include <vector>

#include "Utility.h"

namespace file_manager
{
	class FileManager;

	/**
	 * @brief Files cache
	*/
	class FILE_MANAGER_API Cache
	{
	public:
		/// @brief Result of adding cache
		enum class CacheResultCodes
		{
			noError,
			fileDoesNotExist,
			notEnoughCacheSize
		};

	private:
		std::unordered_map<std::filesystem::path, std::string, utility::pathHash> cacheData;
		uint64_t cacheSize;
		std::atomic<uint64_t> currentCacheSize;
		mutable std::mutex cacheDataMutex;

	private:
		void updateCache();

		static Cache& getCache();

	private:
		Cache();

		~Cache() = default;

	public:
		/// @brief Add cache data
		/// @param filePath Path to file
		/// @return Error code from Cache::CacheErrorCodes
		CacheResultCodes addCache(const std::filesystem::path& filePath, std::ios_base::openmode mode);

		/**
		 * @brief Append specific cache
		 * @param filePath Path to file
		 * @param data Cache data
		 * @return Error code from Cache::CacheErrorCodes
		*/
		CacheResultCodes appendCache(const std::filesystem::path& filePath, const std::vector<char>& data);

		/**
		 * @brief Append specific cache
		 * @param filePath Path to file
		 * @param data Cache data
		 * @return Error code from Cache::CacheErrorCodes
		*/
		CacheResultCodes appendCache(const std::filesystem::path& filePath, std::string_view data);

		/// @brief Check if file data is cached
		/// @param filePath Path to file
		/// @return Returns true if file is already cached
		bool contains(const std::filesystem::path& filePath) const;

		/// @brief Clear all cache
		void clear();

		/// @brief Clear cache of specific file
		/// @param filePath Path to file
		void clear(const std::filesystem::path& filePath);

		/// @brief Set cache size
		/// @param sizeInBytes Size in bytes
		void setCacheSize(uint64_t sizeInBytes);

		/// @brief Get cached data
		/// @return Cached data
		/// @exception FileDoesNotExistException
		const std::string& getCacheData(const std::filesystem::path& filePath) const;

		/// @brief Get global cache size
		/// @return Cache size in bytes
		uint64_t getCacheSize() const;

		/// @brief Used cache size
		/// @return Cache size in bytes
		uint64_t getCurrentCacheSize() const;

		/**
		 * @brief Get cached data
		 * @return Cached data
		 * @exception FileDoesNotExistException
		*/
		const std::string& operator [] (const std::filesystem::path& filePath) const;

		friend void _utility::addCache(std::filesystem::path&& filePath, std::string&& data);

		template<template<typename> typename OperationT> requires _utility::Operation<OperationT<uint64_t>>
		friend void _utility::changeCurrentCacheSize(uint64_t amount);

		friend class FileManager;
	};

	namespace _utility
	{
		template<template<typename> typename OperationT> requires _utility::Operation<OperationT<uint64_t>>
		void changeCurrentCacheSize(uint64_t amount)
		{
			Cache& cache = Cache::getCache();

			cache.currentCacheSize = OperationT<uint64_t>()(cache.currentCacheSize, amount);
		}
	}
}

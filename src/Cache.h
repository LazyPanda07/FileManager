#pragma once

#include <unordered_map>
#include <map>

#include "Utility.h"

namespace file_manager
{
	class FileManager;

	class FILE_MANAGER_API Cache
	{
	public:
		enum class CacheResultCodes
		{
			noError,
			fileDoesNotExist,
			notEnoughCacheSize
		};

	private:
		std::unordered_map<std::filesystem::path, std::string, utility::pathHash> cacheData;
		FileManager& manager;
		uint64_t cacheSize;
		uint64_t currentCacheSize;

	private:
		void updateCache();

	private:
		Cache(FileManager& manager);

		~Cache() = default;

	public:
		/// @brief Add cache data
		/// @param pathToFile Path to file
		/// @return Error code from Cache::CacheErrorCodes
		CacheResultCodes addCache(const std::filesystem::path& pathToFile);

		/// @brief Check if file data is cached
		/// @param pathToFile Path to file
		/// @return Returns true if file is already cached
		bool contains(const std::filesystem::path& pathToFile) const;

		/// @brief Clear all cache
		void clear();

		/// @brief Clear cache of specific file
		/// @param pathToFile Path to file
		void clear(const std::filesystem::path& pathToFile);

		/// @brief Set cache size
		/// @param sizeInBytes Size in bytes
		void setCacheSize(uint64_t sizeInBytes);

		/// @brief Get cached data
		/// @return Cached data
		/// @exception FileDoesNotExistException
		const std::string& getCacheData(const std::filesystem::path& pathToFile) const;

		/// @brief Get global cache size
		/// @return Cache size in bytes
		uint64_t getCacheSize() const;

		/// @brief Used cache size
		/// @return Cache size in bytes
		uint64_t getCurrentCacheSize() const;

		friend class FileManager;
	};
}

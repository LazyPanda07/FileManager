#pragma once

#include <functional>

#include "FileHandle.h"

namespace file_manager
{
	/// @brief Provides writing files
	class FILE_MANAGER_API WriteFileHandle : public FileHandle
	{
	private:
		class CachingBuffer : public std::filebuf
		{
		private:
			class Cache& cache;
			std::filesystem::path filePath;
			std::string cacheData;
			bool isCachingAvailable;

		private:
			bool increaseCacheData();

		public:
			CachingBuffer(class Cache& cache, const std::filesystem::path& filePath, std::ios_base::openmode mode);

			int sync() override;

			~CachingBuffer();
		};

	protected:
		WriteFileHandle(const std::filesystem::path& filePath, std::ios_base::openmode mode = std::ios_base::out);

	public:
		/// @brief Write data to file
		/// @param data Data
		void write(const std::string& data);

		/// @brief Get writing stream
		/// @return Output stream
		std::ostream& getStream();

		virtual ~WriteFileHandle();

		friend class FileManager;
	};
}

#pragma once

#include <filesystem>
#include <unordered_map>
#include <map>
#include <thread>
#include <mutex>
#include <fstream>
#include <functional>
#include <variant>
#include <queue>
#include <sstream>

#include "ForwardDeclaration.h"
#include "Cache.h"

#include "Handlers/FileHandle.h"
#include "Handlers/ReadFileHandle.h"
#include "Handlers/WriteFileHandle.h"

namespace file_manager
{
	/// @brief Provides files accessing from multiple threads. Singleton
	class FILE_MANAGER_API FileManager
	{
	private:
		enum class requestType
		{
			read,
			write
		};

		enum class requestFileHandleType
		{
			read,
			write,
			readBinary,
			writeBinary,
			append,
			appendBinary
		};
		
	private:
		struct filePathState
		{
			size_t readRequests;
			bool isWriteRequest;

			filePathState();
		};

		using fileCallback = std::variant<readFileCallback, writeFileCallback>;

		struct requestStruct
		{
			fileCallback callback;
			std::function<void()> onEndCallback;
			requestFileHandleType handleType;

			requestStruct(fileCallback&& callback, const std::function<void()>& onEndCallback, requestFileHandleType handleType);
		};

		friend bool operator == (const requestStruct& request, requestType type);

	private:
		std::unique_ptr<threading::ThreadPool> threadPool;
		std::unordered_map<std::filesystem::path, filePathState, utility::pathHash> files;
		std::unordered_map<std::filesystem::path, std::queue<requestStruct>, utility::pathHash> requests;
		std::recursive_mutex filesMutex;
		std::mutex requestsMutex;
		Cache cache;

	private:
		FileHandle* createHandle(const std::filesystem::path& pathToFile, requestFileHandleType handleType);

		void notify(std::filesystem::path&& pathToFile, std::ios_base::openmode mode);

		void addRequest(const std::filesystem::path& pathToFile, fileCallback&& callback, const std::function<void()>& onEndCallback, requestFileHandleType handleType);

		void processQueue(const std::filesystem::path& pathToFile);

		void decreaseReadRequests(const std::filesystem::path& pathToFile);

		void completeWriteRequest(const std::filesystem::path& pathToFile);

	private:
		FileManager();

		~FileManager() = default;

	public:
		FileManager(const FileManager&) = delete;

		FileManager(FileManager&&) noexcept = delete;

		FileManager& operator = (const FileManager&) = delete;

		FileManager& operator = (FileManager&&) noexcept = delete;

	private:
		void addReadRequest(const std::filesystem::path& pathToFile, const readFileCallback& callback, requestFileHandleType handleType, bool isWait);

		void addWriteRequest(const std::filesystem::path& pathToFile, const writeFileCallback& callback, requestFileHandleType handleType, bool isWait);

	public:
		/// @brief Singleton getter
		/// @return Singleton instance
		static FileManager& getInstance();

		/// @brief Add file to manager
		/// @param pathToFile Path to file
		/// @param isFileAlreadyExist If true and file does not exist FileDoesNotExistException will be thrown. If true and pathToFile contains path to non regular file NotAFileException will be thrown
		/// @exception FileDoesNotExistException 
		/// @exception NotAFileException 
		void addFile(const std::filesystem::path& pathToFile, bool isFileAlreadyExist = true);

		/// @brief Read file in standard mode
		/// @param pathToFile Path to file
		/// @param callback Function that will be called for reading file
		/// @param isWait If true thread will wait till callback end
		/// @exception FileDoesNotExistException 
		/// @exception NotAFileException 
		void readFile(const std::filesystem::path& pathToFile, const readFileCallback& callback, bool isWait = true);

		/// @brief Read file in binary mode
		/// @param pathToFile Path to file
		/// @param callback Function that will be called for reading file
		/// @param isWait If true thread will wait till callback end
		/// @exception FileDoesNotExistException 
		/// @exception NotAFileException 
		void readBinaryFile(const std::filesystem::path& pathToFile, const readFileCallback& callback, bool isWait = true);

		/// @brief Create/Recreate and write file in standard mode
		/// @param pathToFile Path to file
		/// @param callback Function that will be called for writing file
		/// @param isWait If true thread will wait till callback end
		void writeFile(const std::filesystem::path& pathToFile, const writeFileCallback& callback, bool isWait = true);

		/// @brief Create file if it does not exist and write file in standard mode
		/// @param pathToFile Path to file
		/// @param callback Function that will be called for writing file
		/// @param isWait If true thread will wait till callback end
		void appendFile(const std::filesystem::path& pathToFile, const writeFileCallback& callback, bool isWait = true);

		/// @brief Create/Recreate and write file in binary mode
		/// @param pathToFile Path to file
		/// @param callback Function that will be called for writing file
		/// @param isWait If true thread will wait till callback end
		void writeBinaryFile(const std::filesystem::path& pathToFile, const writeFileCallback& callback, bool isWait = true);

		/// @brief Create file if it does not exist and write file in binary mode
		/// @param pathToFile Path to file
		/// @param callback Function that will be called for writing file
		/// @param isWait If true thread will wait till callback end
		void appendBinaryFile(const std::filesystem::path& pathToFile, const writeFileCallback& callback, bool isWait = true);

		/// @brief Cache getter
		/// @return Cache instance
		Cache& getCache();

		/// @brief Cache getter
		/// @return Cache instance
		const Cache& getCache() const;

		friend class FileHandle;
		friend class ReadFileHandle;
		friend class WriteFileHandle;
		friend class Cache;
	};
}

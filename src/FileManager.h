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
#include <future>

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
		using fileCallback = std::variant<std::function<void(std::unique_ptr<ReadFileHandle>&&)>, std::function<void(std::unique_ptr<WriteFileHandle>&&)>>;

		struct requestStruct
		{
			fileCallback callback;
			std::promise<void> requestPromise;
			requestFileHandleType handleType;

			requestStruct(fileCallback&& callback, std::promise<void>&& requestPromise, requestFileHandleType handleType);
		};

		friend bool operator == (const requestStruct& request, requestType type);

	private:
		class FileNode
		{
		private:
			struct FilePathState
			{
				std::atomic_size_t readRequests;
				std::atomic_bool isWriteRequest;

				FilePathState();
			};

		private:
			std::queue<requestStruct> requests;
			std::mutex requestsMutex;

		public:
			FilePathState state;

		public:
			FileNode() = default;

			void addRequest(fileCallback&& callback, std::promise<void>&& requestPromise, requestFileHandleType handleType);

			void processQueue(const std::filesystem::path& filePath);

			~FileNode() = default;
		};

		class NodesContainer
		{
		private:
			std::unordered_map<std::filesystem::path, FileNode*, utility::pathHash> data;
			mutable std::mutex readWriteMutex;

		public:
			NodesContainer() = default;

			void addNode(const std::filesystem::path& filePath);
			
			FileNode* operator [](const std::filesystem::path& filePath) const;

			~NodesContainer();
		};

	private:
		using FileManagerPtr = std::unique_ptr<FileManager, void(*)(FileManager*)>;

		static FileManagerPtr instance;
		inline static std::mutex instanceMutex;

	private:
		Cache cache;
		NodesContainer nodes;
		threading::ThreadPool* threadPool;
		bool isThreadPoolWeak;

	private:
		static void threadPoolCallback(std::promise<void>&& requestPromise);

		static void deleter(FileManager* instance);

	public:
		FileHandle* createHandle(const std::filesystem::path& filePath, requestFileHandleType handleType);

		void notify(std::filesystem::path&& filePath);

		void addRequest(const std::filesystem::path& filePath, fileCallback&& callback, std::promise<void>&& requestPromise, requestFileHandleType handleType);

		void decreaseReadRequests(const std::filesystem::path& filePath);

		void completeWriteRequest(const std::filesystem::path& filePath);

	private:
		FileManager();

		FileManager(size_t threadsNumber);

		FileManager(threading::ThreadPool* threadPool);

		~FileManager();

		FileManager(const FileManager&) = delete;

		FileManager(FileManager&&) noexcept = delete;

		FileManager& operator = (const FileManager&) = delete;

		FileManager& operator = (FileManager&&) noexcept = delete;

	private:
		std::future<void> addReadRequest(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<ReadFileHandle>&&)>& callback, requestFileHandleType handleType, bool isWait);

		std::future<void> addWriteRequest(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<WriteFileHandle>&&)>& callback, requestFileHandleType handleType, bool isWait);

	public:
		/**
		 * @brief Singleton getter
		 * Also initialize thread pool with max threads for current hardware
		 * Default getter after initialization
		 * @return Singleton instance
		 */
		static FileManager& getInstance();

		/**
		 * @brief Singleton getter. Will reinitialize if threadsNumber != current threadsNumber
		 * @param threadsNumber ThreadPool threads number
		 * @return Singleton instance
		 */
		static FileManager& getInstance(size_t threadsNumber);

		/**
		 * @brief Singleton getter
		 * @param threadPool FileManager will use this thread pool instead of initializing its own thread pool
		 * @return Singleton instance
		 */
		static FileManager& getInstance(threading::ThreadPool* threadPool);

		/**
		 * @brief FileManager version
		 * @return Get FileManager version
		 */
		static std::string getVersion();

		/// @brief Add file to manager
		/// @param filePath Path to file
		/// @param isFileAlreadyExist If true and file does not exist FileDoesNotExistException will be thrown. If true and filePath contains path to non regular file NotAFileException will be thrown
		/// @exception FileDoesNotExistException 
		/// @exception NotAFileException 
		void addFile(const std::filesystem::path& filePath, bool isFileAlreadyExist = true);

		/// @brief Read file in standard mode
		/// @param filePath Path to file
		/// @param callback Function that will be called for reading file
		/// @param isWait If true thread will wait till callback end
		/// @exception FileDoesNotExistException 
		/// @exception NotAFileException 
		std::future<void> readFile(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<ReadFileHandle>&&)>& callback, bool isWait = true);

		/// @brief Read file in binary mode
		/// @param filePath Path to file
		/// @param callback Function that will be called for reading file
		/// @param isWait If true thread will wait till callback end
		/// @exception FileDoesNotExistException 
		/// @exception NotAFileException 
		std::future<void> readBinaryFile(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<ReadFileHandle>&&)>& callback, bool isWait = true);

		/// @brief Create/Recreate and write file in standard mode
		/// @param filePath Path to file
		/// @param callback Function that will be called for writing file
		/// @param isWait If true thread will wait till callback end
		std::future<void> writeFile(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<WriteFileHandle>&&)>& callback, bool isWait = true);

		/// @brief Create file if it does not exist and write file in standard mode
		/// @param filePath Path to file
		/// @param callback Function that will be called for writing file
		/// @param isWait If true thread will wait till callback end
		std::future<void> appendFile(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<WriteFileHandle>&&)>& callback, bool isWait = true);

		/// @brief Create/Recreate and write file in binary mode
		/// @param filePath Path to file
		/// @param callback Function that will be called for writing file
		/// @param isWait If true thread will wait till callback end
		std::future<void> writeBinaryFile(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<WriteFileHandle>&&)>& callback, bool isWait = true);

		/// @brief Create file if it does not exist and write file in binary mode
		/// @param filePath Path to file
		/// @param callback Function that will be called for writing file
		/// @param isWait If true thread will wait till callback end
		std::future<void> appendBinaryFile(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<WriteFileHandle>&&)>& callback, bool isWait = true);

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

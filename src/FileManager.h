#pragma once

#include <filesystem>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <fstream>
#include <functional>
#include <variant>
#include <queue>

#include "ForwardDeclaration.h"

#ifdef FILE_MANAGER_DLL
#define FILE_MANAGER_API __declspec(dllexport)
#else
#define FILE_MANAGER_API
#endif

namespace file_manager
{
	using std::streamsize;

	class FILE_MANAGER_API FileManager
	{
	private:
		struct pathHash
		{
			size_t operator () (const std::filesystem::path& pathToFile) const noexcept;
		};

		struct filePathState
		{
			size_t readRequests;
			bool isWriteRequest;

			filePathState();
		};
		
		class FileHandle
		{
		protected:
			std::filesystem::path pathToFile;
			std::fstream file;
			std::ios_base::openmode mode;
			bool isNotifyOnDestruction;

		public:
			FileHandle(const std::filesystem::path& pathToFile, std::ios_base::openmode mode);

			FileHandle(FileHandle&& other) noexcept;

			FileHandle& operator = (FileHandle&& other) noexcept;

			uintmax_t getFileSize() const;

			virtual ~FileHandle();
		};

	public:
		class FILE_MANAGER_API ReadFileHandle : public FileHandle
		{
		private:
			ReadFileHandle(const std::filesystem::path& pathToFile, std::ios_base::openmode mode = 0);

		public:
			/// @brief Read all file
			/// @return File's data
			std::string readAllData();

			/// @brief Read some data from file
			/// @param outData Data from file
			/// @param count Count of characters to read
			/// @param resizeOutData If true outData has exactly same size as number of characters read. If false you must provide outData size before calling
			/// @return Number of characters read
			streamsize readSome(std::string& outData, streamsize count, bool resizeOutData = true);

			virtual ~ReadFileHandle();

			friend class FileManager;
		};

		class FILE_MANAGER_API WriteFileHandle : public FileHandle
		{
		private:
			WriteFileHandle(const std::filesystem::path& pathToFile, std::ios_base::openmode mode = 0);

		public:
			/// @brief Write data to file
			/// @param data Data
			void write(const std::string& data);

			virtual ~WriteFileHandle();

			friend class FileManager;
		};

		using readFileCallback = std::function<void(ReadFileHandle&&)>;
		using writeFileCallback = std::function<void(WriteFileHandle&&)>;

	private:
		class ReadBinaryFileHandle : public ReadFileHandle
		{
		public:
			ReadBinaryFileHandle(const std::filesystem::path& pathToFile);

			~ReadBinaryFileHandle() = default;
		};

		class WriteBinaryFileHandle : public WriteFileHandle
		{
		public:
			WriteBinaryFileHandle(const std::filesystem::path& pathToFile, std::ios_base::openmode mode = 0);

			virtual ~WriteBinaryFileHandle() = default;
		};

		class AppendBinaryFileHandle : public WriteBinaryFileHandle
		{
		public:
			AppendBinaryFileHandle(const std::filesystem::path& pathToFile);

			~AppendBinaryFileHandle() = default;
		};

		class AppendFileHandle : public WriteFileHandle
		{
		public:
			AppendFileHandle(const std::filesystem::path& pathToFile);

			~AppendFileHandle() = default;
		};

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
		std::unordered_map<std::filesystem::path, filePathState, pathHash> files;
		std::unordered_map<std::filesystem::path, std::queue<requestStruct>, pathHash> requests;
		std::mutex filesMutex;
		std::mutex requestsMutex;

	private:
		FileHandle createHandle(const std::filesystem::path& pathToFile, requestFileHandleType handleType);

		void notify(std::filesystem::path&& pathToFile, std::ios_base::openmode mode);

		void addRequest(const std::filesystem::path& pathToFile, fileCallback&& callback, const std::function<void()>& onEndCallback, requestFileHandleType handleType);

		void processQueue(const std::filesystem::path& pathToFile);

		void changeReadRequests(const std::filesystem::path& pathToFile, int value);

		void changeIsWriteRequest(const std::filesystem::path& pathToFile, bool value);

	private:
		FileManager(uint32_t threadsCount);

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
		static FileManager& getInstance(uint32_t threadsCount = std::thread::hardware_concurrency());

		void addFile(const std::filesystem::path& pathToFile, bool isFileAlreadyExist = true);

		void readFile(const std::filesystem::path& pathToFile, const readFileCallback& callback, bool isWait = true);

		void readBinaryFile(const std::filesystem::path& pathToFile, const readFileCallback& callback, bool isWait = true);

		void writeFile(const std::filesystem::path& pathToFile, const writeFileCallback& callback, bool isWait = true);

		void appendFile(const std::filesystem::path& pathToFile, const writeFileCallback& callback, bool isWait = true);

		void writeBinaryFile(const std::filesystem::path& pathToFile, const writeFileCallback& callback, bool isWait = true);

		void appendBinaryFile(const std::filesystem::path& pathToFile, const writeFileCallback& callback, bool isWait = true);
	};

	using ReadFileHandle = FileManager::ReadFileHandle;
	using WriteFileHandle = FileManager::WriteFileHandle;

	/// <summary>
	/// std::function&lt;void(ReadFileHandle&amp;&amp;)&gt;;
	/// </summary>
	using readFileCallback = FileManager::readFileCallback;
	/// <summary>
	/// std::function&lt;void(WriteFileHandle&amp;&amp;)&gt;;
	/// </summary>
	using writeFileCallback = FileManager::writeFileCallback;
}

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

			virtual ~FileHandle();
		};

	public:
		class FILE_MANAGER_API ReadFileHandle : public FileHandle
		{
		private:
			ReadFileHandle(const std::filesystem::path& pathToFile);

		public:
			std::string readAllFile();

			~ReadFileHandle();

			friend class FileManager;
		};

		class FILE_MANAGER_API WriteFileHandle : public FileHandle
		{
		private:
			WriteFileHandle(const std::filesystem::path& pathToFile);

		public:
			void write(const std::string& data);

			~WriteFileHandle();

			friend class FileManager;
		};

		using readFileCallback = std::function<void(ReadFileHandle&&)>;
		using writeFileCallback = std::function<void(WriteFileHandle&&)>;

	private:
		enum class requestType
		{
			read,
			write
		};

		using fileCallback = std::variant<readFileCallback, writeFileCallback>;

		struct requestStruct
		{
			fileCallback callback;
			std::function<void()> onEndCallback;

			requestStruct(fileCallback&& callback, const std::function<void()>& onEndCallback);
		};

		friend bool operator == (const requestStruct& request, requestType type);

	private:
		std::unique_ptr<threading::ThreadPool> threadPool;
		std::unordered_map<std::filesystem::path, filePathState, pathHash> files;
		std::unordered_map<std::filesystem::path, std::queue<requestStruct>, pathHash> requests;
		std::mutex filesMutex;
		std::mutex requestsMutex;

	private:
		void notify(std::filesystem::path&& pathToFile, std::ios_base::openmode mode);

		void addRequest(const std::filesystem::path& pathToFile, fileCallback&& callback, const std::function<void()>& onEndCallback = nullptr);

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

	public:
		static FileManager& getInstance(uint32_t threadsCount = std::thread::hardware_concurrency());

		void addFile(const std::filesystem::path& pathToFile, bool isFileAlreadyExist = true);

		void readFile(const std::filesystem::path& pathToFile, const readFileCallback& callback, bool isWait = true);

		void writeFile(const std::filesystem::path& pathToFile, const writeFileCallback& callback, bool isWait = true);
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

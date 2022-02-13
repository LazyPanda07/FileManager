#pragma once

#include <filesystem>
#include <unordered_map>
#include <thread>
#include <mutex>

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

	private:
		std::unique_ptr<threading::ThreadPool> threadPool;
		std::unordered_map<std::filesystem::path, filePathState, pathHash> files;
		std::mutex filesMutex;

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
	};
}

#pragma once

#include <filesystem>
#include <thread>

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
		std::unique_ptr<threading::ThreadPool> threadPool;

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


	};
}

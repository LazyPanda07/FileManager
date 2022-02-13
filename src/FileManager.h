#pragma once

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
		FileManager();

		~FileManager() = default;

	public:
		FileManager(const FileManager&) = delete;

		FileManager(FileManager&&) noexcept = delete;

		FileManager& operator = (const FileManager&) = delete;

		FileManager& operator = (FileManager&&) noexcept = delete;

	public:
		static FileManager& getInstance();


	};
}

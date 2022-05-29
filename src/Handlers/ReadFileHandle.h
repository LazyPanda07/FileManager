#pragma once

#include <functional>

#include "FileHandle.h"

namespace file_manager
{
	/// @brief Provides reading files
	class FILE_MANAGER_API ReadFileHandle : public FileHandle
	{
	private:
		class readOnlyBuffer : public std::stringbuf
		{
		public:
			readOnlyBuffer(std::string_view view);
		};

	private:
		std::string data;
		std::unique_ptr<readOnlyBuffer> buffer;

	protected:
		ReadFileHandle(const std::filesystem::path& pathToFile, std::ios_base::openmode mode = 0);

	public:
		/// @brief Read all file
		/// @return File's data
		/// @exception FileDoesNotExistException
		const std::string& readAllData();

		/// @brief Read some data from file
		/// @param outData Data from file
		/// @param count Count of characters to read
		/// @param resizeOutData If true outData has exactly same size as number of characters read. If false you must provide outData size before calling
		/// @return Number of characters read
		std::streamsize readSome(std::string& outData, std::streamsize count, bool resizeOutData = true);

		/// @brief Get reading stream
		/// @return Input stream
		std::istream& getStream();

		virtual ~ReadFileHandle();

		friend class FileManager;
	};

	/// @brief Used in reading requests
	using readFileCallback = std::function<void(std::unique_ptr<ReadFileHandle>&&)>;
}

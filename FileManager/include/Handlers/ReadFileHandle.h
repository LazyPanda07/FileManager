#pragma once

#include <functional>
#include <sstream>

#include "FileHandle.h"

namespace file_manager
{
	/// @brief Provides reading files
	class FILE_MANAGER_API ReadFileHandle : public FileHandle
	{
	private:
		class ReadOnlyBuffer : public std::stringbuf
		{
		public:
			ReadOnlyBuffer(std::string_view view);
		};

	private:
		std::string data;
		std::unique_ptr<ReadOnlyBuffer> buffer;

	protected:
		ReadFileHandle(const std::filesystem::path& filePath, std::ios_base::openmode mode = std::ios_base::in);

	public:
		/// @brief Read all file
		/// @return File's data
		/// @exception FileDoesNotExistException
		const std::string& readAllData();

		/// @brief Read some data from file
		/// @param outData Data from file
		/// @param count Count of characters to read
		/// @param shrinkOutData If true outData will be shrinked
		/// @param resizeOutData If true outData will be resized to count size. If false you must provide outData size before calling
		/// @return Number of characters read
		std::streamsize readSome(std::string& outData, std::streamsize count, bool shrinkOutData = true, bool resizeOutData = true);

		/// @brief Get reading stream
		/// @return Input stream
		std::istream& getStream();

		virtual ~ReadFileHandle();

		friend class FileManager;
	};
}

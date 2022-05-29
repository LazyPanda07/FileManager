#pragma once

#include <functional>

#include "FileHandle.h"

namespace file_manager
{
	/// @brief Provides writing files
	class FILE_MANAGER_API WriteFileHandle : public FileHandle
	{
	protected:
		WriteFileHandle(const std::filesystem::path& pathToFile, std::ios_base::openmode mode = 0);

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

	/// @brief Used in writing requests
	using writeFileCallback = std::function<void(std::unique_ptr<WriteFileHandle>&&)>;
}

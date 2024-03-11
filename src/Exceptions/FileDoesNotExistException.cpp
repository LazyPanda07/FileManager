#include "FileDoesNotExistException.h"

#include <format>

using namespace std;

namespace file_manager
{
	namespace exceptions
	{
		FileDoesNotExistException::FileDoesNotExistException(const filesystem::path& filePath) :
			BaseFileManagerException(format("File '{}' does not exist"sv, filePath.string()))
		{

		}
	}
}

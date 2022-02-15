#include "FileDoesNotExistException.h"

#include <format>

using namespace std;

namespace file_manager
{
	namespace exceptions
	{
		FileDoesNotExistException::FileDoesNotExistException(const filesystem::path& pathToFile) :
			BaseFileManagerException(format("File '{}' does not exist"sv, pathToFile.string()))
		{

		}
	}
}

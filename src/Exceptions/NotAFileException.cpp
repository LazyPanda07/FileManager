#include "NotAFileException.h"

#include <format>

using namespace std;

namespace file_manager
{
	namespace exceptions
	{
		NotAFileException::NotAFileException(const filesystem::path& path) :
			BaseFileManagerException(format("Path '{}' does not represent file"sv, path.string()))
		{

		}
	}
}

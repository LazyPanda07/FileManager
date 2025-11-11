#include "Exceptions/BaseFileManagerException.h"

namespace file_manager::exceptions
{
	BaseFileManagerException::BaseFileManagerException(const std::string& message) :
		runtime_error(message)
	{

	}
}

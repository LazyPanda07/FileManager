#include "BaseFileManagerException.h"

using namespace std;

namespace file_manager
{
	namespace exceptions
	{
		BaseFileManagerException::BaseFileManagerException(const string& message) : 
			runtime_error(message)
		{

		}
	}
}

#include "Utility.h"

using namespace std;

namespace file_manager
{
	namespace utility
	{
		size_t pathHash::operator () (const filesystem::path& pathToFile) const noexcept
		{
			return hash<string>()(pathToFile.string());
		}
	}
}

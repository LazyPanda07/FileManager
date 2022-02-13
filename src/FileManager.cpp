#include "FileManager.h"

using namespace std;

namespace file_manager
{
	FileManager::FileManager()
	{

	}

	FileManager& FileManager::getInstance()
	{
		static FileManager instance;

		return instance;
	}
}

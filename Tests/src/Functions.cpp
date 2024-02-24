#include "Functions.h"

void appendCallback(std::unique_ptr<file_manager::WriteFileHandle>&& handle)
{
	handle->write("test\n");
}

void threadWrite()
{
	file_manager::FileManager& manager = file_manager::FileManager::getInstance();
	std::vector<std::future<void>> futures;

	futures.reserve(writes);

	for (size_t i = 0; i < writes; i++)
	{
		futures.emplace_back(manager.appendFile("test.txt", &appendCallback, false));
	}

	for (std::future<void>& future : futures)
	{
		future.wait();
	}
}
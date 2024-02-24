#include "gtest/gtest.h"

#include "Functions.h"

using namespace std::chrono_literals;

TEST(FileManager, AsyncWrite)
{
	file_manager::FileManager& manager = file_manager::FileManager::getInstance();
	std::vector<std::future<void>> threads;
	std::string data;

	threads.reserve(threadsCount);

	std::ofstream("test.txt");

	manager.addFile("test.txt");

	for (size_t i = 0; i < threadsCount; i++)
	{
		threads.emplace_back(std::async(std::launch::async, &threadWrite));
	}

	std::this_thread::sleep_for(1s);

	for (std::future<void>& thread : threads)
	{
		thread.wait();
	}

	manager.readFile
	(
		"test.txt",
		[&data](std::unique_ptr<file_manager::ReadFileHandle>&& handle)
		{
			data = handle->readAllData();
		}
	);

	std::istringstream is(data);
	std::string line;
	size_t count = 0;

	while (std::getline(is, line))
	{
		count++;
	}

	ASSERT_EQ(count, totalWrites);
}

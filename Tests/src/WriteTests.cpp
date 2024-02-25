#include "gtest/gtest.h"

#include "FileManager.h"

using namespace std::chrono_literals;

inline constexpr int threadsCount = 8;
inline constexpr int writes = 512;
inline constexpr int totalWrites = threadsCount * writes;

void threadWrite(const std::string& fileName, const std::string& text)
{
	file_manager::FileManager& manager = file_manager::FileManager::getInstance();
	std::vector<std::future<void>> futures;

	futures.reserve(writes);

	for (size_t i = 0; i < writes; i++)
	{
		futures.emplace_back
		(
			manager.appendFile
			(
				fileName,
				[text](std::unique_ptr<file_manager::WriteFileHandle>&& handle)
				{
					handle->write
					(
						text.empty() ?
						"test\n" :
						text
					);
				},
				false
			)
		);
	}

	for (std::future<void>& future : futures)
	{
		future.wait();
	}
}

TEST(FileManager, Write)
{
	file_manager::FileManager& manager = file_manager::FileManager::getInstance();
	const std::string fileName("write_test.txt");
	std::vector<std::future<void>> threads;
	std::string data;

	threads.reserve(threadsCount);
	
	{
		std::ofstream file(fileName);
	}

	manager.addFile(fileName);

	for (size_t i = 0; i < threadsCount; i++)
	{
		threads.emplace_back(std::async(std::launch::async, &threadWrite, std::ref(fileName), ""));
	}

	std::this_thread::sleep_for(1s);

	for (std::future<void>& thread : threads)
	{
		thread.wait();
	}

	manager.readFile
	(
		fileName,
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

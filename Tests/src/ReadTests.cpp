#include <random>

#include "gtest/gtest.h"

#include "FileManager.h"

using namespace std::chrono_literals;

std::atomic<size_t> totalSize = 0;

size_t randomFill(const std::string& fileName)
{
	file_manager::FileManager& manager = file_manager::FileManager::getInstance();
	std::mt19937_64 random(time(nullptr));
	size_t result = 0;

	for (size_t i = 0; i < random() % 2048 + 2048; i++)
	{
		if (random() % 2)
		{
			result++;

			manager.appendFile
			(
				fileName,
				[](std::unique_ptr<file_manager::WriteFileHandle>&& handle)
				{
					handle->write("1");

					totalSize++;
				},
				false
			);
		}
		else
		{
			manager.readFile
			(
				fileName,
				[](std::unique_ptr<file_manager::ReadFileHandle>&& handle)
				{
					ASSERT_EQ(handle->readAllData().size(), totalSize);
				},
				false
			);
		}
	}

	return result;
}

TEST(FileManager, Read)
{
	file_manager::FileManager& manager = file_manager::FileManager::getInstance();
	const std::string fileName("read_test.txt");
	std::mt19937_64 random(time(nullptr));
	std::string data;

	{
		std::ofstream file(fileName);
	}

	manager.addFile(fileName);

	std::future<size_t> first = std::async(std::launch::async, &randomFill, std::ref(fileName));
	std::future<size_t> second = std::async(std::launch::async, &randomFill, std::ref(fileName));

	size_t randomFillWrites = first.get() + second.get();

	manager.readFile
	(
		fileName,
		[&data](std::unique_ptr<file_manager::ReadFileHandle>&& handle)
		{
			data = handle->readAllData();
		}
	);

	ASSERT_EQ(randomFillWrites, data.size());
	ASSERT_EQ(totalSize, data.size());
}

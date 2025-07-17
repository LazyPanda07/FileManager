#include <random>
#include <thread>
#include <format>
#include <unordered_map>

#include "gtest/gtest.h"

#include "FileManager.h"
#include "ThreadPool.h"

using namespace std::chrono_literals;

std::unordered_map<std::string, size_t> totalSizes;

static size_t randomFill(const std::string& fileName)
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
				[&fileName](std::unique_ptr<file_manager::WriteFileHandle>&& handle)
				{
					handle->write("1");

					try
					{
						totalSizes.at(fileName)++;
					}
					catch (const std::exception&)
					{
						std::cerr << "Can't find " << fileName << " in totalSizes at " << __LINE__ << std::endl;

						throw;
					}
				},
				false
			);
		}
		else
		{
			manager.readFile
			(
				fileName,
				[&fileName](std::unique_ptr<file_manager::ReadFileHandle>&& handle)
				{
					try
					{
						ASSERT_EQ(handle->readAllData().size(), totalSizes.at(fileName));
					}
					catch (const std::exception&)
					{
						std::cerr << "Can't find " << fileName << " in totalSizes at " << __LINE__ << std::endl;

						throw;
					}
				},
				false
			);
		}
	}

	return result;
}

TEST(FileManager, Read)
{
	file_manager::FileManager& manager = file_manager::FileManager::getInstance(std::thread::hardware_concurrency());
	const std::string fileName("read_test.txt");
	std::string data;

	{
		std::ofstream file(fileName);
	}

	totalSizes[fileName] = 0;

	manager.addFile(fileName);

	std::future<size_t> first = std::async(std::launch::async, &randomFill, std::ref(fileName));
	std::future<size_t> second = std::async(std::launch::async, &randomFill, std::ref(fileName));
	std::future<size_t> third = std::async(std::launch::async, &randomFill, std::ref(fileName));
	std::future<size_t> fourth = std::async(std::launch::async, &randomFill, std::ref(fileName));

	size_t randomFillWrites = first.get() + second.get() + third.get() + fourth.get();

	manager.readFile
	(
		fileName,
		[&data](std::unique_ptr<file_manager::ReadFileHandle>&& handle)
		{
			data = handle->readAllData();
		}
	);

	ASSERT_EQ(randomFillWrites, data.size());
	ASSERT_EQ(totalSizes.at(fileName), data.size());
}

TEST(FileManager, MultipleRead)
{
	for (size_t i = 1; i <= 8; i++)
	{
		std::shared_ptr<threading::ThreadPool> threadPool = std::make_shared<threading::ThreadPool>(i);
		file_manager::FileManager& manager = file_manager::FileManager::getInstance(threadPool);
		const std::string fileName(std::format("read_test{}.txt", i));
		std::string data;

		{
			std::ofstream file(fileName);
		}

		totalSizes[fileName] = 0;

		manager.addFile(fileName);

		std::future<size_t> first = std::async(std::launch::async, &randomFill, std::ref(fileName));
		std::future<size_t> second = std::async(std::launch::async, &randomFill, std::ref(fileName));
		std::future<size_t> third = std::async(std::launch::async, &randomFill, std::ref(fileName));
		std::future<size_t> fourth = std::async(std::launch::async, &randomFill, std::ref(fileName));

		size_t randomFillWrites = first.get() + second.get() + third.get() + fourth.get();

		manager.readFile
		(
			fileName,
			[&data](std::unique_ptr<file_manager::ReadFileHandle>&& handle)
			{
				data = handle->readAllData();
			}
		);

		ASSERT_EQ(randomFillWrites, data.size());
		ASSERT_EQ(totalSizes.at(fileName), data.size());
	}
}

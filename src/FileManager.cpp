#include "FileManager.h"

#include "ThreadPool.h"

using namespace std;

namespace file_manager
{
	size_t FileManager::pathHash::operator () (const filesystem::path& pathToFile) const noexcept
	{
		return hash<string>()(pathToFile.string());
	}

	FileManager::filePathState::filePathState() :
		readRequests(0),
		isWriteRequest(false)
	{

	}

	void FileManager::notify(filesystem::path&& pathToFile)
	{
		lock_guard<mutex> filesLock(filesMutex);

		if (!files[pathToFile].isWriteRequest)
		{
			threadPool->addTask([this, path = move(pathToFile)]()
				{
					this->processQueue(path);
				});
		}
	}

	void FileManager::processQueue(const filesystem::path& pathToFile)
	{

	}

	void FileManager::changeReadRequests(const filesystem::path& pathToFile, int value)
	{
		lock_guard<mutex> filesLock(filesMutex);

		files[pathToFile].readRequests += value;
	}

	void FileManager::changeIsWriteRequest(const filesystem::path& pathToFile, bool value)
	{
		lock_guard<mutex> filesLock(filesMutex);

		files[pathToFile].isWriteRequest = value;
	}

	FileManager::FileManager(uint32_t threadsCount) :
		threadPool(make_unique<threading::ThreadPool>(threadsCount))
	{

	}

	FileManager& FileManager::getInstance(uint32_t threadsCount)
	{
		static FileManager instance(threadsCount);

		if (instance.threadPool->getThreadsCount() != threadsCount)
		{
			// TODO: sync

			instance.threadPool->reinit(true, threadsCount);
		}

		return instance;
	}

	void FileManager::addFile(const filesystem::path& pathToFile, bool isFileAlreadyExist)
	{
		if (isFileAlreadyExist)
		{
			if (!filesystem::exists(pathToFile))
			{
				lock_guard<mutex> filesLock(filesMutex);

				files.erase(pathToFile);

				throw runtime_error("File doesn't exist");
			}
		}

		lock_guard<mutex> filesLock(filesMutex);

		if (files.find(pathToFile) == files.end())
		{
			files[pathToFile] = filePathState();
		}
	}
}

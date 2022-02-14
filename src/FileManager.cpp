#include "FileManager.h"

#include <future>

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

	static bool operator == (const FileManager::fileCallback& callback, FileManager::callbackType type)
	{
		return callback.index() == static_cast<size_t>(type);
	}

	void FileManager::notify(filesystem::path&& pathToFile)
	{
		lock_guard<mutex> filesLock(filesMutex);

		if (!files[pathToFile].isWriteRequest)
		{
			threadPool->addTask([this, tem = move(pathToFile)]()
			{
				this->processQueue(tem);
			});
		}
	}

	void FileManager::addRequest(const filesystem::path& pathToFile, fileCallback&& callback)
	{
		lock_guard<mutex> requestsLock(requestsMutex);

		requests[pathToFile].push(move(callback));
	}

	void FileManager::processQueue(const filesystem::path& pathToFile, const function<void()>& onEndCallback)
	{
		lock_guard<mutex> requestsLock(requestsMutex);
		queue<fileCallback>& requestsQueue = requests[pathToFile];

		while (requestsQueue.size())
		{
			fileCallback& callback = requestsQueue.front();

			if (callback == callbackType::read)
			{
				{
					lock_guard<mutex> filesLock(filesMutex);

					if (files[pathToFile].isWriteRequest)
					{
						return;
					}
				}

				readFileCallback tem = move(get<readFileCallback>(callback));

				requestsQueue.pop();

				threadPool->addTask([pathToFile, readCallback = move(tem)]()
				{
					readCallback(ReadFileHandle(pathToFile));
				}, onEndCallback);
			}
			else if (callback == callbackType::write)
			{
				{
					lock_guard<mutex> filesLock(filesMutex);

					if (files[pathToFile].readRequests)
					{
						return;
					}
				}

				writeFileCallback tem = move(get<writeFileCallback>(callback));

				requestsQueue.pop();

				threadPool->addTask([pathToFile, writeCallback = move(tem)]()
				{
					writeCallback(WriteFileHandle(pathToFile));
				}, onEndCallback);

				return;
			}
		}
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

	void FileManager::readFile(const filesystem::path& pathToFile, const readFileCallback& callback, bool isWait)
	{
		this->addFile(pathToFile);

		this->addRequest(pathToFile, callback);

		if (isWait)
		{
			promise<void> isReady;
			future<void> waiting = isReady.get_future();

			this->processQueue(pathToFile, [&isReady]()
				{
					isReady.set_value();
				});

			this->processQueue(pathToFile);

			waiting.wait();
		}
		else
		{
			this->processQueue(pathToFile);
		}
	}

	void FileManager::writeFile(const filesystem::path& pathToFile, const writeFileCallback& callback, bool isWait)
	{
		this->addFile(pathToFile, false);

		this->addRequest(pathToFile, callback);

		if (isWait)
		{
			promise<void> isReady;
			future<void> waiting = isReady.get_future();

			this->processQueue(pathToFile, [&isReady]()
				{
					isReady.set_value();
				});

			this->processQueue(pathToFile);

			waiting.wait();
		}
		else
		{
			this->processQueue(pathToFile);
		}
	}
}

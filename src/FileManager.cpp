#include "FileManager.h"

#include <future>
#include <iostream>

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

	FileManager::requestStruct::requestStruct(fileCallback&& callback, const function<void()>& onEndCallback) :
		callback(move(callback)),
		onEndCallback(onEndCallback)
	{

	}

	static bool operator == (const FileManager::requestStruct& request, FileManager::requestType type)
	{
		return request.callback.index() == static_cast<size_t>(type);
	}

	void FileManager::notify(filesystem::path&& pathToFile, ios_base::openmode mode)
	{
		lock_guard<mutex> filesLock(filesMutex);

		if (mode == ios_base::out && !files[pathToFile].isWriteRequest)
		{
			threadPool->addTask([this, tem = move(pathToFile)]()
			{
				this->processQueue(tem);
			});
		}
	}

	void FileManager::addRequest(const filesystem::path& pathToFile, fileCallback&& callback, const function<void()>& onEndCallback)
	{
		lock_guard<mutex> requestsLock(requestsMutex);

		requests[pathToFile].push(requestStruct(move(callback), onEndCallback));
	}

	void FileManager::processQueue(const filesystem::path& pathToFile)
	{
		lock_guard<mutex> requestsLock(requestsMutex);
		queue<requestStruct>& requestsQueue = requests[pathToFile];

		while (requestsQueue.size())
		{
			requestStruct& request = requestsQueue.front();

			if (request == requestType::read)
			{
				{
					lock_guard<mutex> filesLock(filesMutex);

					if (files[pathToFile].isWriteRequest)
					{
						return;
					}
				}

				readFileCallback tem = move(get<readFileCallback>(request.callback));
				function<void()> onEndCallback = move(request.onEndCallback);

				requestsQueue.pop();

				threadPool->addTask([pathToFile, callback = move(tem)]()
				{
					callback(ReadFileHandle(pathToFile));
				}, onEndCallback);
			}
			else if (request == requestType::write)
			{
				{
					lock_guard<mutex> filesLock(filesMutex);

					if (files[pathToFile].readRequests)
					{
						return;
					}
				}

				writeFileCallback tem = move(get<writeFileCallback>(request.callback));
				function<void()> onEndCallback = move(request.onEndCallback);

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

		if (isWait)
		{
			promise<void> isReady;
			future<void> waitOperation = isReady.get_future();

			this->addRequest(pathToFile, callback, [&isReady]()
				{
					isReady.set_value();
				});

			this->processQueue(pathToFile);

			waitOperation.wait();
		}
		else
		{
			this->addRequest(pathToFile, callback);

			this->processQueue(pathToFile);
		}
	}

	void FileManager::writeFile(const filesystem::path& pathToFile, const writeFileCallback& callback, bool isWait)
	{
		this->addFile(pathToFile, false);

		if (isWait)
		{
			promise<void> isReady;
			future<void> waitOperation = isReady.get_future();

			this->addRequest(pathToFile, callback, [&isReady]()
				{
					isReady.set_value();
				});

			this->processQueue(pathToFile);

			waitOperation.wait();
		}
		else
		{
			this->addRequest(pathToFile, callback);

			this->processQueue(pathToFile);
		}
	}
}

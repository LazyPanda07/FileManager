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

	FileManager::requestStruct::requestStruct(fileCallback&& callback, const function<void()>& onEndCallback, requestFileHandleType handleType) :
		callback(move(callback)),
		onEndCallback(onEndCallback),
		handleType(handleType)
	{

	}

	static bool operator == (const FileManager::requestStruct& request, FileManager::requestType type)
	{
		return request.callback.index() == static_cast<size_t>(type);
	}

	FileManager::FileHandle FileManager::createHandle(const filesystem::path& pathToFile, requestFileHandleType handleType)
	{
		switch (handleType)
		{
		case file_manager::FileManager::requestFileHandleType::read:
			return ReadFileHandle(pathToFile);

		case file_manager::FileManager::requestFileHandleType::write:
			return WriteFileHandle(pathToFile);
			
		case file_manager::FileManager::requestFileHandleType::readBinary:
			return ReadBinaryFileHandle(pathToFile);

		case file_manager::FileManager::requestFileHandleType::writeBinary:
			return WriteBinaryFileHandle(pathToFile);

		case file_manager::FileManager::requestFileHandleType::append:
			return AppendFileHandle(pathToFile);

		case file_manager::FileManager::requestFileHandleType::appendBinary:
			return AppendBinaryFileHandle(pathToFile);
		}

		return FileHandle(pathToFile, NULL);
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

	void FileManager::addRequest(const filesystem::path& pathToFile, fileCallback&& callback, const function<void()>& onEndCallback, requestFileHandleType handleType)
	{
		lock_guard<mutex> requestsLock(requestsMutex);

		requests[pathToFile].push(requestStruct(move(callback), onEndCallback, handleType));
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
				requestFileHandleType handleType = request.handleType;

				requestsQueue.pop();

				threadPool->addTask([this, pathToFile, callback = move(tem), handleType]()
				{
					callback(static_cast<ReadFileHandle&&>(this->createHandle(pathToFile, handleType)));
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
				requestFileHandleType handleType = request.handleType;

				requestsQueue.pop();

				threadPool->addTask([this, pathToFile, writeCallback = move(tem), handleType]()
				{
					writeCallback(static_cast<WriteFileHandle&&>(this->createHandle(pathToFile, handleType)));
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

	void FileManager::addReadRequest(const filesystem::path& pathToFile, const readFileCallback& callback, requestFileHandleType handleType, bool isWait)
	{
		this->addFile(pathToFile);

		if (isWait)
		{
			promise<void> isReady;
			future<void> waitOperation = isReady.get_future();

			this->addRequest(pathToFile, callback, [&isReady]()
				{
					isReady.set_value();
				}, handleType);

			this->processQueue(pathToFile);

			waitOperation.wait();
		}
		else
		{
			this->addRequest(pathToFile, callback, nullptr, handleType);

			this->processQueue(pathToFile);
		}
	}

	void FileManager::addWriteRequest(const std::filesystem::path& pathToFile, const writeFileCallback& callback, requestFileHandleType handleType, bool isWait)
	{
		this->addFile(pathToFile, false);

		if (isWait)
		{
			promise<void> isReady;
			future<void> waitOperation = isReady.get_future();

			this->addRequest(pathToFile, callback, [&isReady]()
				{
					isReady.set_value();
				}, handleType);

			this->processQueue(pathToFile);

			waitOperation.wait();
		}
		else
		{
			this->addRequest(pathToFile, callback, nullptr, handleType);

			this->processQueue(pathToFile);
		}
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
		this->addReadRequest(pathToFile, callback, requestFileHandleType::read, isWait);
	}

	void FileManager::readBinaryFile(const filesystem::path& pathToFile, const readFileCallback& callback, bool isWait)
	{
		this->addReadRequest(pathToFile, callback, requestFileHandleType::readBinary, isWait);
	}

	void FileManager::writeFile(const filesystem::path& pathToFile, const writeFileCallback& callback, bool isWait)
	{
		this->addWriteRequest(pathToFile, callback, requestFileHandleType::write, isWait);
	}

	void FileManager::appendFile(const std::filesystem::path& pathToFile, const writeFileCallback& callback, bool isWait)
	{
		this->addWriteRequest(pathToFile, callback, requestFileHandleType::append, isWait);
	}

	void FileManager::writeBinaryFile(const std::filesystem::path& pathToFile, const writeFileCallback& callback, bool isWait)
	{
		this->addWriteRequest(pathToFile, callback, requestFileHandleType::writeBinary, isWait);
	}

	void FileManager::appendBinaryFile(const std::filesystem::path& pathToFile, const writeFileCallback& callback, bool isWait)
	{
		this->addWriteRequest(pathToFile, callback, requestFileHandleType::appendBinary, isWait);
	}
}

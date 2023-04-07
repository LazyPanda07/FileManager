#include "FileManager.h"

#include "Handlers/ReadBinaryFileHandle.h"
#include "Handlers/WriteBinaryFileHandle.h"
#include "Handlers/AppendFileHandle.h"
#include "Handlers/AppendBinaryFileHandle.h"

#include "Exceptions/FileDoesNotExistException.h"
#include "Exceptions/NotAFileException.h"

#include "ThreadPool.h"

using namespace std;

struct RequestPromiseHandler
{
	promise<void> requestPromise;

	RequestPromiseHandler(promise<void>&& requestPromise) :
		requestPromise(move(requestPromise))
	{

	}

	RequestPromiseHandler(const RequestPromiseHandler& other)
	{
		(*this) = other;
	}

	RequestPromiseHandler& operator = (const RequestPromiseHandler& other)
	{
		requestPromise = move(const_cast<RequestPromiseHandler&>(other).requestPromise);

		return *this;
	}
};

namespace file_manager
{
	FileManager::filePathState::filePathState() :
		readRequests(0),
		isWriteRequest(false)
	{

	}

	FileManager::requestStruct::requestStruct(fileCallback&& callback, promise<void>&& requestPromise, requestFileHandleType handleType) :
		callback(move(callback)),
		requestPromise(move(requestPromise)),
		handleType(handleType)
	{
		
	}

	static bool operator == (const FileManager::requestStruct& request, FileManager::requestType type)
	{
		return request.callback.index() == static_cast<size_t>(type);
	}

	void FileManager::threadPoolCallback(std::promise<void>&& requestPromise)
	{
		requestPromise.set_value();
	}

	FileHandle* FileManager::createHandle(const filesystem::path& pathToFile, requestFileHandleType handleType)
	{
		switch (handleType)
		{
		case file_manager::FileManager::requestFileHandleType::read:
			return new ReadFileHandle(pathToFile);

		case file_manager::FileManager::requestFileHandleType::write:
			return new WriteFileHandle(pathToFile);

		case file_manager::FileManager::requestFileHandleType::readBinary:
			return new ReadBinaryFileHandle(pathToFile);

		case file_manager::FileManager::requestFileHandleType::writeBinary:
			return new WriteBinaryFileHandle(pathToFile);

		case file_manager::FileManager::requestFileHandleType::append:
			return new AppendFileHandle(pathToFile);

		case file_manager::FileManager::requestFileHandleType::appendBinary:
			return new AppendBinaryFileHandle(pathToFile);
		}

		return new FileHandle(pathToFile, NULL);
	}

	void FileManager::notify(filesystem::path&& pathToFile, ios_base::openmode mode)
	{
		unique_lock<mutex> filesLock(filesMutex);

		if (mode & ios_base::out && !files[pathToFile].isWriteRequest)
		{
			threadPool->addTask([this, tem = move(pathToFile)]()
			{
				this->processQueue(tem);
			});
		}
	}

	void FileManager::addRequest(const filesystem::path& pathToFile, fileCallback&& callback, promise<void>&& requestPromise, requestFileHandleType handleType)
	{
		unique_lock<mutex> requestsLock(requestsMutex);

		requests[pathToFile].push(requestStruct(move(callback), move(requestPromise), handleType));
	}

	void FileManager::processQueue(const filesystem::path& pathToFile)
	{
		unique_lock<mutex> requestsLock(requestsMutex);
		queue<requestStruct>& requestsQueue = requests[pathToFile];

		while (requestsQueue.size())
		{
			requestStruct& request = requestsQueue.front();

			if (request == requestType::read)
			{
				{
					unique_lock<mutex> filesLock(filesMutex);
					filePathState& fileState = files[pathToFile];

					if (fileState.isWriteRequest)
					{
						return;
					}

					fileState.readRequests++;
				}

				function<void(unique_ptr<ReadFileHandle>&&)> readCallback = move(get<function<void(unique_ptr<ReadFileHandle>&&)>>(request.callback));
				RequestPromiseHandler handler(move(request.requestPromise));
				requestFileHandleType handleType = request.handleType;

				requestsQueue.pop();

				threadPool->addTask
				(
					[this, pathToFile, readCallback = move(readCallback), handler = move(handler), handleType = handleType]() mutable
					{
						readCallback(unique_ptr<ReadFileHandle>(static_cast<ReadFileHandle*>(this->createHandle(pathToFile, handleType))));

						handler.requestPromise.set_value();
					}
				);
			}
			else if (request == requestType::write)
			{
				{
					unique_lock<mutex> filesLock(filesMutex);
					filePathState& fileState = files[pathToFile];

					if (fileState.isWriteRequest || fileState.readRequests)
					{
						return;
					}

					fileState.isWriteRequest = true;

					cache.clear(pathToFile);
				}

				function<void(unique_ptr<WriteFileHandle>&&)> writeCallback = move(get<function<void(unique_ptr<WriteFileHandle>&&)>>(request.callback));
				RequestPromiseHandler handler(move(request.requestPromise));
				requestFileHandleType handleType = request.handleType;

				requestsQueue.pop();

				threadPool->addTask
				(
					[this, pathToFile, writeCallback = move(writeCallback), handler = move(handler), handleType]() mutable
					{
						writeCallback(unique_ptr<WriteFileHandle>(static_cast<WriteFileHandle*>(this->createHandle(pathToFile, handleType))));

						handler.requestPromise.set_value();
					}
				);

				return;
			}
		}
	}

	void FileManager::decreaseReadRequests(const filesystem::path& pathToFile)
	{
		unique_lock<mutex> filesLock(filesMutex);

		files[pathToFile].readRequests--;
	}

	void FileManager::completeWriteRequest(const filesystem::path& pathToFile)
	{
		unique_lock<mutex> filesLock(filesMutex);

		files[pathToFile].isWriteRequest = false;
	}

	FileManager::FileManager() :
		threadPool(new threading::ThreadPool())
	{

	}

	FileManager::~FileManager()
	{
		delete threadPool;

		threadPool = nullptr;
	}

	future<void> FileManager::addReadRequest(const filesystem::path& pathToFile, const function<void(unique_ptr<ReadFileHandle>&&)>& callback, requestFileHandleType handleType, bool isWait)
	{
		this->addFile(pathToFile);

		promise<void> requestPromise;
		future<void> isReady = requestPromise.get_future();

		this->addRequest(pathToFile, callback, move(requestPromise), handleType);

		this->processQueue(pathToFile);

		if (isWait)
		{
			isReady.wait();
		}

		return isReady;
	}

	future<void> FileManager::addWriteRequest(const filesystem::path& pathToFile, const function<void(unique_ptr<WriteFileHandle>&&)>& callback, requestFileHandleType handleType, bool isWait)
	{
		this->addFile(pathToFile, false);

		promise<void> requestPromise;
		future<void> isReady = requestPromise.get_future();

		this->addRequest(pathToFile, callback, move(requestPromise), handleType);

		this->processQueue(pathToFile);

		if (isWait)
		{
			isReady.wait();
		}

		return isReady;
	}

	FileManager& FileManager::getInstance()
	{
		static FileManager instance;

		return instance;
	}

	void FileManager::addFile(const filesystem::path& pathToFile, bool isFileAlreadyExist)
	{
		if (isFileAlreadyExist)
		{
			if (!filesystem::exists(pathToFile))
			{
				unique_lock<mutex> filesLock(filesMutex);

				files.erase(pathToFile);

				throw exceptions::FileDoesNotExistException(pathToFile);
			}

			if (!filesystem::is_regular_file(pathToFile))
			{
				throw exceptions::NotAFileException(pathToFile);
			}
		}

		unique_lock<mutex> filesLock(filesMutex);

		if (files.find(pathToFile) == files.end())
		{
			files[pathToFile] = filePathState();
		}
	}

	future<void> FileManager::readFile(const filesystem::path& pathToFile, const function<void(unique_ptr<ReadFileHandle>&&)>& callback, bool isWait)
	{
		return this->addReadRequest(pathToFile, callback, requestFileHandleType::read, isWait);
	}

	future<void> FileManager::readBinaryFile(const filesystem::path& pathToFile, const function<void(unique_ptr<ReadFileHandle>&&)>& callback, bool isWait)
	{
		return this->addReadRequest(pathToFile, callback, requestFileHandleType::readBinary, isWait);
	}

	future<void> FileManager::writeFile(const filesystem::path& pathToFile, const function<void(unique_ptr<WriteFileHandle>&&)>& callback, bool isWait)
	{
		return this->addWriteRequest(pathToFile, callback, requestFileHandleType::write, isWait);
	}

	future<void> FileManager::appendFile(const filesystem::path& pathToFile, const function<void(unique_ptr<WriteFileHandle>&&)>& callback, bool isWait)
	{
		return this->addWriteRequest(pathToFile, callback, requestFileHandleType::append, isWait);
	}

	future<void> FileManager::writeBinaryFile(const filesystem::path& pathToFile, const function<void(unique_ptr<WriteFileHandle>&&)>& callback, bool isWait)
	{
		return this->addWriteRequest(pathToFile, callback, requestFileHandleType::writeBinary, isWait);
	}

	future<void> FileManager::appendBinaryFile(const filesystem::path& pathToFile, const function<void(unique_ptr<WriteFileHandle>&&)>& callback, bool isWait)
	{
		return this->addWriteRequest(pathToFile, callback, requestFileHandleType::appendBinary, isWait);
	}

	Cache& FileManager::getCache()
	{
		return cache;
	}

	const Cache& FileManager::getCache() const
	{
		return cache;
	}
}

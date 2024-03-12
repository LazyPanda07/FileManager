#include "FileManager.h"

#include <iostream>

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
	FileManager::requestStruct::requestStruct(fileCallback&& callback, promise<void>&& requestPromise, requestFileHandleType handleType) :
		callback(move(callback)),
		requestPromise(move(requestPromise)),
		handleType(handleType)
	{

	}

	bool operator == (const FileManager::requestStruct& request, FileManager::requestType type)
	{
		return request.callback.index() == static_cast<size_t>(type);
	}

	FileManager::FileNode::FilePathState::FilePathState() :
		readRequests(0),
		isWriteRequest(false)
	{

	}

	void FileManager::FileNode::addRequest(fileCallback&& callback, std::promise<void>&& requestPromise, requestFileHandleType handleType)
	{
		unique_lock<mutex> lock(requestsMutex);

		requests.emplace(move(callback), move(requestPromise), handleType);
	}

	void FileManager::FileNode::processQueue(const filesystem::path& filePath)
	{
		unique_lock<mutex> lock(requestsMutex);
		FileManager& manager = FileManager::getInstance();

		while (requests.size())
		{
			requestStruct& request = requests.front();

			if (request == requestType::read)
			{
				if (state.isWriteRequest)
				{
					return;
				}

				state.readRequests++;

				function<void(unique_ptr<ReadFileHandle>&&)> readCallback = move(get<function<void(unique_ptr<ReadFileHandle>&&)>>(request.callback));
				RequestPromiseHandler handler(move(request.requestPromise));
				requestFileHandleType handleType = request.handleType;

				requests.pop();

				manager.threadPool->addTask
				(
					[this, &manager, filePath, readCallback = move(readCallback), handler = move(handler), handleType = handleType]() mutable
					{
						readCallback(unique_ptr<ReadFileHandle>(static_cast<ReadFileHandle*>(manager.createHandle(filePath, handleType))));

						handler.requestPromise.set_value();
					}
				);
			}
			else if (request == requestType::write)
			{
				if (state.isWriteRequest || state.readRequests)
				{
					return;
				}

				state.isWriteRequest = true;

				manager.cache.clear(filePath);

				function<void(unique_ptr<WriteFileHandle>&&)> writeCallback = move(get<function<void(unique_ptr<WriteFileHandle>&&)>>(request.callback));
				RequestPromiseHandler handler(move(request.requestPromise));
				requestFileHandleType handleType = request.handleType;

				requests.pop();

				manager.threadPool->addTask
				(
					[this, &manager, filePath, writeCallback = move(writeCallback), handler = move(handler), handleType]() mutable
					{
						writeCallback(unique_ptr<WriteFileHandle>(static_cast<WriteFileHandle*>(manager.createHandle(filePath, handleType))));

						handler.requestPromise.set_value();
					}
				);

				return;
			}
		}
	}

	void FileManager::NodesContainer::addNode(const filesystem::path& filePath)
	{
		unique_lock<mutex> lock(readWriteMutex);

		if (data.contains(filePath))
		{
			return;
		}

		data[filePath] = new FileNode();
	}

	FileManager::FileNode* FileManager::NodesContainer::operator [](const filesystem::path& filePath) const
	{
		unique_lock<mutex> lock(readWriteMutex);

		return data.at(filePath);
	}

	FileManager::NodesContainer::~NodesContainer()
	{
		for (const auto& [key, value] : data)
		{
			delete value;
		}

		data.clear();
	}

	FileManager::FileManagerPtr FileManager::instance = FileManager::FileManagerPtr(nullptr, [](FileManager*) {});

	void FileManager::threadPoolCallback(promise<void>&& requestPromise)
	{
		requestPromise.set_value();
	}

	void FileManager::deleter(FileManager* instance)
	{
		delete instance;
	}

	FileHandle* FileManager::createHandle(const filesystem::path& filePath, requestFileHandleType handleType)
	{
		switch (handleType)
		{
		case file_manager::FileManager::requestFileHandleType::read:
			return new ReadFileHandle(filePath);

		case file_manager::FileManager::requestFileHandleType::write:
			return new WriteFileHandle(filePath);

		case file_manager::FileManager::requestFileHandleType::readBinary:
			return new ReadBinaryFileHandle(filePath);

		case file_manager::FileManager::requestFileHandleType::writeBinary:
			return new WriteBinaryFileHandle(filePath);

		case file_manager::FileManager::requestFileHandleType::append:
			return new AppendFileHandle(filePath);

		case file_manager::FileManager::requestFileHandleType::appendBinary:
			return new AppendBinaryFileHandle(filePath);
		}

		return new FileHandle(filePath, ios_base::in);
	}

	void FileManager::notify(filesystem::path&& filePath, ios_base::openmode mode)
	{
		threadPool->addTask([this, tem = move(filePath)]()
			{
				nodes[tem]->processQueue(tem);
			});
	}

	void FileManager::addRequest(const filesystem::path& filePath, fileCallback&& callback, promise<void>&& requestPromise, requestFileHandleType handleType)
	{
		FileNode* node = nodes[filePath];

		node->addRequest(move(callback), move(requestPromise), handleType);

		node->processQueue(filePath);
	}

	void FileManager::decreaseReadRequests(const filesystem::path& filePath)
	{
		nodes[filePath]->state.readRequests--;
	}

	void FileManager::completeWriteRequest(const filesystem::path& filePath)
	{
		nodes[filePath]->state.isWriteRequest = false;
	}

	FileManager::FileManager() :
		threadPool(nullptr),
		isThreadPoolWeak(true)
	{

	}

	FileManager::FileManager(size_t threadsNumber) :
		threadPool(new threading::ThreadPool(threadsNumber)),
		isThreadPoolWeak(false)
	{

	}

	FileManager::FileManager(threading::ThreadPool* threadPool) :
		threadPool(threadPool),
		isThreadPoolWeak(true)
	{

	}

	FileManager::~FileManager()
	{
		if (!isThreadPoolWeak)
		{
			delete threadPool;
		}

		threadPool = nullptr;
	}

	future<void> FileManager::addReadRequest(const filesystem::path& filePath, const function<void(unique_ptr<ReadFileHandle>&&)>& callback, requestFileHandleType handleType, bool isWait)
	{
		this->addFile(filePath);

		promise<void> requestPromise;
		future<void> isReady = requestPromise.get_future();

		this->addRequest(filePath, callback, move(requestPromise), handleType);

		if (isWait)
		{
			isReady.wait();
		}

		return isReady;
	}

	future<void> FileManager::addWriteRequest(const filesystem::path& filePath, const function<void(unique_ptr<WriteFileHandle>&&)>& callback, requestFileHandleType handleType, bool isWait)
	{
		this->addFile(filePath, false);

		promise<void> requestPromise;
		future<void> isReady = requestPromise.get_future();

		this->addRequest(filePath, callback, move(requestPromise), handleType);

		if (isWait)
		{
			isReady.wait();
		}

		return isReady;
	}

	FileManager& FileManager::getInstance()
	{
		if (!instance)
		{
			unique_lock<mutex> lock(FileManager::instanceMutex);

			instance = FileManagerPtr(new FileManager(thread::hardware_concurrency()), &FileManager::deleter);
		}

		return *instance;
	}

	FileManager& FileManager::getInstance(size_t threadsNumber)
	{
		if (!instance)
		{
			unique_lock<mutex> lock(FileManager::instanceMutex);

			instance = FileManagerPtr(new FileManager(threadsNumber), &FileManager::deleter);
		}

		if (instance->threadPool->getThreadsCount() != threadsNumber)
		{
			delete instance->threadPool;

			instance->threadPool = new threading::ThreadPool(threadsNumber);
		}

		return *instance;
	}

	FileManager& FileManager::getInstance(threading::ThreadPool* threadPool)
	{
		if (!instance)
		{
			unique_lock<mutex> lock(FileManager::instanceMutex);

			instance = FileManagerPtr(new FileManager(threadPool), &FileManager::deleter);
		}

		if (instance->threadPool != threadPool)
		{
			instance->threadPool = threadPool;
		}

		return *instance;
	}

	string FileManager::getVersion()
	{
		string version = "1.3.0";

		return version;
	}

	void FileManager::addFile(const filesystem::path& filePath, bool isFileAlreadyExist)
	{
		if (isFileAlreadyExist)
		{
			if (!filesystem::exists(filePath))
			{
				for (const auto& it : filesystem::directory_iterator(filePath.parent_path()))
				{
					cout << it << endl;
				}

				throw exceptions::FileDoesNotExistException(filePath);
			}

			if (!filesystem::is_regular_file(filePath))
			{
				throw exceptions::NotAFileException(filePath);
			}
		}

		nodes.addNode(filePath);
	}

	future<void> FileManager::readFile(const filesystem::path& filePath, const function<void(unique_ptr<ReadFileHandle>&&)>& callback, bool isWait)
	{
		return this->addReadRequest(filePath, callback, requestFileHandleType::read, isWait);
	}

	future<void> FileManager::readBinaryFile(const filesystem::path& filePath, const function<void(unique_ptr<ReadFileHandle>&&)>& callback, bool isWait)
	{
		return this->addReadRequest(filePath, callback, requestFileHandleType::readBinary, isWait);
	}

	future<void> FileManager::writeFile(const filesystem::path& filePath, const function<void(unique_ptr<WriteFileHandle>&&)>& callback, bool isWait)
	{
		return this->addWriteRequest(filePath, callback, requestFileHandleType::write, isWait);
	}

	future<void> FileManager::appendFile(const filesystem::path& filePath, const function<void(unique_ptr<WriteFileHandle>&&)>& callback, bool isWait)
	{
		return this->addWriteRequest(filePath, callback, requestFileHandleType::append, isWait);
	}

	future<void> FileManager::writeBinaryFile(const filesystem::path& filePath, const function<void(unique_ptr<WriteFileHandle>&&)>& callback, bool isWait)
	{
		return this->addWriteRequest(filePath, callback, requestFileHandleType::writeBinary, isWait);
	}

	future<void> FileManager::appendBinaryFile(const filesystem::path& filePath, const function<void(unique_ptr<WriteFileHandle>&&)>& callback, bool isWait)
	{
		return this->addWriteRequest(filePath, callback, requestFileHandleType::appendBinary, isWait);
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

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

static unique_ptr<file_manager::FileManager> instance;
static mutex instanceMutex;

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
	FileManager::RequestStruct::RequestStruct(FileCallback&& callback, promise<void>&& requestPromise, RequestFileHandleType handleType) :
		callback(move(callback)),
		requestPromise(move(requestPromise)),
		handleType(handleType)
	{

	}

	bool operator == (const FileManager::RequestStruct& request, FileManager::RequestType type)
	{
		return request.callback.index() == static_cast<size_t>(type);
	}

	FileManager::FileNode::FilePathState::FilePathState() :
		readRequests(0),
		isWriteRequest(false)
	{

	}

	void FileManager::FileNode::addRequest(FileCallback&& callback, std::promise<void>&& requestPromise, RequestFileHandleType handleType)
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
			RequestStruct& request = requests.front();

			if (request == RequestType::read)
			{
				if (state.isWriteRequest)
				{
					return;
				}

				state.readRequests++;

				function<void(unique_ptr<ReadFileHandle>&&)> readCallback = move(get<function<void(unique_ptr<ReadFileHandle>&&)>>(request.callback));
				RequestPromiseHandler handler(move(request.requestPromise));
				RequestFileHandleType handleType = request.handleType;

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
			else if (request == RequestType::write)
			{
				if (state.isWriteRequest || state.readRequests)
				{
					return;
				}

				state.isWriteRequest = true;

				manager.cache.clear(filePath);

				function<void(unique_ptr<WriteFileHandle>&&)> writeCallback = move(get<function<void(unique_ptr<WriteFileHandle>&&)>>(request.callback));
				RequestPromiseHandler handler(move(request.requestPromise));
				RequestFileHandleType handleType = request.handleType;

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

	void FileManager::threadPoolCallback(promise<void>&& requestPromise)
	{
		requestPromise.set_value();
	}

	FileHandle* FileManager::createHandle(const filesystem::path& filePath, RequestFileHandleType handleType)
	{
		switch (handleType)
		{
		case file_manager::FileManager::RequestFileHandleType::read:
			return new ReadFileHandle(filePath);

		case file_manager::FileManager::RequestFileHandleType::write:
			return new WriteFileHandle(filePath);

		case file_manager::FileManager::RequestFileHandleType::readBinary:
			return new ReadBinaryFileHandle(filePath);

		case file_manager::FileManager::RequestFileHandleType::writeBinary:
			return new WriteBinaryFileHandle(filePath);

		case file_manager::FileManager::RequestFileHandleType::append:
			return new AppendFileHandle(filePath);

		case file_manager::FileManager::RequestFileHandleType::appendBinary:
			return new AppendBinaryFileHandle(filePath);
		}

		return new FileHandle(filePath, ios_base::in);
	}

	void FileManager::notify(filesystem::path&& filePath)
	{
		threadPool->addTask([this, tem = move(filePath)]()
			{
				nodes[tem]->processQueue(tem);
			});
	}

	void FileManager::addRequest(const filesystem::path& filePath, FileCallback&& callback, promise<void>&& requestPromise, RequestFileHandleType handleType)
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
		threadPool(nullptr)
	{

	}

	FileManager::FileManager(size_t threadsNumber) :
		threadPool(new threading::ThreadPool(threadsNumber))
	{

	}

	FileManager::FileManager(shared_ptr<threading::ThreadPool> threadPool) :
		threadPool(threadPool)
	{

	}

	future<void> FileManager::addReadRequest(const filesystem::path& filePath, const function<void(unique_ptr<ReadFileHandle>&&)>& callback, RequestFileHandleType handleType, bool isWait)
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

	future<void> FileManager::addWriteRequest(const filesystem::path& filePath, const function<void(unique_ptr<WriteFileHandle>&&)>& callback, RequestFileHandleType handleType, bool isWait)
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
			constexpr size_t defaultThreadsNumber = 2;
			unique_lock<mutex> lock(instanceMutex);

			instance = unique_ptr<FileManager>(new FileManager(defaultThreadsNumber));
		}

		return *instance;
	}

	FileManager& FileManager::getInstance(size_t threadsNumber)
	{
		if (!instance)
		{
			unique_lock<mutex> lock(instanceMutex);

			instance = unique_ptr<FileManager>(new FileManager(threadsNumber));
		}

		if (instance->threadPool->getThreadsCount() != threadsNumber)
		{
			instance->threadPool = make_shared<threading::ThreadPool>(threadsNumber);
		}

		return *instance;
	}

	FileManager& FileManager::getInstance(shared_ptr<threading::ThreadPool> threadPool)
	{
		if (!instance)
		{
			unique_lock<mutex> lock(instanceMutex);

			instance = unique_ptr<FileManager>(new FileManager(threadPool));
		}

		if (instance->threadPool != threadPool)
		{
			instance->threadPool = threadPool;
		}

		return *instance;
	}

	string FileManager::getVersion()
	{
		string version = "1.7.0";

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
		return this->addReadRequest(filePath, callback, RequestFileHandleType::read, isWait);
	}

	future<void> FileManager::readBinaryFile(const filesystem::path& filePath, const function<void(unique_ptr<ReadFileHandle>&&)>& callback, bool isWait)
	{
		return this->addReadRequest(filePath, callback, RequestFileHandleType::readBinary, isWait);
	}

	future<void> FileManager::writeFile(const filesystem::path& filePath, const function<void(unique_ptr<WriteFileHandle>&&)>& callback, bool isWait)
	{
		return this->addWriteRequest(filePath, callback, RequestFileHandleType::write, isWait);
	}

	future<void> FileManager::appendFile(const filesystem::path& filePath, const function<void(unique_ptr<WriteFileHandle>&&)>& callback, bool isWait)
	{
		return this->addWriteRequest(filePath, callback, RequestFileHandleType::append, isWait);
	}

	future<void> FileManager::writeBinaryFile(const filesystem::path& filePath, const function<void(unique_ptr<WriteFileHandle>&&)>& callback, bool isWait)
	{
		return this->addWriteRequest(filePath, callback, RequestFileHandleType::writeBinary, isWait);
	}

	future<void> FileManager::appendBinaryFile(const filesystem::path& filePath, const function<void(unique_ptr<WriteFileHandle>&&)>& callback, bool isWait)
	{
		return this->addWriteRequest(filePath, callback, RequestFileHandleType::appendBinary, isWait);
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

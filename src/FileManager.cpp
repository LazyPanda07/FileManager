#include "FileManager.h"

#include <iostream>

#include "Handlers/ReadBinaryFileHandle.h"
#include "Handlers/WriteBinaryFileHandle.h"
#include "Handlers/AppendFileHandle.h"
#include "Handlers/AppendBinaryFileHandle.h"

#include "Exceptions/FileDoesNotExistException.h"
#include "Exceptions/NotAFileException.h"

#include "ThreadPool.h"

static std::unique_ptr<file_manager::FileManager> instance;
static std::mutex instanceMutex;

struct RequestPromiseHandler
{
	std::promise<void> requestPromise;

	RequestPromiseHandler(std::promise<void>&& requestPromise) :
		requestPromise(std::move(requestPromise))
	{

	}

	RequestPromiseHandler(const RequestPromiseHandler& other)
	{
		(*this) = other;
	}

	RequestPromiseHandler& operator = (const RequestPromiseHandler& other)
	{
		requestPromise = std::move(const_cast<RequestPromiseHandler&>(other).requestPromise);

		return *this;
	}
};

namespace file_manager
{
	FileManager::RequestStruct::RequestStruct(FileCallback&& callback, std::promise<void>&& requestPromise, RequestFileHandleType handleType) :
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
		std::lock_guard<std::mutex> lock(requestsMutex);

		requests.emplace(std::move(callback), std::move(requestPromise), handleType);
	}

	void FileManager::FileNode::processQueue(const std::filesystem::path& filePath)
	{
		std::lock_guard<std::mutex> lock(requestsMutex);
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

				std::function<void(std::unique_ptr<ReadFileHandle>&&)> readCallback = std::move(std::get<std::function<void(std::unique_ptr<ReadFileHandle>&&)>>(request.callback));
				RequestPromiseHandler handler(move(request.requestPromise));
				RequestFileHandleType handleType = request.handleType;

				requests.pop();

				manager.threadPool->addTask
				(
					[this, &manager, filePath, readCallback = std::move(readCallback), handler = std::move(handler), handleType = handleType]() mutable
					{
						readCallback(std::unique_ptr<ReadFileHandle>(static_cast<ReadFileHandle*>(manager.createHandle(filePath, handleType))));

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

				std::function<void(std::unique_ptr<WriteFileHandle>&&)> writeCallback = std::move(std::get<std::function<void(std::unique_ptr<WriteFileHandle>&&)>>(request.callback));
				RequestPromiseHandler handler(move(request.requestPromise));
				RequestFileHandleType handleType = request.handleType;

				requests.pop();

				manager.threadPool->addTask
				(
					[this, &manager, filePath, writeCallback = std::move(writeCallback), handler = std::move(handler), handleType]() mutable
					{
						writeCallback(std::unique_ptr<WriteFileHandle>(static_cast<WriteFileHandle*>(manager.createHandle(filePath, handleType))));

						handler.requestPromise.set_value();
					}
				);

				return;
			}
		}
	}

	void FileManager::NodesContainer::addNode(const std::filesystem::path& filePath)
	{
		std::lock_guard<std::mutex> lock(readWriteMutex);

		if (data.contains(filePath))
		{
			return;
		}

		data.try_emplace(filePath, new FileNode());
	}

	FileManager::FileNode* FileManager::NodesContainer::operator [](const std::filesystem::path& filePath) const
	{
		std::lock_guard<std::mutex> lock(readWriteMutex);

		return data.at(filePath);
	}

	FileHandle* FileManager::createHandle(const std::filesystem::path& filePath, RequestFileHandleType handleType)
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

		return new FileHandle(filePath, std::ios_base::in);
	}

	void FileManager::notify(std::filesystem::path&& filePath)
	{
		threadPool->addTask([this, tem = std::move(filePath)]()
			{
				nodes[tem]->processQueue(tem);
			});
	}

	void FileManager::addRequest(const std::filesystem::path& filePath, FileCallback&& callback, std::promise<void>&& requestPromise, RequestFileHandleType handleType)
	{
		FileNode* node = nodes[filePath];

		node->addRequest(std::move(callback), std::move(requestPromise), handleType);

		node->processQueue(filePath);
	}

	void FileManager::decreaseReadRequests(const std::filesystem::path& filePath)
	{
		nodes[filePath]->state.readRequests--;
	}

	void FileManager::completeWriteRequest(const std::filesystem::path& filePath)
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

	FileManager::FileManager(std::shared_ptr<threading::ThreadPool> threadPool) :
		threadPool(threadPool)
	{

	}

	std::future<void> FileManager::addReadRequest(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<ReadFileHandle>&&)>& callback, RequestFileHandleType handleType, bool wait)
	{
		this->addFile(filePath);

		std::promise<void> requestPromise;
		std::future<void> isReady = requestPromise.get_future();

		this->addRequest(filePath, callback, std::move(requestPromise), handleType);

		if (wait)
		{
			isReady.wait();
		}

		return isReady;
	}

	std::future<void> FileManager::addWriteRequest(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<WriteFileHandle>&&)>& callback, RequestFileHandleType handleType, bool wait)
	{
		this->addFile(filePath, false);

		std::promise<void> requestPromise;
		std::future<void> isReady = requestPromise.get_future();

		this->addRequest(filePath, callback, std::move(requestPromise), handleType);

		if (wait)
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
			std::lock_guard<std::mutex> lock(instanceMutex);

			instance = std::unique_ptr<FileManager>(new FileManager(defaultThreadsNumber));
		}

		return *instance;
	}

	FileManager& FileManager::getInstance(size_t threadsNumber)
	{
		if (!instance)
		{
			std::lock_guard<std::mutex> lock(instanceMutex);

			instance = std::unique_ptr<FileManager>(new FileManager(threadsNumber));
		}

		if (instance->threadPool->getThreadsCount() != threadsNumber)
		{
			instance->threadPool = std::make_shared<threading::ThreadPool>(threadsNumber);
		}

		return *instance;
	}

	FileManager& FileManager::getInstance(std::shared_ptr<threading::ThreadPool> threadPool)
	{
		if (!instance)
		{
			std::lock_guard<std::mutex> lock(instanceMutex);

			instance = std::unique_ptr<FileManager>(new FileManager(threadPool));
		}

		if (instance->threadPool != threadPool)
		{
			instance->threadPool = threadPool;
		}

		return *instance;
	}

	std::string FileManager::getVersion()
	{
		std::string version = "1.8.1";

		return version;
	}

	void FileManager::addFile(const std::filesystem::path& filePath, bool isFileAlreadyExist)
	{
		if (isFileAlreadyExist)
		{
			if (!std::filesystem::exists(filePath))
			{
				for (const auto& it : std::filesystem::directory_iterator(filePath.parent_path()))
				{
					std::cout << it << std::endl;
				}

				throw exceptions::FileDoesNotExistException(filePath);
			}

			if (!std::filesystem::is_regular_file(filePath))
			{
				throw exceptions::NotAFileException(filePath);
			}
		}

		nodes.addNode(filePath);
	}

	std::future<void> FileManager::readFile(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<ReadFileHandle>&&)>& callback, bool wait)
	{
		return this->addReadRequest(filePath, callback, RequestFileHandleType::read, wait);
	}

	std::future<void> FileManager::readBinaryFile(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<ReadFileHandle>&&)>& callback, bool wait)
	{
		return this->addReadRequest(filePath, callback, RequestFileHandleType::readBinary, wait);
	}

	std::future<void> FileManager::writeFile(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<WriteFileHandle>&&)>& callback, bool wait)
	{
		return this->addWriteRequest(filePath, callback, RequestFileHandleType::write, wait);
	}

	std::future<void> FileManager::appendFile(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<WriteFileHandle>&&)>& callback, bool wait)
	{
		return this->addWriteRequest(filePath, callback, RequestFileHandleType::append, wait);
	}

	std::future<void> FileManager::writeBinaryFile(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<WriteFileHandle>&&)>& callback, bool wait)
	{
		return this->addWriteRequest(filePath, callback, RequestFileHandleType::writeBinary, wait);
	}

	std::future<void> FileManager::appendBinaryFile(const std::filesystem::path& filePath, const std::function<void(std::unique_ptr<WriteFileHandle>&&)>& callback, bool wait)
	{
		return this->addWriteRequest(filePath, callback, RequestFileHandleType::appendBinary, wait);
	}

	std::future<void> FileManager::removeFile(const std::filesystem::path& filePath, bool wait)
	{
		return this->addWriteRequest
		(
			filePath, 
			[this](std::unique_ptr<file_manager::WriteFileHandle>&& handle)
			{
				const std::filesystem::path& path = handle->getPathToFile();

				std::filesystem::remove(path);

				cache.clear(path);
			},
			RequestFileHandleType::write, 
			wait
		);
	}

	bool FileManager::exists(const std::filesystem::path& filePath) const
	{
		return std::filesystem::exists(filePath);
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

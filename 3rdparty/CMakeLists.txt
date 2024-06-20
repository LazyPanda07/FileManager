cmake_minimum_required(VERSION 3.27.0)

project(3rdparty)

set(THREAD_POOL_TAG v1.5.3)
set(THREAD_POOL_PATH ${PROJECT_SOURCE_DIR}/ThreadPool)
set(THREAD_POOL_INCLUDE ${PROJECT_SOURCE_DIR}/ThreadPool/src/ PARENT_SCOPE)

if (EXISTS ${THREAD_POOL_PATH})
	execute_process(
		WORKING_DIRECTORY ${THREAD_POOL_PATH}
		COMMAND git pull
	)

	execute_process(
		WORKING_DIRECTORY ${THREAD_POOL_PATH}
		COMMAND git checkout ${THREAD_POOL_TAG}
	)
else()
	execute_process(
		COMMAND git clone https://github.com/LazyPanda07/ThreadPool -b ${THREAD_POOL_TAG} ${THREAD_POOL_PATH}
	)
endif()

add_subdirectory(ThreadPool)

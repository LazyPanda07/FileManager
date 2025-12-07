cmake_minimum_required(VERSION 3.27.0)

set(FETCHCONTENT_QUIET OFF CACHE BOOL "")
set(THREAD_POOL_VERSION 1.8.0)

project(3rdparty)

include(FetchContent)

if (NOT TARGET ThreadPool)
	FetchContent_Declare(
		ThreadPool
		GIT_REPOSITORY https://github.com/LazyPanda07/ThreadPool.git
		GIT_TAG v${THREAD_POOL_VERSION}
	)

	FetchContent_MakeAvailable(ThreadPool)
endif()

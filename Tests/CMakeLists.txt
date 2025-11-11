cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)
set(DLL ${CMAKE_SOURCE_DIR}/../FileManager)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
set(GTEST_VERSION 1.17.0)

if (UNIX)
	add_definitions(-D__LINUX__)

	set(DLL ${DLL}/lib/libFileManager.so)
else ()
	set(DLL ${DLL}/dll/FileManager.dll)
endif (UNIX)

project(Tests)

include(FetchContent)

FetchContent_Declare(
	gtest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG v${GTEST_VERSION}
)

FetchContent_MakeAvailable(gtest)

add_executable(
	${PROJECT_NAME}
	main.cpp
	src/ReadTests.cpp
	src/WriteTests.cpp
)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	${CMAKE_SOURCE_DIR}/../FileManager/include
)

target_link_directories(
	${PROJECT_NAME} PUBLIC
	${CMAKE_SOURCE_DIR}/../FileManager/lib
)

target_link_libraries(
	${PROJECT_NAME} PUBLIC
	FileManager
	ThreadPool
	gtest
	gtest_main
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES ${DLL} DESTINATION bin)

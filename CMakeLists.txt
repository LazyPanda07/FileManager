cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)

if (UNIX)
	add_definitions(-D__LINUX__)

	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif (UNIX)

project(FileManager VERSION 1.8.3)

add_library(
	${PROJECT_NAME} SHARED
	src/Cache.cpp
	src/FileManager.cpp
	src/Utility.cpp
	src/Exceptions/BaseFileManagerException.cpp
	src/Exceptions/FileDoesNotExistException.cpp
	src/Exceptions/NotAFileException.cpp
	src/Handlers/AppendBinaryFileHandle.cpp
	src/Handlers/AppendFileHandle.cpp
	src/Handlers/FileHandle.cpp
	src/Handlers/ReadBinaryFileHandle.cpp
	src/Handlers/ReadFileHandle.cpp
	src/Handlers/WriteBinaryFileHandle.cpp
	src/Handlers/WriteFileHandle.cpp
)

if (DEFINED ENV{MARCH} AND NOT "$ENV{MARCH}" STREQUAL "")
	target_compile_options(${PROJECT_NAME} PRIVATE -march=$ENV{MARCH})
endif()

add_subdirectory(3rdparty)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	include
)

target_link_libraries(
	${PROJECT_NAME} PRIVATE
	ThreadPool
)

install(
	TARGETS ${PROJECT_NAME}
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION dll
)

install(DIRECTORY include DESTINATION .)

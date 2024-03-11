cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)
set(LINK_LIBRARY_PATH dependencies/ThreadPool/lib)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install-dir)
option(BUILD_SHARED_LIBS "" OFF)

if (UNIX)
	set(LINK_LIBRARY_PATH ${LINK_LIBRARY_PATH}/Linux)

	add_definitions(-D__LINUX__)

	install(FILES ${LINK_LIBRARY_PATH}/libThreadPool.a DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
else ()
	if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		set(LINK_LIBRARY_PATH ${LINK_LIBRARY_PATH}/Windows/debug)
	else ()
		set(LINK_LIBRARY_PATH ${LINK_LIBRARY_PATH}/Windows/release)
	endif()

	install(FILES ${LINK_LIBRARY_PATH}/ThreadPool.lib DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
endif (UNIX)

if (${BUILD_SHARED_LIBS})
	add_definitions(-DFILE_MANAGER_DLL)
endif ()

project(FileManager)

add_library(
	${PROJECT_NAME}
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

target_include_directories(
	${PROJECT_NAME} PUBLIC
	src/
	dependencies/ThreadPool/include
)

target_link_directories(
	${PROJECT_NAME} PRIVATE
	${LINK_LIBRARY_PATH}
)

target_link_libraries(
	${PROJECT_NAME} PRIVATE
	ThreadPool
)

install(
	TARGETS ${PROJECT_NAME}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/dll
)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
	install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include FILES_MATCHING PATTERN "*.h")
endif ()

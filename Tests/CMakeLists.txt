cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)
set(LINK_LIBRARY_PATH ../dependencies/ThreadPool/lib)
set(DLL ${CMAKE_SOURCE_DIR}/../FileManager)

if (UNIX)
	add_definitions(-D__LINUX__)

	set(LINK_LIBRARY_PATH ${LINK_LIBRARY_PATH}/Linux)
	set(DLL ${DLL}/lib/libFileManager.so)
else ()
	if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		set(LINK_LIBRARY_PATH ${LINK_LIBRARY_PATH}/Windows/debug)
	else ()
		set(LINK_LIBRARY_PATH ${LINK_LIBRARY_PATH}/Windows/release)
	endif ()

	set(DLL ${DLL}/dll/FileManager.dll)
endif (UNIX)

project(Tests)

add_executable(
	${PROJECT_NAME}
	main.cpp
	src/ReadTests.cpp
	src/WriteTests.cpp
)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	${CMAKE_SOURCE_DIR}/../src
	${CMAKE_SOURCE_DIR}/../dependencies/ThreadPool/include
	gtest/include
)

target_link_directories(
	${PROJECT_NAME} PUBLIC
	${CMAKE_SOURCE_DIR}/../FileManager/lib
	${LINK_LIBRARY_PATH}
	gtest/lib
)

target_link_libraries(
	${PROJECT_NAME} PUBLIC
	FileManager
	ThreadPool
	gtest
	gtest_main
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR}/bin)
install(FILES ${DLL} DESTINATION ${CMAKE_BINARY_DIR}/bin)

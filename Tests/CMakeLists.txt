cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)
set(LINK_LIBRARY_PATH ../dependencies/ThreadPool/lib)
set(SHARED_OBJECT ../FileManager)

if (UNIX)
	set(LINK_LIBRARY_PATH ${LINK_LIBRARY_PATH}/Linux)

	set(SHARED_OBJECT ${SHARED_OBJECT}/lib/libFileManager.so)

	add_definitions(-D__LINUX__)
else ()
	if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		set(LINK_LIBRARY_PATH ${LINK_LIBRARY_PATH}/Windows/debug)
	else ()
		set(LINK_LIBRARY_PATH ${LINK_LIBRARY_PATH}/Windows/release)
	endif ()

	set(SHARED_OBJECT ${SHARED_OBJECT}/dll/FileManager.dll)
endif (UNIX)

project(Tests)

add_executable(
	${PROJECT_NAME}
	main.cpp
	src/Functions.cpp
	src/ReadTests.cpp
	src/WriteTests.cpp
)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	../src
	../dependencies/ThreadPool/include
	gtest/include
	include
)

target_link_directories(
	${PROJECT_NAME} PUBLIC
	../FileManager/lib
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

if (EXISTS ${SHARED_OBJECT})
	install(FILES ${SHARED_OBJECT} DESTINATION ${CMAKE_BINARY_DIR}/bin)
endif ()

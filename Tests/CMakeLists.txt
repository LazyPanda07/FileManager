cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 20)
set(DLL ${CMAKE_SOURCE_DIR}/../FileManager)

if (UNIX)
	add_definitions(-D__LINUX__)

	set(DLL ${DLL}/lib/libFileManager.so)
else ()
	set(DLL ${DLL}/dll/FileManager.dll)
endif (UNIX)

project(Tests)

find_package(GTest)

if(NOT ${GTest_FOUND})
	set(GTEST_BOTH_LIBRARIES gtest gtest_main)
	set(GTEST_INCLUDE_DIRS gtest/include/)
endif()

add_executable(
	${PROJECT_NAME}
	main.cpp
	src/ReadTests.cpp
	src/WriteTests.cpp
)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	${CMAKE_SOURCE_DIR}/../FileManager/include
	${GTEST_INCLUDE_DIRS}
)

target_link_directories(
	${PROJECT_NAME} PUBLIC
	${CMAKE_SOURCE_DIR}/../FileManager/lib
	gtest/lib
)

target_link_libraries(
	${PROJECT_NAME} PUBLIC
	FileManager
	ThreadPool
	${GTEST_BOTH_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR}/bin)
install(FILES ${DLL} DESTINATION ${CMAKE_BINARY_DIR}/bin)

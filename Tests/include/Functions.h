#pragma once

#include "FileManager.h"

inline constexpr int threadsCount = 8;
inline constexpr int writes = 512;
inline constexpr int totalWrites = threadsCount * writes;

void appendCallback(std::unique_ptr<file_manager::WriteFileHandle>&& handle);

void threadWrite();

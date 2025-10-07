#pragma once
#include <ctime>
#include <string>
#include <chrono>
#include <cstdio>
#include <filesystem>
#ifndef LF_SIZE_LIMIT
#define LF_SIZE_LIMIT 2 * 1024 * 1024  // 单个日志文件大小控制 2 MB
#endif
#ifndef LF_NUM_LIMIT
#define LF_NUM_LIMIT 1024              // 最多创建多少个日志文件, 超过之后删除最早的日志文件
#endif
#ifndef SL_LEN_LIMIT
#define SL_LEN_LIMIT 4096              // 单条日志的最大长度, 超过该长度的日志将被截断
#endif
#ifndef LF_BUFFER_SIZE
#define LF_BUFFER_SIZE 8192            // 日志缓冲区大小
#endif
#ifndef ROOT_PATH
#define ROOT_PATH ""                   // 项目根目录, 使用预定义宏指定
#endif


#ifdef _WIN32
#include <windows.h>
#include <sys/stat.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <pthread.h>
#include <unistd.h>
#endif

namespace Log {
constexpr int LogFileSizeLimit = LF_SIZE_LIMIT;  
constexpr int LogFilesNumLimit = LF_NUM_LIMIT;   
constexpr int SingleLogLenLimit = SL_LEN_LIMIT;  
constexpr int LogBufferSize = LF_BUFFER_SIZE;  	

inline static std::string GetCurTimeString() noexcept {
	using namespace std::chrono;
	auto now = system_clock::now();
	auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
	auto time_table = system_clock::to_time_t(now);
	tm local_time;
/* std::localtime 因为不安全已经弃用(MSVC) */
#ifdef _WIN32
	gmtime_s(&local_time, &time_table);
#else
	gmtime_r(&time_table, &local_time);
#endif
	static thread_local char buffer[64];
	std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
		local_time.tm_year + 1900, local_time.tm_mon + 1, local_time.tm_mday,
		local_time.tm_hour, local_time.tm_min, local_time.tm_sec,
		static_cast<int>(ms.count()));
	std::string tmp(buffer);
	return std::string(tmp.begin(), tmp.end());
}

inline static uint32_t GetCurProcessId() noexcept {
#ifdef _WIN32
	return static_cast<uint32_t>(::GetCurrentProcessId());
#else
	return static_cast<uint32_t>(::getpid());
#endif
}

inline static uint32_t GetCurThreadId() noexcept {
#ifdef _WIN32
	return static_cast<uint32_t>(::GetCurrentThreadId());
#else
	return static_cast<uint32_t>(::pthread_self());
#endif	
}

inline static std::string GetProjectRootPath() noexcept {
    return ROOT_PATH;
}

/**
* @brief 获取当前进程ID和线程ID字符串
* @return 格式为 "[进程ID::线程ID] " 的字符串
* @note: 此处的线程ID是OS的线程ID, 不是C++11的std::thread::id (标准不保证相等)
*/
inline static std::string GetCurSysIdString() noexcept {
	return "[" + std::to_string(GetCurProcessId()) + "::" + std::to_string(GetCurThreadId()) + "] ";
}



/**
* @brief 根据当前时间生成日志文件名
*/
inline static std::string GenLogFileName() noexcept {

}

/**
* @brief 创建日志文件
* @param log_path 日志文件路径
*/
inline static void CreateLogFile(std::filesystem::path log_path, std::string file_name="") {
	std::string name = file_name.empty() ? GenLogFileName() : file_name;

}
}
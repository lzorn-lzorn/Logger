
#pragma once
#include <mutex>
#include <string>
#include <filesystem>
#include <chrono>
#include <map>
#include <source_location>
#ifndef LF_SIZE_LIMIT
#define LF_SIZE_LIMIT 2 * 1024 * 1024  // 单个日志文件大小控制 2 MB
#endif
#ifndef LF_NUM_LIMIT
#define LF_NUM_LIMIT 1024              // 最多创建多少个日志文件, 超过之后删除最早的日志文件
#endif
#ifndef SL_LEN_LIMIT
#define SL_LEN_LIMIT 4096              // 单条日志的最大长度, 超过该长度的日志将被截断
#endif
namespace Log {
constexpr int LogFileSizeLimit = LF_SIZE_LIMIT;  
constexpr int LogFilesNumLimit = LF_NUM_LIMIT;   
constexpr int SingleLogLenLimit = SL_LEN_LIMIT;  
enum class Level {
	Debug = 0,
	Info = 1,
	Warning = 2,
	Error = 3,
	Fatal = 4
};
constexpr const char* LevelToString(Level level) noexcept {
    switch (level) {
        case Level::Debug:   return "[Debug]";
        case Level::Info:    return "[Info]";
        case Level::Warning: return "[Warning]";
        case Level::Error:   return "[Error]";
        case Level::Fatal:   return "[Fatal]";
        default:             return "[Unknown]";
    }
}

/**
* @brief 日志中每一条日志信息  
*/
struct Message {
	std::source_location location; // 记录日志的源码位置
	std::chrono::system_clock::time_point writing_time; // 记录日志的时间
	std::string content; // 日志内容
    Level level {Level::Info}; // 日志级别 
	
	/**
	* @brief 将日志信息转换为字符串
	* @format [日志等级] [时间 YY-MM-DD HH:MM:SS] [文件名:行号 函数名] 日志内容
	*/
	static std::string ToString() noexcept;
};
 
/**
* @brief 日志文件的内容, 每一条信息用 Message 结构体表示
*/
class Buffer {
	std::map<std::chrono::system_clock::time_point, Message> contents;
	std::mutex mtx_writing; // 用于保护对文件的写访问
};

/**
* @brief 每一个日志文件的记录信息
*/
class Log {
private:
	std::chrono::system_clock::time_point creation_time;
	std::filesystem::path log_path;
	Buffer buffer;
};
class Logger{

	std::map<std::chrono::system_clock::time_point, Log> catalog;
};


}
#pragma once

#include <source_location>
#include <format>
#include "LogLevel.h"
#include "Log.h"
namespace Log {
/**
* @brief 日志中每一条日志信息  
*/
struct Message {
	std::source_location location; // 记录日志的源码位置
	std::string content; // 日志内容
    Level level {Level::Info}; // 日志级别 
};
/**
* @brief 将日志信息转换为字符串
* @format [日志等级] [时间 YY-MM-DD HH:MM:SS] [进程ID::线程ID] [文件名:行号 函数名] 日志内容
*/
inline static std::string MsgToString(const Message& msg) noexcept{
	std::string time = GetCurTimeString();
	std::string sys_id = GetCurSysIdString();
	std::string location = std::string(msg.location.file_name()) + ":" + std::to_string(msg.location.line()) + " " + std::string(msg.location.function_name());
	return "[" + std::string(LevelToString(msg.level)) + "] [" + time + "] [" + sys_id + "] [" + location + "] " + msg.content;
}

template <typename ... Args>
inline static Message MakeMessage(Level level, std::format_string<Args...> fmt, Args&&... args) noexcept {
	return Message{
		std::source_location::current(),
		std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)),
		level
	};
}
inline static Message MakeMessage(Level level, const std::string& content) noexcept {
	return Message{
		std::source_location::current(), 
		content, 
		level
	};
}

}
#pragma once
#include <unordered_map>
#include <string>

#include "Logger.h"
namespace Log {

class LogFileManager {
	using FileName = std::string;
public:
	static LogFileManager& Instance() {
		static LogFileManager instance;
		return instance;
	}
	LogFileManager(const LogFileManager&) = delete;
	LogFileManager& operator=(const LogFileManager&) = delete;
	LogFileManager(LogFileManager&&) = delete;
	LogFileManager& operator=(LogFileManager&&) = delete;

public:
	/**
	 * @brief 合并日志文件
	 */
	void MergeLogFiles();

	/**
	* @brief 恢复日志文件
	*/
	void RestoreLogFiles();

	void ClearAllLogFiles();

	void ClearUnusedLogFiles();

	void ClearMark(Level, const std::string name);
	void ClearMark(Level, const std::filesystem::path path);
private:
	LogFileManager() = default;
	~LogFileManager() = default;

private:
	std::unordered_map<FileName, FileLogger> log_files; // 日志文件名到路径的映射
};
}
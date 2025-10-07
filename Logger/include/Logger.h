
#pragma once

#include <atomic>
#include <deque>
#include <filesystem>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <mutex>

#include "LogBuffer.h"
#include "LogFileWriter.h"
#include "Log.h"
namespace Log {

/**
* @brief 每一个日志文件的记录信息
*/
class Logger {
private:
	std::chrono::system_clock::time_point creation_time;
	std::filesystem::path log_path;
	std::string file_name;
	Buffer<LogBufferSize> buffer;

};

class ConsoleLogger : public Logger {};

class FileLogger : public Logger {
public:
	[[nodiscard]] bool IsValid() const noexcept {
		return is_valid.load(std::memory_order_acquire);
	}

	[[nodiscard]] bool IsRunning() const noexcept {
		return is_running.load(std::memory_order_acquire);
	}
private: 
	FileWriter writer;
	std::mutex mtx;
	std::atomic<bool> is_running {false};
 	/* 控制是否有效: 有效: 即对应文件还存在; 失效: 即对应文件被删除. */
	/* 该字段用于数据恢复 */
	std::atomic<bool> is_valid {true};    
	std::condition_variable cv;
	std::jthread worker;
	std::deque<Buffer<LogBufferSize>> full_buffers;
	std::deque<Buffer<LogBufferSize>> empty_buffers;
	Buffer<LogBufferSize>* current_buffer {nullptr};
};
}
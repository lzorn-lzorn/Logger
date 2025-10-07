#pragma once 
#include <cstddef>
#include <filesystem>
#include <fstream>
#include "LogBuffer.h"
#include "Log.h"

namespace Log {

class FileWriter {
	using LogFilePath = std::filesystem::path;
public:
	FileWriter(LogFilePath);
	FileWriter(const FileWriter&) = delete;
	FileWriter& operator=(const FileWriter&) = delete;
	FileWriter(FileWriter&&) = delete;
	FileWriter& operator=(FileWriter&&) = delete;
	~FileWriter() {
		Flush();
		if(file_stream.is_open()){
			file_stream.close();
		}
	}

public:
	void Write(const char*, std::size_t);
	void Write(const std::string& content){
		Write(content.data(), content.size());
	}
	void Flush(){
		if(file_stream.is_open()){
			file_stream.flush();
		}
	}
	/**
	* @brief 检查当前日志文件是否已满
	*/
	[[nodiscard]] bool IsLogFull() const noexcept {
		return current_file_size >= LogFileSizeLimit;
	}
private:

	
	/**
	* @brief 检查 log_path 的合法性:
	* 	1. 检查父目录是否存在, 不存在则创建
	* 	2. 检查文件是否存在, 不存在则创建
	*/
	void CheckPath();

	bool IsFileExist() const;
	bool IsCatalogueExist() const;


private:
	Buffer<LogBufferSize> buffer;
	LogFilePath log_path;
	std::ofstream file_stream;
	std::size_t current_file_size {0};
};

}
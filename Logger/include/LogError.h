#pragma once
#include <system_error>

namespace Log {
enum class LogError {
	Ok = 0,
	FileNotFound,            // 文件未找到
	FileNotWritable,         // 文件不可写
	FileWritingFailed,       // 文件写入失败
	CatalogueNotFound,       // 日志目录未找到
	CatalogueCreationFailed, // 日志目录创建失败
	RetryLimitExceeded,      // 重试次数超限
	UnknownError             // 未知错误
};
class LogErrorCategory : public std::error_category {
public:
	const char* name() const noexcept override {
		return "LogError";
	}
	std::string message(int ev) const override;
};

inline const LogErrorCategory& GetLogErrorCategory() {
	static LogErrorCategory instance;
	return instance;
}
/**
* @usage: std::error_code ec = MakeLogErrorCode(Log::LogError::FileNotFound);
*/
inline std::error_code MakeLogErrorCode(LogError e){
	return {static_cast<int>(e), GetLogErrorCategory()};
}


}

/**
* @brief 以下代码可以支持 std::error_code ec = Log::LogError::FileNotFound; 这种隐式转换
*/
// namespace std{
// template <>
// struct is_error_code_enum<Log::LogError> : true_type {};
// }
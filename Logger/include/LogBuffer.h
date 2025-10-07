#pragma once

#include <array>
#include <string>
#include <cassert>
#include "LogMessage.h"
namespace Log{

/**
* @brief 日志文件的内容, 每一条信息用 Message 结构体表示
*/
template <size_t SIZE>
class Buffer {
public:
	Buffer() noexcept {
		this->Reset();
	}

	~Buffer() noexcept = default;
public:
	void Write(const std::string& str) noexcept {
		assert(WriteableSize() >= str.size());
		std::copy(str.begin(), str.end(), buffer.begin() + Capacity());
	}

	void Write(const Message& msg) noexcept {
		this->Write(MsgToString(msg));
	}

	[[nodiscard]] bool Writeable() const noexcept {
		return this->Capacity() < MaxSize();
	}

	[[nodiscard]] size_t WriteableSize() const noexcept {
		return MaxSize() - Capacity();
	}

	[[nodiscard]] constexpr size_t MaxSize() const noexcept {
		return SIZE;
	}

	[[nodiscard]] size_t Capacity() const noexcept {
		return buffer.size();
	}

	[[nodiscard]] const char* Data() const noexcept {
		return buffer.data();
	}

	[[nodiscard]] bool Empty() const noexcept {
		return buffer.empty();
	}

	void Reset() const noexcept {
		buffer.fill(0);
	}

private:
	std::array<char, SIZE> buffer;
};
}
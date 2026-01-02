#pragma once
#include <format>
#include <string>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <deque>
#include <mutex>

namespace Report {
    static inline std::atomic<unsigned long long> report_uid = 0;
    enum class Level {
        Temp,    // 临时等级(无指定颜色), 不会被输出到文件中
        Info,    // 普通信息(蓝色)
        Warning, // 警告信息(黄色)
        Error,   // 错误信息(橙色)
        Fatal,   // 致命错误信息(红色), 会直接终止程序
    };
    static std::string LevelToString(const Level level) {
        std::string result;

        switch (level) {
            case Level::Temp: result = "[Temp]"; break;
            case Level::Info: result = "[Info]"; break;
            case Level::Warning: result = "[Warning]"; break;
            case Level::Error: result = "[Error]"; break;
            case Level::Fatal: result = "[Fatal]"; break;
            default: result = "[Temp]"; break;
        }
        return result;
    }


    struct Message {
    public:
        Message() = default;
        Message(const Message& message) : m_msg(message.GetRawMessage()), m_level(message.GetLevel()) {}
        Message(Message&& message) noexcept : m_msg(std::move(message.GetRawMessage())), m_level(message.GetLevel()) {}
        explicit Message(const std::string& message, const Level level = Level::Info) : m_msg(message), m_level(level) {}
        explicit Message(std::string&& message, const Level level = Level::Info) : m_msg(std::move(message)), m_level(level) {}

        Message& operator=(const Message& message) {
            m_msg = message.GetRawMessage();
            m_level = message.GetLevel();
            return *this;
        }
        Message& operator=(Message&& message) noexcept {
            m_msg = std::move(message.GetRawMessage());
            m_level = message.GetLevel();
            return *this;
        }
        Message& operator=(const std::string& message) {
            m_msg = message;
            m_level = Level::Info;
            return *this;
        }
    public:
        enum class OutType {
            Default = 0,
            Console = 1,
        };
    public:
        [[nodiscard]] std::string GetRawMessage() const { return m_msg; }
        [[nodiscard]] std::string GetMessage(OutType type = OutType::Default) const {
            std::string prefix = LevelToString(m_level);
            // 这里的颜色是真彩色, 并不是所有终端都支持(老一点终端可能不支持)
            // todo: 之后可以专门封装一个 ColorWrapper(std::string text, int r, int g, int b) 来支持不同终端中的颜色显示
            if (type == OutType::Console) {
                switch (m_level) {
                    case Level::Temp: /* 没有颜色, 不做任何设置*/                                break;
                    case Level::Info: prefix = "\033[38;2;0;122;255m " + prefix + "\033[0m:   "; break;   // Dodger Blue
                    case Level::Warning: prefix = "\033[38;2;255;204;0m " + prefix + "\033[0m:"; break;   // Gold
                    case Level::Error: prefix = "\033[38;2;255;165;0m " + prefix + "\033[0m:  "; break;   // Orange Red
                    case Level::Fatal: prefix = "\033[38;2;220;20;60m " + prefix + "\033[0m:  "; break;   // Crimson
                }
            }
            std::string formatted_msg = prefix + m_msg + "\n";
            return formatted_msg;
        }
        [[nodiscard]] Level GetLevel() const { return m_level; }
        void SetRawMessage(const std::string &message) { m_msg = message; }
        void SetCatalogue(const size_t uid) { m_catalogue_uid = uid; }
        void SetUID(const size_t uid) { m_uid = uid; }
        [[nodiscard]] size_t GetCatalogue() const { return m_uid; }
        [[nodiscard]] size_t GetUID() const { return m_catalogue_uid; }
        [[nodiscard]] bool IsNeedWriteToFile() const { return m_level != Level::Temp; }
    private:
        std::string m_msg;
        Level m_level { Level::Info };
        size_t m_catalogue_uid { 0 };
        size_t m_uid { 0 };
    };

    inline Message MakeMessage(const std::string& message, Level level = Level::Info) {
        return Message(message, level);
    }

    template <typename... Args>
    Message MakeMessage(const Level level, std::format_string<Args...> fmt, Args&&... args) {;
        return MakeMessage(std::format(fmt, std::forward<Args>(args)...), level);
    }

    struct Buffer {
    public:
        static constexpr std::format_string<size_t> DefaultLogFileNameFormat = "Log_Buffer_{}.log";
    public:
        Buffer() : m_uid(report_uid++) {
            m_file_path = std::filesystem::current_path() / std::format(DefaultLogFileNameFormat, static_cast<size_t>(m_uid));
            m_file_stream.open(m_file_path, std::ios::out | std::ios::app);
            if (!m_file_stream.is_open()) {
                Message err_msg = MakeMessage(Level::Error, "Buffer_{} failed to open log file: {}", m_uid, m_file_path.string());
                m_messages.push_back(std::move(err_msg));
            }
        };

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&& buffer) noexcept
            : m_uid(buffer.m_uid),
              m_file_path(std::move(buffer.m_file_path)),
              m_file_stream(std::move(buffer.m_file_stream)),
              m_messages(std::move(buffer.m_messages)) {}

        Buffer& operator=(Buffer&& other) noexcept {
            m_uid = other.m_uid;
            m_file_path = std::move(other.m_file_path);
            m_file_stream = std::move(other.m_file_stream);
            m_messages = std::move(other.m_messages);
            return *this;
        }

        ~Buffer() {
            if (m_file_stream.is_open()) {
                m_file_stream.close();
            }
        }

    public:
        void Push(const Message& msg) {
            std::unique_lock ulock(m_mutex);
            m_messages.push_back(msg);
            if (m_file_stream.is_open()) {
                m_file_stream << msg.GetMessage();
            }
        }

        void Push(const std::string &msg, const Level level = Level::Info) {
            Push(Message(msg, level));
        }

        void Push(const char* msg, const Level level = Level::Info) {
            Push(Message(std::string(msg), level));
        }

        [[nodiscard]] const Message& Pop() const {
            return m_messages.front();
        }

        [[nodiscard]] Message Top() {
            return m_messages.front();
        }

        Message& operator[](const size_t index) noexcept {
            std::unique_lock ulock (m_mutex);
            return m_messages[index];
        }
        const Message& operator[](const size_t index) const noexcept { return m_messages[index]; }

        [[nodiscard]] Message& At(const size_t index) {
            std::unique_lock ulock (m_mutex);
            if (index >= m_messages.size()) throw std::out_of_range("index out of range");
            return m_messages[index];
        }
        [[nodiscard]] const Message& At(const size_t index) const {
            if (index >= m_messages.size()) throw std::out_of_range("index out of range");
            return m_messages[index];
        }
    private:
        size_t m_uid { 0 };
        std::filesystem::path m_file_path {};
        std::fstream m_file_stream {};
        std::deque<Message> m_messages {};
        std::mutex m_mutex;
    };
    static void Write(const Level level, Buffer& buffer, const std::string& message) {

    }

    template <typename... Args>
    static void Write(const Level level, Buffer& buffer, const std::format_string<Args...> format, Args&&... args) {;
        Write(level, buffer, std::vformat(format, std::make_format_args(std::forward<Args>(args)...)));
    }
}

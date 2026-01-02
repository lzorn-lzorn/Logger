#pragma once
#include <format>
#include <string>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <deque>
#include <mutex>
#include <string_view>
#include <print>
#include <chrono>
#include <optional>


#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)
#define DO_PRAGMA(x) _Pragma(#x)
#define WARN_MSG(msg) DO_PRAGMA(message (__FILE__ ":" STRINGIZE(__LINE__) " WARNING: " msg))


namespace Report {
    using namespace std::chrono;
    static inline std::atomic<unsigned long long> report_uid = 1;
    static inline std::atomic<unsigned long long> massage_uid = 1;

    enum class Level {
        None,    // 无效日志, 表示内部错误
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
            default: result = ">> [[InnerError]]"; break;
        }
        return result;
    }
    static Level StringToLevel(const std::string& level) {
        const std::string_view sv(level);
        size_t left = 0;
        const size_t len = sv.size();
        // 去掉左右可能存在的空格
        while (left < len && std::isspace(sv[left])) ++left;
        size_t right = len;
        while (right > left && std::isspace(sv[right - 1])) --right;

        if (right - left < 2 || sv[right-1] != ']' || sv[left-1] != '[') {
            return Level::None;
        }
        // 去掉两端 []
        const std::string_view inner = sv.substr(left + 1, right - left - 2);
        if (inner.empty()) return Level::None;

        if (inner == "Temp") return Level::Temp;
        else if (inner == "Info") return Level::Info;
        else if (inner == "Warning") return Level::Warning;
        else if (inner == "Error") return Level::Error;
        else if (inner == "Fatal") return Level::Fatal;
        else return Level::None;
    }


    struct Message {

    public:
        Message() = default;
        Message(const Message& message) : m_msg(message.GetRawMessage()), m_level(message.GetLevel()) {}
        Message(Message&& message) noexcept : m_msg(std::move(message.GetRawMessage())), m_level(message.GetLevel()) {}
        explicit Message(const std::string& message, const Level level = Level::Info) : m_msg(message), m_level(level) {}
        explicit Message(std::string&& message, const Level level = Level::Info) : m_msg(std::move(message)), m_level(level) {}
        explicit Message(const char* message, const Level level = Level::Info) : m_msg(message), m_level(level) {}

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
        [[nodiscard]] std::string GetMessage(const OutType type = OutType::Default) const {
            std::string prefix = LevelToString(m_level);
            // 这里的颜色是真彩色, 并不是所有终端都支持(老一点终端可能不支持)
            // todo: 之后可以专门封装一个 ColorWrapper(std::string text, int r, int g, int b) 来支持不同终端中的颜色显示
            if (type == OutType::Console) {
                switch (m_level) {
                    case Level::Temp: /* 没有颜色, 不做任何设置*/                              ; break;
                    case Level::None: /* 没有颜色, 不做任何设置*/                              ; break;
                    case Level::Info: prefix = "\033[38;2;0;122;255m " + prefix + "\033[0m:   "; break;   // Dodger Blue
                    case Level::Warning: prefix = "\033[38;2;255;204;0m " + prefix + "\033[0m:"; break;   // Gold
                    case Level::Error: prefix = "\033[38;2;255;165;0m " + prefix + "\033[0m:  "; break;   // Orange Red
                    case Level::Fatal: prefix = "\033[38;2;220;20;60m " + prefix + "\033[0m:  "; break;   // Crimson
                }

            }else {
                switch (m_level) {
                    case Level::Temp: /* 没有颜色 */          ; break;
                    case Level::Info: prefix = prefix + ":   "; break;   // Dodger Blue
                    case Level::Warning: prefix = prefix + ":"; break;   // Gold
                    case Level::Error: prefix = prefix + ":  "; break;   // Orange Red
                    case Level::Fatal: prefix = prefix + ":  "; break;   // Crimson
                    case Level::None: prefix = prefix + ":"   ; break;   // Crimson
                }
            }
            std::string formatted_msg = prefix + m_msg + "\n";
            return formatted_msg;
        }
        [[nodiscard]] Level GetLevel() const { return m_level; }
        void SetMessage(const std::string &message, const Level level = Level::Info) {
            m_msg = message;
            m_level = level;
            m_uid = massage_uid++;
        }
        // m_catalogue_uid 是用于外部扩展的, 如果外部还需要打一个新的 Tag, 可以使用这个字段
        void SetCatalogue(const size_t uid) { m_catalogue_uid = uid; }
        // 获取该信息的秒级时间戳
        [[nodiscard]] long long GetSecondTimeStamp(const size_t uid) const {
            return duration_cast<seconds>(m_time.time_since_epoch()).count();;
        }
        // 获取该信息的毫秒级时间戳
        [[nodiscard]] long long GetMilliSecondTimeStamp(const size_t uid) const {
            return duration_cast<milliseconds>(m_time.time_since_epoch()).count();;
        }
        [[nodiscard]] size_t GetCatalogue() const { return m_uid; }
        [[nodiscard]] unsigned long long GetUID() const { return m_catalogue_uid; }
        [[nodiscard]] bool IsNeedWriteToFile() const { return m_level != Level::Temp; }
        [[nodiscard]] bool IsValid() const { return m_level == Level::None; }
    private:
        std::string m_msg;
        Level m_level { Level::None };
        size_t m_catalogue_uid { 0 };
        unsigned long long m_uid { 0 };
        system_clock::time_point m_time { system_clock::now() };
    };

    inline Message MakeMessage(const std::string& message, Level level = Level::Info) {
        return Message(message, level);
    }

    template <typename... Args>
    Message MakeMessage(const Level level, std::format_string<Args...> fmt, Args&&... args) {;
        return MakeMessage(std::format(fmt, std::forward<Args>(args)...), level);
    }

    inline void PrintMessageToConsole(const Message& msg) {
        std::print("{}", msg.GetMessage(Report::Message::OutType::Console));
    }

    inline void PrintMessageToFile(const Message& msg, std::fstream& file_stream) {
        if (file_stream.is_open()) {
            file_stream << msg.GetMessage();
        }
    }
    /*
     * @brief：从文件中读取并删除最后一行（保留 Buffer 中的 file_stream 打开状态）
     * @note：函数会先 flush 传入的 file_stream，然后用独立的 ifstream 读取并截断文件。
     *       返回的 Message 的 level 会基于文件行前缀自动推断（例如 "[Info]"、"[Warning]" 等），
     *       并把行前缀（例如 "[Info]:   "）从返回内容中剥离，仅返回纯消息文本。
     */
    inline Message GetMessageFromFile(std::fstream& file_stream, const std::filesystem::path& file_path) {
        if (file_stream) {
            file_stream.flush();
        }
        std::error_code error_code;

        if (!std::filesystem::exists(file_path, error_code) || error_code) {
            return {};
        }

        auto file_size = std::filesystem::file_size(file_path, error_code);
        if (error_code || file_size == 0) {
            return {};
        }

        // 从末尾跳过换行符
        std::uintmax_t file_end = file_size;
        while (file_end > 0) {
            file_stream.seekg(static_cast<std::streamoff>(file_end-1), std::ios::beg);
            char ch = 0;
            file_stream.get(ch);
            if (!file_stream) return {};

            if (ch == '\n' || ch == '\r') {
                --file_end;
            } else {
                break;
            }
        }
        if (file_end == 0) {
            std::filesystem::resize_file(file_path, 0, error_code);
            (void)error_code;
            return {};
        }

        // 前向搜索最后一个 '\n' 的位置
        constexpr std::size_t buf_size = 2048;
        std::uintmax_t search_pos = file_size;
        std::optional<std::uintmax_t> found_pos = std::nullopt;
        std::vector<char> buf(buf_size);

        while (search_pos > 0) {
            std::size_t to_read = std::min<std::uintmax_t>(search_pos, buf_size);
            std::uintmax_t start = search_pos - to_read;
            file_stream.seekg(static_cast<std::streamoff>(start));
            buf.resize(to_read);
            file_stream.read(buf.data(), static_cast<std::streamsize>(to_read));
            std::streamsize actually = file_stream.gcount();
            if (actually <= 0) return {};

            for (std::streamoff i = actually-1; i >= 0; i--) {
                if (buf[static_cast<size_t>(i)] == '\n') {
                    found_pos = start + static_cast<std::uintmax_t>(i);
                    break;
                }
                if (i == 0) break;
            }
            if (found_pos.has_value()) break;
            if (start == 0) break;
            search_pos = start;
        }

        std::uintmax_t last_line_start = found_pos.has_value() ? (*found_pos + 1) : 0;
        std::uintmax_t last_line_length = file_end - last_line_start;

        // 读取最后一行内容
        std::string last_line;
        last_line.resize(last_line_length);
        file_stream.seekg(static_cast<std::streamoff>(last_line_start));
        file_stream.read(&last_line[0], static_cast<std::streamsize>(last_line_length));
        if (!file_stream || !file_stream.eof()) return {};

        // 去掉可能的末尾 CR (处理 CRLF)
        if (!last_line.empty() && last_line.back() == '\r') last_line.pop_back();

        // 截断文件到 new_size (保留上一个换行符，如果存在)
        std::uintmax_t new_size = found_pos.has_value() ? (*found_pos + 1) : 0;
        std::filesystem::resize_file(file_path, new_size, error_code);
        (void)error_code;

        // 从行头推断 Level 并剥离前缀 "[Info]" / "[Warning]" / "[Error]" / "[Fatal]" 以及 ':' 和空白
        Level detected = Level::None;
        Message result {};
        size_t content_pos = 0;

        if (!last_line.empty() && last_line.front() == '[') {
            // 找到 ']'
            auto rb = last_line.find(']');
            if (rb != std::string::npos) {
                std::string tag = last_line.substr(0, rb+1); // 包括 ']'
                if (const auto level = StringToLevel(tag); level == Level::None) {
                    detected = level;
                    Message real_result(last_line.substr(rb + 1), detected);
                    result = std::move(real_result);
                }
                content_pos = rb + 1;
                while (content_pos < last_line.size() && (last_line[content_pos] == ':' || std::isspace(last_line[content_pos]))) {
                    ++content_pos;
                }
            }
        }

        // 找到内容则返回Message对象, 否则返回表示错误的Message对象
        if (result.IsValid()) {
            return result;
        }

        std::string content;
        if (content_pos < last_line.size()) {
            content = last_line.substr(content_pos);
        }
        Message real_result(content, detected);
        result = std::move(real_result);
        return result;
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
            m_messages.push_front(msg);
            PrintMessageToFile(msg, m_file_stream);
        }

        void Push(const std::string &msg, const Level level = Level::Info) {
            Push(Message(msg, level));
        }

        void Push(const char* msg, const Level level = Level::Info) {
            Push(Message(std::string(msg), level));
        }

        Message Pop() {
            if (m_messages.empty()) throw std::out_of_range("Buffer is empty");
            Message msg = m_messages.back();
            {
                std::unique_lock ulock(m_mutex);
                // todo: 是否需要从文件中 pop?
                // GetMessageFromFile(m_file_stream, m_file_path);
                m_messages.pop_back();
            }
            return msg;
        }

        [[nodiscard]] Message Top() {
            if (m_messages.empty()) throw std::out_of_range("Buffer is empty");
            return m_messages.back();
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

    static Message Read(Buffer& buffer) {
        Message msg = buffer.Pop();
        return msg;
    }
}

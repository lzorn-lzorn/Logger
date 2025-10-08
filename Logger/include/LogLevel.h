#pragma once

namespace Log {
enum class Level {
    None = 0,
	Debug = 1,
	Info = 2,
	Warning = 3,
	Error = 4,
	Fatal = 5
};
inline constexpr const char* LevelToString(Level level) noexcept {
    switch (level) {
        case Level::None:    return "";
        case Level::Debug:   return "[Debug]";
        case Level::Info:    return "[Info]";
        case Level::Warning: return "[Warning]";
        case Level::Error:   return "[Error]";
        case Level::Fatal:   return "[Fatal]";
        default:             return "[Unknown]";
    }
}

}
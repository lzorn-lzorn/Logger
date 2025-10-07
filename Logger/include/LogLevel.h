#pragma once

namespace Log {
enum class Level {
	Debug = 0,
	Info = 1,
	Warning = 2,
	Error = 3,
	Fatal = 4
};
inline constexpr const char* LevelToString(Level level) noexcept {
    switch (level) {
        case Level::Debug:   return "[Debug]";
        case Level::Info:    return "[Info]";
        case Level::Warning: return "[Warning]";
        case Level::Error:   return "[Error]";
        case Level::Fatal:   return "[Fatal]";
        default:             return "[Unknown]";
    }
}

}
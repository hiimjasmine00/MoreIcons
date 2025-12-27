#include <fmt/format.h>

namespace Notify {
    void error(const std::string& message);
    template <typename... Args>
    void error(fmt::format_string<Args...> message, Args&&... args) {
        error(fmt::format(message, std::forward<Args>(args)...));
    }

    void info(const std::string& message);
    template <typename... Args>
    void info(fmt::format_string<Args...> message, Args&&... args) {
        info(fmt::format(message, std::forward<Args>(args)...));
    }

    void success(const std::string& message);
    template <typename... Args>
    void success(fmt::format_string<Args...> message, Args&&... args) {
        success(fmt::format(message, std::forward<Args>(args)...));
    }
};

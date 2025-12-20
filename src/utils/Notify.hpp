#include <fmt/format.h>

class Notify {
public:
    template <typename... Args>
    static void error(fmt::format_string<Args...> message, Args&&... args) {
        error(fmt::format(message, std::forward<Args>(args)...));
    }
    static void error(const std::string& message);

    template <typename... Args>
    static void info(fmt::format_string<Args...> message, Args&&... args) {
        info(fmt::format(message, std::forward<Args>(args)...));
    }
    static void info(const std::string& message);

    template <typename... Args>
    static void success(fmt::format_string<Args...> message, Args&&... args) {
        success(fmt::format(message, std::forward<Args>(args)...));
    }
    static void success(const std::string& message);
};

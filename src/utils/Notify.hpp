#include <Geode/utils/ZStringView.hpp>

namespace Notify {
    void error(geode::ZStringView message);
    template <typename... Args>
    void error(fmt::format_string<Args...> message, Args&&... args) {
        error(fmt::format(message, std::forward<Args>(args)...));
    }

    void info(geode::ZStringView message);
    template <typename... Args>
    void info(fmt::format_string<Args...> message, Args&&... args) {
        info(fmt::format(message, std::forward<Args>(args)...));
    }

    void success(geode::ZStringView message);
    template <typename... Args>
    void success(fmt::format_string<Args...> message, Args&&... args) {
        success(fmt::format(message, std::forward<Args>(args)...));
    }
}

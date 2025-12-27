#include <Geode/Enums.hpp>
#include <Geode/loader/Types.hpp>

struct LogData {
    std::string name;
    std::string message;
    geode::Severity severity;
};

namespace Log {
    extern std::map<IconType, std::vector<LogData>> logs;
    extern IconType currentType;

    void error(std::string&& name, std::string&& message);
    void warn(std::string&& name, std::string&& message);
};

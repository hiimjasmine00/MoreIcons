#include <Geode/Enums.hpp>
#include <Geode/loader/Types.hpp>

struct LogData {
    std::string name;
    std::string message;
    geode::Severity severity;
};

class Log {
public:
    static std::map<IconType, std::vector<LogData>> logs;
    static IconType currentType;

    static void error(std::string&& name, std::string&& message);
    static void warn(std::string&& name, std::string&& message);
};

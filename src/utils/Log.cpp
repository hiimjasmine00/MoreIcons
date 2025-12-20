#include "Log.hpp"
#include <algorithm>
#include <Geode/loader/Log.hpp>

using namespace geode::prelude;

std::map<IconType, std::vector<LogData>> Log::logs;
IconType Log::currentType = IconType::Cube;

void Log::error(std::string&& name, std::string&& message) {
    log::error("{}: {}", name, message);
    auto& currentLogs = logs[currentType];
    currentLogs.emplace(std::ranges::find_if(currentLogs, [&name](const LogData& log) {
        return log.severity == Severity::Error ? log.name > name : log.severity.m_value < Severity::Error;
    }), std::move(name), std::move(message), Severity::Error);
}

void Log::warn(std::string&& name, std::string&& message) {
    log::warn("{}: {}", name, message);
    auto& currentLogs = logs[currentType];
    currentLogs.emplace(std::ranges::find_if(currentLogs, [&name](const LogData& log) {
        return log.severity == Severity::Warning ? log.name > name : log.severity.m_value < Severity::Warning;
    }), std::move(name), std::move(message), Severity::Warning);
}

#include "Notify.hpp"
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

void Notify::error(const std::string& message) {
    Notification::create(message, NotificationIcon::Error)->show();
}

void Notify::info(const std::string& message) {
    Notification::create(message, NotificationIcon::Info)->show();
}

void Notify::success(const std::string& message) {
    Notification::create(message, NotificationIcon::Success)->show();
}

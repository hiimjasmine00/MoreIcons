#include "Notify.hpp"
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

void Notify::error(ZStringView message) {
    Notification::create(message.c_str(), NotificationIcon::Error)->show();
}

void Notify::info(ZStringView message) {
    Notification::create(message.c_str(), NotificationIcon::Info)->show();
}

void Notify::success(ZStringView message) {
    Notification::create(message.c_str(), NotificationIcon::Success)->show();
}

#include "Notify.hpp"
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

void Notify::error(ZStringView message) {
    Notification::create(message, NotificationIcon::Error)->show();
}

void Notify::info(ZStringView message) {
    Notification::create(message, NotificationIcon::Info)->show();
}

void Notify::success(ZStringView message) {
    Notification::create(message, NotificationIcon::Success)->show();
}

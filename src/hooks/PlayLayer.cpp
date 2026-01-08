#include "../utils/Icons.hpp"
#include <Geode/modify/PlayLayer.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

class $modify(MIPlayLayer, PlayLayer) {
    struct Fields {
        ~Fields() {
            if (auto info = more_icons::activeIcon(IconType::DeathEffect, false)) {
                Icons::uncacheIcon(info);
            }
            if (auto info = more_icons::activeIcon(IconType::DeathEffect, true)) {
                Icons::uncacheIcon(info);
            }
        }
    };

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();

        m_fields.self();

        if (auto info = more_icons::activeIcon(IconType::DeathEffect, false)) {
            Icons::createAndAddFrames(info);
        }

        if (auto info = more_icons::activeIcon(IconType::DeathEffect, true)) {
            Icons::createAndAddFrames(info);
        }
    }
};

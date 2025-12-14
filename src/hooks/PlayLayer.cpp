#include "../MoreIcons.hpp"
#include "../utils/Get.hpp"
#include <Geode/modify/PlayLayer.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

class $modify(MIPlayLayer, PlayLayer) {
    struct Fields {
        ~Fields() {
            auto textureCache = Get::TextureCache();
            auto spriteFrameCache = Get::SpriteFrameCache();
            if (auto info = more_icons::getIcon(IconType::DeathEffect, false)) {
                for (auto& frameName : info->getFrameNames()) {
                    spriteFrameCache->removeSpriteFrameByName(frameName.c_str());
                }
                textureCache->removeTextureForKey(info->getTextureString().c_str());
            }
            if (auto info = more_icons::getIcon(IconType::DeathEffect, true)) {
                for (auto& frameName : info->getFrameNames()) {
                    spriteFrameCache->removeSpriteFrameByName(frameName.c_str());
                }
                textureCache->removeTextureForKey(info->getTextureString().c_str());
            }
        }
    };

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();

        if (auto info = more_icons::getIcon(IconType::DeathEffect, false)) {
            m_fields.self();
            MoreIcons::createAndAddFrames(info);
        }

        if (auto info = more_icons::getIcon(IconType::DeathEffect, true)) {
            m_fields.self();
            MoreIcons::createAndAddFrames(info);
        }
    }
};

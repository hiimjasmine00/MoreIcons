#include "../MoreIcons.hpp"
#include "../utils/Get.hpp"
#include <Geode/binding/GameManager.hpp>
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

        if (MoreIcons::traditionalPacks) {
            auto gameManager = Get::GameManager();
            auto effectID = gameManager->m_loadedDeathEffect;
            if (effectID > 1) {
                auto texturePath = fmt::format("PlayerExplosion_{:02}.png", effectID - 1);
                auto fullPath = Get::FileUtils()->fullPathForFilename(texturePath.c_str(), false);
                auto vanillaPath = MoreIcons::vanillaTexturePath(texturePath, false);
                if (MoreIcons::strPath(fullPath) != MoreIcons::strPath(vanillaPath)) {
                    auto textureCache = Get::TextureCache();
                    textureCache->removeTextureForKey(fullPath.c_str());
                    auto texture = textureCache->addImage(vanillaPath.c_str(), false);
                    vanillaPath.replace(vanillaPath.size() - 4, 4, ".plist");
                    Get::SpriteFrameCache()->addSpriteFramesWithFile(vanillaPath.c_str(), texture);
                }
            }
        }

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

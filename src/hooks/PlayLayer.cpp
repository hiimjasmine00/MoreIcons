#include "../MoreIcons.hpp"
#include "../utils/Filesystem.hpp"
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
            auto effectID = gameManager->m_loadedDeathEffect - 1;
            if (effectID > 0) {
                auto factor = Get::Director()->getContentScaleFactor();
                auto texturePath = fmt::format("PlayerExplosion_{:02}{}.png", effectID, factor >= 4.0f ? "-uhd" : factor >= 2.0f ? "-hd" : "");
                std::string fullPath = Get::FileUtils()->fullPathForFilename(texturePath.c_str(), false);
                auto vanillaPath = MoreIcons::vanillaTexturePath(texturePath, false);
                if (Filesystem::strPath(fullPath) != Filesystem::strPath(vanillaPath)) {
                    auto textureCache = Get::TextureCache();
                    auto spriteFrameCache = Get::SpriteFrameCache();
                    textureCache->removeTextureForKey(fullPath.c_str());
                    fullPath.replace(fullPath.size() - 4, 4, ".plist");
                    spriteFrameCache->removeSpriteFramesFromFile(fullPath.c_str());
                    auto texture = textureCache->addImage(vanillaPath.c_str(), false);
                    vanillaPath.replace(vanillaPath.size() - 4, 4, ".plist");
                    spriteFrameCache->addSpriteFramesWithFile(vanillaPath.c_str(), texture);
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

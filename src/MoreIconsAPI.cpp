#include "MoreIcons.hpp"
#include "classes/popup/info/MoreInfoPopup.hpp"
#include "utils/Get.hpp"
#include "utils/Load.hpp"
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#define GEODE_DEFINE_EVENT_EXPORTS
#include <MoreIconsV2.hpp>
#include <ranges>
#include <texpack.hpp>

using namespace geode::prelude;

$execute {
    new EventListener(+[](SimplePlayer* player, const std::string& icon, IconType type) {
        more_icons::updateSimplePlayer(player, icon, type);
        return ListenerResult::Propagate;
    }, DispatchFilter<SimplePlayer*, std::string, IconType>("simple-player"_spr));

    new EventListener(+[](GJRobotSprite* sprite, const std::string& icon, IconType type) {
        more_icons::updateRobotSprite(sprite, icon, type);
        return ListenerResult::Propagate;
    }, DispatchFilter<GJRobotSprite*, std::string, IconType>("robot-sprite"_spr));

    new EventListener(+[](PlayerObject* object, const std::string& icon, IconType type) {
        more_icons::updatePlayerObject(object, icon, type);
        return ListenerResult::Propagate;
    }, DispatchFilter<PlayerObject*, std::string, IconType>("player-object"_spr));

    new EventListener(+[](std::vector<IconInfo*>* vec) {
        vec->clear();
        for (auto& icons : std::views::values(MoreIcons::icons)) {
            auto size = icons.size();
            vec->reserve(size);
            for (int i = 0; i < size; i++) vec->push_back(icons.data() + i);
        }
        return ListenerResult::Propagate;
    }, DispatchFilter<std::vector<IconInfo*>*>("all-icons"_spr));

    new EventListener(+[](std::vector<IconInfo*>* vec, IconType type) {
        vec->clear();
        auto icons = more_icons::getIcons(type);
        if (!icons) return ListenerResult::Propagate;
        auto size = icons->size();
        vec->reserve(size);
        for (int i = 0; i < size; i++) vec->push_back(icons->data() + i);
        return ListenerResult::Propagate;
    }, DispatchFilter<std::vector<IconInfo*>*, IconType>("get-icons"_spr));

    new EventListener(+[](IconInfo** info, const std::string& name, IconType type) {
        *info = more_icons::getIcon(name, type);
        return ListenerResult::Propagate;
    }, DispatchFilter<IconInfo**, std::string, IconType>("get-icon"_spr));

    new EventListener(+[](const std::string& icon, IconType type, int requestID) {
        more_icons::loadIcon(icon, type, requestID);
        return ListenerResult::Propagate;
    }, DispatchFilter<std::string, IconType, int>("load-icon"_spr));

    new EventListener(+[](const std::string& icon, IconType type, int requestID) {
        more_icons::unloadIcon(icon, type, requestID);
        return ListenerResult::Propagate;
    }, DispatchFilter<std::string, IconType, int>("unload-icon"_spr));

    new EventListener(+[](int requestID) {
        more_icons::unloadIcons(requestID);
        return ListenerResult::Propagate;
    }, DispatchFilter<int>("unload-icons"_spr));

    new EventListener(+[](FLAlertLayer** layer, const std::string& name, IconType type) {
        *layer = more_icons::createInfoPopup(name, type);
        return ListenerResult::Propagate;
    }, DispatchFilter<FLAlertLayer**, std::string, IconType>("info-popup"_spr));
}

FLAlertLayer* more_icons::createInfoPopup(const std::string& name, IconType type) {
    if (auto info = more_icons::getIcon(name, type)) return MoreInfoPopup::create(info);
    else return nullptr;
}

std::map<IconType, std::vector<IconInfo>>* more_icons::getIcons() {
    return &MoreIcons::icons;
}

std::vector<IconInfo>* more_icons::getIcons(IconType type) {
    auto it = MoreIcons::icons.find(type);
    return it != MoreIcons::icons.end() ? &it->second : nullptr;
}

IconInfo* more_icons::getIcon(const std::string& name, IconType type) {
    if (name.empty()) return nullptr;
    auto icons = getIcons(type);
    if (!icons) return nullptr;
    auto it = std::ranges::find(*icons, name, &IconInfo::name);
    return it < icons->end() ? std::to_address(it) : nullptr;
}

CCTexture2D* more_icons::loadIcon(const std::string& name, IconType type, int requestID) {
    auto info = getIcon(name, type);
    if (!info) return nullptr;

    auto& png = info->textures[0];
    auto texture = Get::TextureCache()->textureForKey(png.c_str());
    if (MoreIcons::preloadIcons) return texture;

    auto& loadedIcon = MoreIcons::loadedIcons[{ name, type }];

    if (loadedIcon < 1) {
        if (auto res = Load::createFrames(MoreIcons::strPath(info->textures[0]), MoreIcons::strPath(info->sheetName), info->name, info->type)) {
            texture = Load::addFrames(res.unwrap(), info->frameNames);
        }
        else {
            log::error("{}: {}", info->name, res.unwrapErr());
        }
    }

    auto& requestedIcon = MoreIcons::requestedIcons[requestID][type];
    if (requestedIcon != name) {
        loadedIcon++;
        if (!requestedIcon.empty()) unloadIcon(requestedIcon, type, requestID);
        MoreIcons::requestedIcons[requestID][type] = name;
    }

    return texture;
}

void more_icons::unloadIcon(const std::string& name, IconType type, int requestID) {
    if (MoreIcons::preloadIcons || name.empty()) return;

    auto info = getIcon(name, type);
    if (!info) return;

    auto& loadedIcon = MoreIcons::loadedIcons[{ name, type }];

    loadedIcon--;
    if (loadedIcon < 1) {
        auto spriteFrameCache = Get::SpriteFrameCache();
        if (spriteFrameCache->m_pLoadedFileNames) {
            for (auto& frame : info->frameNames) {
                spriteFrameCache->removeSpriteFrameByName(frame.c_str());
            }
        }
        info->frameNames.clear();

        Get::TextureCache()->removeTextureForKey(info->textures[0].c_str());
    }

    MoreIcons::requestedIcons[requestID].erase(type);
    if (MoreIcons::requestedIcons[requestID].empty()) MoreIcons::requestedIcons.erase(requestID);
}

void more_icons::unloadIcons(int requestID) {
    if (MoreIcons::preloadIcons) return;

    auto foundRequests = MoreIcons::requestedIcons.find(requestID);
    if (foundRequests == MoreIcons::requestedIcons.end()) return;

    auto& iconRequests = foundRequests->second;
    for (int i = 0; i < 9; i++) {
        auto type = (IconType)i;
        if (auto found = iconRequests.find(type); found != iconRequests.end()) {
            auto& icon = found->second;
            if (!icon.empty()) {
                auto lastIcon = iconRequests.size() == 1;
                unloadIcon(icon, type, requestID);
                if (lastIcon) return;
            }
        }
    }

    MoreIcons::requestedIcons.erase(requestID);
}

IconInfo* more_icons::addIcon(
    const std::string& name, const std::string& shortName, IconType type, const std::string& png, const std::string& plist,
    const std::string& packID, const std::string& packName, int trailID, const TrailInfo& trailInfo, bool vanilla, bool zipped
) {
    auto icons = getIcons(type);
    if (!icons) return nullptr;
    auto it = std::ranges::find_if(*icons, [&packID, &shortName, type](const IconInfo& icon) {
        return icon.compare(packID, shortName, type) >= 0;
    });
    if (it != icons->end() && it->type == type && it->name == name) icons->erase(it);
    return std::to_address(icons->emplace(it, name, png, plist, packName, packID, type, trailID, trailInfo, shortName, vanilla, zipped));
}

void more_icons::moveIcon(IconInfo* info, const std::filesystem::path& path) {
    auto oldPng = info->textures[0];
    info->textures[0] = string::pathToString(path / MoreIcons::strPath(oldPng).filename());
    info->sheetName = string::pathToString(path / MoreIcons::strPath(info->sheetName).filename());
    info->vanilla = false;

    auto textureCache = Get::TextureCache();
    if (Ref texture = textureCache->textureForKey(oldPng.c_str())) {
        textureCache->removeTextureForKey(oldPng.c_str());
        textureCache->m_pTextures->setObject(texture, info->textures[0]);
    }
}

void more_icons::removeIcon(IconInfo* info) {
    auto& name = info->name;
    auto type = info->type;
    MoreIcons::loadedIcons[{ name, type }] = 0;

    auto spriteFrameCache = Get::SpriteFrameCache();
    for (auto& frame : info->frameNames) {
        spriteFrameCache->removeSpriteFrameByName(frame.c_str());
    }
    Get::TextureCache()->removeTextureForKey(info->textures[0].c_str());

    if (!MoreIcons::preloadIcons) {
        for (auto it = MoreIcons::requestedIcons.begin(); it != MoreIcons::requestedIcons.end();) {
            auto& iconRequests = it->second;
            auto iconRequest = iconRequests.find(type);
            if (iconRequest != iconRequests.end() && iconRequest->second == name) {
                iconRequests.erase(iconRequest);
                if (iconRequests.empty()) it = MoreIcons::requestedIcons.erase(it);
                else ++it;
            }
            else ++it;
        }
    }

    auto icons = getIcons(type);
    if (icons) icons->erase(icons->begin() + (info - icons->data()));
}

void more_icons::renameIcon(IconInfo* info, const std::string& name) {
    auto oldName = info->name;
    auto newName = info->packID.empty() ? name : fmt::format("{}:{}", info->packID, name);
    info->name = newName;
    info->shortName = name;

    auto quality = MI_PATH("");
    auto oldPng = info->textures[0];
    auto type = info->type;
    if (type <= IconType::Jetpack) {
        if (oldPng.ends_with("-uhd.png")) quality = MI_PATH("-uhd");
        else if (oldPng.ends_with("-hd.png")) quality = MI_PATH("-hd");
    }

    #ifdef GEODE_IS_WINDOWS
    auto wName = string::utf8ToWide(name);
    #else
    auto& wName = name;
    #endif
    info->textures[0] = string::pathToString(MoreIcons::strPath(oldPng).replace_filename(wName + quality + MI_PATH(".png")));
    info->sheetName = string::pathToString(MoreIcons::strPath(info->sheetName).replace_filename(wName + quality + MI_PATH(".plist")));

    auto textureCache = Get::TextureCache();
    if (Ref texture = textureCache->textureForKey(oldPng.c_str())) {
        textureCache->removeTextureForKey(oldPng.c_str());
        textureCache->m_pTextures->setObject(texture, info->textures[0]);

        auto spriteFrameCache = Get::SpriteFrameCache();
        for (auto& frameName : info->frameNames) {
            if (Ref spriteFrame = MoreIcons::getFrame(frameName)) {
                spriteFrameCache->removeSpriteFrameByName(frameName.c_str());
                frameName = Load::getFrameName(frameName, newName, type);
                spriteFrameCache->addSpriteFrame(spriteFrame, frameName.c_str());
            }
        }
    }

    if (auto it = MoreIcons::loadedIcons.find({ oldName, type }); it != MoreIcons::loadedIcons.end()) {
        MoreIcons::loadedIcons.emplace(std::make_pair(newName, type), it->second);
        MoreIcons::loadedIcons.erase(it);
    }

    for (auto& iconRequests : std::views::values(MoreIcons::requestedIcons)) {
        if (auto found = iconRequests.find(type); found != iconRequests.end() && found->second == oldName) {
            found->second = newName;
        }
    }

    if (activeIcon(type, false) == oldName) setIcon(newName, type, false);
    if (activeIcon(type, true) == oldName) setIcon(newName, type, true);

    auto icons = getIcons(type);
    if (!icons) return;

    auto it = std::ranges::find_if(*icons, [info](const IconInfo& icon) {
        return icon > *info;
    });
    if (std::to_address(it) == info) return;

    auto icon = std::move(*info);
    icons->erase(icons->begin() + (info - icons->data()));
    icons->insert(it, std::move(icon));
}

void more_icons::updateIcon(IconInfo* info) {
    auto texture = Get::TextureCache()->textureForKey(info->textures[0].c_str());
    if (!texture) return;

    auto imageRes = texpack::fromPNG(MoreIcons::strPath(info->textures[0]), true);
    if (!imageRes.isOk()) return;

    auto image = std::move(imageRes).unwrap();

    texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
        (float)image.width,
        (float)image.height
    });
    texture->m_bHasPremultipliedAlpha = true;

    auto framesRes = Load::createFrames(MoreIcons::strPath(info->sheetName), texture, info->name, info->type, true);
    if (!framesRes.isOk()) return;

    auto frames = std::move(framesRes).unwrap();
    if (!frames) return;

    auto spriteFrameCache = Get::SpriteFrameCache();
    for (auto& frameName : info->frameNames) {
        if (!frames->objectForKey(frameName)) spriteFrameCache->removeSpriteFrameByName(frameName.c_str());
    }

    info->frameNames.clear();
    for (auto [frameName, frame] : CCDictionaryExt<std::string, CCSpriteFrame*>(frames)) {
        if (auto spriteFrame = MoreIcons::getFrame(frameName)) {
            spriteFrame->m_obOffset = frame->m_obOffset;
            spriteFrame->m_obOriginalSize = frame->m_obOriginalSize;
            spriteFrame->m_obRectInPixels = frame->m_obRectInPixels;
            spriteFrame->m_bRotated = frame->m_bRotated;
            spriteFrame->m_obRect = frame->m_obRect;
            spriteFrame->m_obOffsetInPixels = frame->m_obOffsetInPixels;
            spriteFrame->m_obOriginalSizeInPixels = frame->m_obOriginalSizeInPixels;
        }
        else spriteFrameCache->addSpriteFrame(frame, frameName.c_str());
        info->frameNames.push_back(std::move(frameName));
    }
}

void more_icons::updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type) {
    if (!player || icon.empty() || !hasIcon(icon, type)) return;

    player->setUserObject("name"_spr, CCString::create(icon));

    player->m_firstLayer->setVisible(type != IconType::Robot && type != IconType::Spider);
    player->m_secondLayer->setVisible(type != IconType::Robot && type != IconType::Spider);
    player->m_birdDome->setVisible(type == IconType::Ufo);

    if (type == IconType::Robot) {
        if (!player->m_robotSprite) player->createRobotSprite(1);
        player->m_robotSprite->setVisible(true);
        player->m_robotSprite->m_color = player->m_firstLayer->getColor();
        player->m_robotSprite->m_secondColor = player->m_secondLayer->getColor();
        player->m_robotSprite->updateColors();
        updateRobotSprite(player->m_robotSprite, icon, type);
    }
    else if (player->m_robotSprite) player->m_robotSprite->setVisible(false);

    if (type == IconType::Spider) {
        if (!player->m_spiderSprite) player->createSpiderSprite(1);
        player->m_spiderSprite->setVisible(true);
        player->m_spiderSprite->m_color = player->m_firstLayer->getColor();
        player->m_spiderSprite->m_secondColor = player->m_secondLayer->getColor();
        player->m_spiderSprite->updateColors();
        updateRobotSprite(player->m_spiderSprite, icon, type);
    }
    else if (player->m_spiderSprite) player->m_spiderSprite->setVisible(false);

    if (type == IconType::Robot || type == IconType::Spider) return;

    loadIcon(icon, type, player->m_iconRequestID);

    player->m_firstLayer->setDisplayFrame(MoreIcons::getFrame(fmt::format("{}_001.png"_spr, icon)));
    player->m_firstLayer->setScale(type == IconType::Ball ? 0.9f : 1.0f);
    player->m_firstLayer->setPosition({ 0.0f, type == IconType::Ufo ? -7.0f : 0.0f });
    player->m_secondLayer->setDisplayFrame(MoreIcons::getFrame(fmt::format("{}_2_001.png"_spr, icon)));
    auto firstCenter = player->m_firstLayer->getContentSize() / 2.0f;
    player->m_secondLayer->setPosition(firstCenter);
    player->m_outlineSprite->setDisplayFrame(MoreIcons::getFrame(fmt::format("{}_glow_001.png"_spr, icon)));
    player->m_outlineSprite->setPosition(firstCenter);
    if (type == IconType::Ufo) {
        player->m_birdDome->setDisplayFrame(MoreIcons::getFrame(fmt::format("{}_3_001.png"_spr, icon)));
        player->m_birdDome->setPosition(firstCenter);
    }
    auto extraFrame = MoreIcons::getFrame(fmt::format("{}_extra_001.png"_spr, icon));
    player->m_detailSprite->setVisible(extraFrame != nullptr);
    if (extraFrame) {
        player->m_detailSprite->setDisplayFrame(extraFrame);
        player->m_detailSprite->setPosition(firstCenter);
    }
}

void more_icons::updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type) {
    if (!sprite) return;

    auto info = getIcon(icon, type);
    if (!info) return;

    sprite->setUserObject("name"_spr, CCString::create(icon));

    auto texture = loadIcon(icon, type, sprite->m_iconRequestID);

    sprite->setBatchNode(nullptr);
    sprite->setTexture(texture);
    sprite->m_paSprite->setBatchNode(nullptr);
    sprite->m_paSprite->setTexture(texture);

    auto spriteParts = sprite->m_paSprite->m_spriteParts;
    for (int i = 0; i < spriteParts->count(); i++) {
        auto spritePart = static_cast<CCSprite*>(spriteParts->objectAtIndex(i));
        auto tag = spritePart->getTag();

        spritePart->setBatchNode(nullptr);
        spritePart->setDisplayFrame(MoreIcons::getFrame(fmt::format("{}_{:02}_001.png"_spr, icon, tag)));
        if (auto secondSprite = static_cast<CCSprite*>(sprite->m_secondArray->objectAtIndex(i))) {
            secondSprite->setBatchNode(nullptr);
            secondSprite->setDisplayFrame(MoreIcons::getFrame(fmt::format("{}_{:02}_2_001.png"_spr, icon, tag)));
            secondSprite->setPosition(spritePart->getContentSize() / 2.0f);
        }

        if (auto glowChild = sprite->m_glowSprite->getChildByIndex<CCSprite>(i)) {
            glowChild->setBatchNode(nullptr);
            glowChild->setDisplayFrame(MoreIcons::getFrame(fmt::format("{}_{:02}_glow_001.png"_spr, icon, tag)));
        }

        if (spritePart == sprite->m_headSprite) {
            auto extraFrame = MoreIcons::getFrame(fmt::format("{}_{:02}_extra_001.png"_spr, icon, tag));
            if (extraFrame) {
                if (sprite->m_extraSprite) {
                    sprite->m_extraSprite->setBatchNode(nullptr);
                    sprite->m_extraSprite->setDisplayFrame(extraFrame);
                }
                else {
                    sprite->m_extraSprite = CCSprite::createWithSpriteFrame(extraFrame);
                    spritePart->addChild(sprite->m_extraSprite, 2);
                }
                sprite->m_extraSprite->setPosition(spritePart->getContentSize() / 2.0f);
            }
            sprite->m_extraSprite->setVisible(extraFrame != nullptr);
        }
    }
}

void more_icons::updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type) {
    if (!object || icon.empty() || !hasIcon(icon, type)) return;

    object->setUserObject("name"_spr, CCString::create(icon));

    if (type == IconType::Robot) {
        if (Ref robotSprite = object->m_robotSprite) {
            robotSprite->removeFromParentAndCleanup(false);
            updateRobotSprite(robotSprite, icon, type);
            object->m_robotBatchNode->setTexture(robotSprite->getTexture());
            object->m_robotBatchNode->addChild(robotSprite);
        }
        return;
    }
    else if (type == IconType::Spider) {
        if (Ref spiderSprite = object->m_spiderSprite) {
            spiderSprite->removeFromParentAndCleanup(false);
            updateRobotSprite(spiderSprite, icon, type);
            object->m_spiderBatchNode->setTexture(spiderSprite->getTexture());
            object->m_spiderBatchNode->addChild(spiderSprite);
        }
        return;
    }

    loadIcon(icon, type, object->m_iconRequestID);

    auto isVehicle = type == IconType::Ship || type == IconType::Ufo || type == IconType::Jetpack;
    auto firstLayer = isVehicle ? object->m_vehicleSprite : object->m_iconSprite;
    auto secondLayer = isVehicle ? object->m_vehicleSpriteSecondary : object->m_iconSpriteSecondary;
    auto outlineSprite = isVehicle ? object->m_vehicleGlow : object->m_iconGlow;
    auto detailSprite = isVehicle ? object->m_vehicleSpriteWhitener : object->m_iconSpriteWhitener;

    firstLayer->setDisplayFrame(MoreIcons::getFrame(fmt::format("{}_001.png"_spr, icon)));
    secondLayer->setDisplayFrame(MoreIcons::getFrame(fmt::format("{}_2_001.png"_spr, icon)));
    auto firstCenter = firstLayer->getContentSize() / 2.0f;
    secondLayer->setPosition(firstCenter);
    if (type == IconType::Ufo) {
        object->m_birdVehicle->setDisplayFrame(MoreIcons::getFrame(fmt::format("{}_3_001.png"_spr, icon)));
        object->m_birdVehicle->setPosition(firstCenter);
    }
    outlineSprite->setDisplayFrame(MoreIcons::getFrame(fmt::format("{}_glow_001.png"_spr, icon)));
    auto extraFrame = MoreIcons::getFrame(fmt::format("{}_extra_001.png"_spr, icon));
    detailSprite->setVisible(extraFrame != nullptr);
    if (extraFrame) {
        detailSprite->setDisplayFrame(extraFrame);
        detailSprite->setPosition(firstCenter);
    }
}

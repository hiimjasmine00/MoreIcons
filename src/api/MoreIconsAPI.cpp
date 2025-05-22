#include "MoreIconsAPI.hpp"
#include "../classes/misc/ThreadPool.hpp"
#ifdef GEODE_IS_ANDROID
#include <android/asset_manager_jni.h>
#endif
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#ifdef GEODE_IS_ANDROID
#include <Geode/cocos/platform/android/jni/JniHelper.h>
#endif
#include <Geode/utils/ranges.hpp>
#include <MoreIcons.hpp>
#include <texpack.hpp>

using namespace geode::prelude;

$execute {
    new EventListener(+[](SimplePlayer* player, const std::string& icon, IconType type) {
        MoreIconsAPI::updateSimplePlayer(player, icon, type);
        return ListenerResult::Propagate;
    }, MoreIcons::SimplePlayerFilter("simple-player"_spr));

    new EventListener(+[](GJRobotSprite* sprite, const std::string& icon, IconType type) {
        MoreIconsAPI::updateRobotSprite(sprite, icon, type);
        return ListenerResult::Propagate;
    }, MoreIcons::RobotSpriteFilter("robot-sprite"_spr));

    new EventListener(+[](PlayerObject* object, const std::string& icon, IconType type) {
        MoreIconsAPI::updatePlayerObject(object, icon, type);
        return ListenerResult::Propagate;
    }, MoreIcons::PlayerObjectFilter("player-object"_spr));

    new EventListener(+[](std::vector<IconInfo*>* vec) {
        vec->clear();
        for (int i = 0; i < MoreIconsAPI::icons.size(); i++) vec->push_back(MoreIconsAPI::icons.data() + i);
        return ListenerResult::Propagate;
    }, MoreIcons::AllIconsFilter("all-icons"_spr));

    new EventListener(+[](std::vector<IconInfo*>* vec, IconType type) {
        vec->clear();
        if (!MoreIconsAPI::iconIndices.contains(type)) return ListenerResult::Propagate;
        auto& [start, end] = MoreIconsAPI::iconIndices[type];
        for (int i = start; i < end; i++) vec->push_back(MoreIconsAPI::icons.data() + i);
        return ListenerResult::Propagate;
    }, MoreIcons::GetIconsFilter("get-icons"_spr));

    new EventListener(+[](IconInfo** info, std::string name, IconType type) {
        *info = MoreIconsAPI::getIcon(name, type);
        return ListenerResult::Propagate;
    }, MoreIcons::GetIconFilter("get-icon"_spr));

    new EventListener(+[](std::string icon, IconType type, int requestID) {
        MoreIconsAPI::loadIcon(icon, type, requestID);
        return ListenerResult::Propagate;
    }, MoreIcons::LoadIconFilter("load-icon"_spr));

    new EventListener(+[](std::string icon, IconType type, int requestID) {
        MoreIconsAPI::unloadIcon(icon, type, requestID);
        return ListenerResult::Propagate;
    }, MoreIcons::UnloadIconFilter("unload-icon"_spr));

    new EventListener(+[](int requestID) {
        MoreIconsAPI::unloadIcons(requestID);
        return ListenerResult::Propagate;
    }, MoreIcons::UnloadIconsFilter("unload-icons"_spr));
    #if GEODE_COMP_GD_VERSION == 22074 // Keep this until the next Geometry Dash update
    new EventListener(+[](std::vector<std::string>* vec, IconType type) {
        vec->clear();
        if (!MoreIconsAPI::iconIndices.contains(type)) return ListenerResult::Propagate;
        auto& [start, end] = MoreIconsAPI::iconIndices[type];
        for (int i = start; i < end; i++) vec->push_back(MoreIconsAPI::icons[i].name);
        return ListenerResult::Propagate;
    }, DispatchFilter<std::vector<std::string>*, IconType>("all-icons"_spr));
    #endif
}

IconInfo* MoreIconsAPI::getIcon(const std::string& name, IconType type) {
    auto found = std::ranges::find_if(icons, [name, type](const IconInfo& info) { return info.name == name && info.type == type; });
    return found != icons.end() ? icons.data() + (found - icons.begin()) : nullptr;
}

IconInfo* MoreIconsAPI::getIcon(IconType type, bool dual) {
    return getIcon(activeIcon(type, dual), type);
}

std::string MoreIconsAPI::activeIcon(IconType type, bool dual) {
    return MoreIcons::activeIcon(type, dual);
}

std::string MoreIconsAPI::setIcon(const std::string& icon, IconType type, bool dual) {
    return MoreIcons::setIcon(icon, type, dual);
}

IconType MoreIconsAPI::getIconType(PlayerObject* object) {
    return MoreIcons::getIconType(object);
}

bool MoreIconsAPI::hasIcon(const std::string& icon, IconType type) {
    return !icon.empty() && std::ranges::any_of(icons, [icon, type](const IconInfo& info) { return info.name == icon && info.type == type; });
}

int MoreIconsAPI::getCount(IconType type) {
    return std::ranges::count_if(icons, [type](const IconInfo& info) { return info.type == type; });
}

std::string MoreIconsAPI::getFrameName(const std::string& name, const std::string& prefix, IconType type) {
    auto modID = name.empty() ? "" : GEODE_MOD_ID "/";
    if (type != IconType::Robot && type != IconType::Spider) {
        if (name.ends_with("_2_001.png")) return fmt::format("{}{}_2_001.png", modID, prefix);
        else if (name.ends_with("_3_001.png")) return fmt::format("{}{}_3_001.png", modID, prefix);
        else if (name.ends_with("_extra_001.png")) return fmt::format("{}{}_extra_001.png", modID, prefix);
        else if (name.ends_with("_glow_001.png")) return fmt::format("{}{}_glow_001.png", modID, prefix);
        else if (name.ends_with("_001.png")) return fmt::format("{}{}_001.png", modID, prefix);
    }
    else for (int i = 1; i < 5; i++) {
        if (name.ends_with(fmt::format("_{:02}_2_001.png", i))) return fmt::format("{}{}_{:02}_2_001.png", modID, prefix, i);
        else if (i == 1 && name.ends_with(fmt::format("_{:02}_extra_001.png", i))) return fmt::format("{}{}_{:02}_extra_001.png", modID, prefix, i);
        else if (name.ends_with(fmt::format("_{:02}_glow_001.png", i))) return fmt::format("{}{}_{:02}_glow_001.png", modID, prefix, i);
        else if (name.ends_with(fmt::format("_{:02}_001.png", i))) return fmt::format("{}{}_{:02}_001.png", modID, prefix, i);
    }

    return name;
}

struct Image {
    std::string name;
    std::vector<uint8_t> data;
    CCDictionary* frames;
    uint32_t width;
    uint32_t height;
};

std::mutex imageMutex;
std::vector<Image> images;

void loadFolderIcon(const IconInfo& info, bool async) {
    texpack::Packer packer;
    auto& folderName = info.folderName;

    for (int i = 0; i < info.textures.size(); i++) {
        auto& frameName = info.frameNames[i];
        auto res = packer.frame(frameName, info.textures[i]).mapErr([&frameName](const std::string& err) {
            return fmt::format("{}: {}", frameName, err);
        });
        if (res.isErr()) return log::error("{}: {}", folderName, res.unwrapErr());
    }

    auto packRes = packer.pack();
    if (packRes.isErr()) return log::error("{}: {}", folderName, packRes.unwrapErr());

    auto& image = packer.image();

    if (async) {
        auto frames = new CCDictionary();
        for (auto& frame : packer.frames()) {
            auto spriteFrame = new CCSpriteFrame();
            spriteFrame->initWithTexture(
                nullptr,
                { (float)frame.rect.origin.x, (float)frame.rect.origin.y, (float)frame.rect.size.width, (float)frame.rect.size.height },
                frame.rotated,
                { (float)frame.offset.x, (float)frame.offset.y },
                { (float)frame.size.width, (float)frame.size.height }
            );
            frames->setObject(spriteFrame, frame.name);
            spriteFrame->release();
        }

        std::unique_lock lock(imageMutex);

        return images.push_back({ info.folderName, image.data, frames, image.width, image.height });
    }

    auto texture = new CCTexture2D();
    if (!texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
        (float)image.width,
        (float)image.height
    })) return texture->release(), log::error("{}: Failed to load texture", folderName);

    CCTextureCache::get()->m_pTextures->setObject(texture, folderName);
    texture->release();

    auto spriteFrameCache = CCSpriteFrameCache::get();
    for (auto& frame : packer.frames()) {
        spriteFrameCache->addSpriteFrame(CCSpriteFrame::createWithTexture(
            texture,
            { (float)frame.rect.origin.x, (float)frame.rect.origin.y, (float)frame.rect.size.width, (float)frame.rect.size.height },
            frame.rotated,
            { (float)frame.offset.x, (float)frame.offset.y },
            { (float)frame.size.width, (float)frame.size.height }
        ), frame.name.c_str());
    }
}

void loadFileIcon(const IconInfo& info, bool async) {
    auto& textureName = info.textures[0];

    auto imageRes = texpack::fromPNG(textureName);
    if (imageRes.isErr()) return log::error("{}: {}", textureName, imageRes.unwrapErr());

    auto image = imageRes.unwrap();

    auto sheet = MoreIconsAPI::createDictionary(info.sheetName, async);
    if (!sheet) return log::error("{}: Failed to load sheet", info.sheetName);

    auto frames = static_cast<CCDictionary*>(sheet->objectForKey("frames"));
    if (!frames) return sheet->release(), log::error("{}: Failed to load frames", info.sheetName);

    auto metadata = static_cast<CCDictionary*>(sheet->objectForKey("metadata"));
    auto formatStr = metadata ? metadata->valueForKey("format") : nullptr;
    auto format = formatStr ? numFromString<int>(formatStr->m_sString).unwrapOr(0) : 0;

    if (async) {
        auto newFrames = new CCDictionary();
        for (auto [frame, dict] : CCDictionaryExt<std::string, CCDictionary*>(frames)) {
            if (auto spriteFrame = MoreIconsAPI::createSpriteFrame(dict, nullptr, format)) {
                newFrames->setObject(spriteFrame, MoreIconsAPI::getFrameName(frame, info.name, info.type));
                spriteFrame->release();
            }
        }

        sheet->release();

        std::unique_lock lock(imageMutex);

        return images.push_back({ textureName, image.data, newFrames, image.width, image.height });
    }

    auto texture = new CCTexture2D();
    if (!texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
        (float)image.width,
        (float)image.height
    })) return texture->release(), log::error("{}: Failed to load texture", info.sheetName);

    CCTextureCache::get()->m_pTextures->setObject(texture, textureName);
    texture->release();

    auto spriteFrameCache = CCSpriteFrameCache::get();
    for (auto [frame, dict] : CCDictionaryExt<std::string, CCDictionary*>(frames)) {
        if (auto spriteFrame = MoreIconsAPI::createSpriteFrame(dict, texture, format))
            spriteFrameCache->addSpriteFrame(spriteFrame, MoreIconsAPI::getFrameName(frame, info.name, info.type).c_str());
    }

    sheet->release();
}

void MoreIconsAPI::loadIcon(const std::string& name, IconType type, int requestID) {
    if (preloadIcons || !hasIcon(name, type)) return;

    auto& loadedIcon = loadedIcons[{ name, type }];

    if (loadedIcon < 1) {
        if (auto info = getIcon(name, type)) {
            if (!info->folderName.empty()) loadFolderIcon(*info, false);
            else if (!info->sheetName.empty()) loadFileIcon(*info, false);
        }
    }

    auto& requestedIcon = requestedIcons[requestID][type];
    if (requestedIcon != name) {
        loadedIcon++;
        if (!requestedIcon.empty()) unloadIcon(requestedIcon, type, requestID);
        requestedIcon = name;
    }
}

void MoreIconsAPI::loadIconAsync(const IconInfo& info) {
    auto& threadPool = ThreadPool::get();
    if (!info.folderName.empty()) threadPool.pushTask([info] { loadFolderIcon(info, true); });
    else if (!info.sheetName.empty()) threadPool.pushTask([info] { loadFileIcon(info, true); });
    else if (info.textures.size() > 0) threadPool.pushTask([info] {
        auto& textureName = info.textures[0];
        auto imageRes = texpack::fromPNG(textureName);
        if (imageRes.isErr()) return log::error("{}: {}", textureName, imageRes.unwrapErr());

        auto image = imageRes.unwrap();

        std::unique_lock lock(imageMutex);

        images.push_back({ textureName, image.data, nullptr, image.width, image.height });
    });
}

void MoreIconsAPI::finishLoadIcons() {
    std::unique_lock lock(imageMutex);

    auto textureCache = CCTextureCache::get();
    auto spriteFrameCache = CCSpriteFrameCache::get();
    for (auto& image : images) {
        auto texture = new CCTexture2D();
        if (!texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
            (float)image.width,
            (float)image.height
        })) {
            texture->release();
            continue;
        }

        textureCache->m_pTextures->setObject(texture, image.name);
        texture->release();

        if (!image.frames) continue;

        for (auto [frameName, frame] : CCDictionaryExt<std::string, CCSpriteFrame*>(image.frames)) {
            frame->setTexture(texture);
            spriteFrameCache->addSpriteFrame(frame, frameName.c_str());
        }

        image.frames->release();
    }

    images.clear();
}

void unloadFolderIcon(const IconInfo& info) {
    auto spriteFrameCache = CCSpriteFrameCache::get();
    for (auto& frame : info.frameNames) {
        spriteFrameCache->removeSpriteFrameByName(frame.c_str());
    }

    CCTextureCache::get()->removeTextureForKey(info.folderName.c_str());
}

void unloadFileIcon(const IconInfo& info) {
    auto sheet = MoreIconsAPI::createDictionary(info.sheetName, false);
    if (!sheet) return log::error("{}: Failed to load sheet", info.sheetName);

    auto frames = static_cast<CCDictionary*>(sheet->objectForKey("frames"));
    if (!frames) return log::error("{}: Failed to load frames", info.sheetName);

    auto spriteFrameCache = CCSpriteFrameCache::get();
    for (auto [frame, dict] : CCDictionaryExt<std::string, CCDictionary*>(frames)) {
        spriteFrameCache->removeSpriteFrameByName(MoreIconsAPI::getFrameName(frame, info.name, info.type).c_str());
    }

    CCTextureCache::get()->removeTextureForKey(info.textures[0].c_str());
}

void MoreIconsAPI::unloadIcon(const std::string& name, IconType type, int requestID) {
    if (preloadIcons || !hasIcon(name, type)) return;

    auto& loadedIcon = loadedIcons[{ name, type }];

    loadedIcon--;
    if (loadedIcon < 1) {
        if (auto info = getIcon(name, type)) {
            if (!info->folderName.empty()) unloadFolderIcon(*info);
            else if (!info->sheetName.empty()) unloadFileIcon(*info);
        }
    }

    requestedIcons[requestID].erase(type);
    if (requestedIcons[requestID].empty()) requestedIcons.erase(requestID);
}

void MoreIconsAPI::unloadIcons(int requestID) {
    if (!requestedIcons.contains(requestID)) return;

    auto& iconRequests = requestedIcons[requestID];
    for (int i = 0; i < 9; i++) {
        auto type = (IconType)i;
        if (!iconRequests.contains(type)) continue;
        auto icon = iconRequests[type];
        if (!icon.empty()) unloadIcon(icon, type, requestID);
    }

    requestedIcons.erase(requestID);
}

void MoreIconsAPI::updateSimplePlayer(SimplePlayer* player, IconType type, bool dual) {
    updateSimplePlayer(player, activeIcon(type, dual), type);
}

void MoreIconsAPI::updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type) {
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
        return updateRobotSprite(player->m_robotSprite, icon, type);
    }
    else if (player->m_robotSprite) player->m_robotSprite->setVisible(false);

    if (type == IconType::Spider) {
        if (!player->m_spiderSprite) player->createSpiderSprite(1);
        player->m_spiderSprite->setVisible(true);
        player->m_spiderSprite->m_color = player->m_firstLayer->getColor();
        player->m_spiderSprite->m_secondColor = player->m_secondLayer->getColor();
        player->m_spiderSprite->updateColors();
        return updateRobotSprite(player->m_spiderSprite, icon, type);
    }
    else if (player->m_spiderSprite) player->m_spiderSprite->setVisible(false);

    loadIcon(icon, type, player->m_iconRequestID);

    auto sfc = CCSpriteFrameCache::get();
    player->m_firstLayer->setDisplayFrame(sfc->spriteFrameByName(fmt::format("{}_001.png"_spr, icon).c_str()));
    player->m_firstLayer->setScale(type == IconType::Ball ? 0.9f : 1.0f);
    player->m_firstLayer->setPosition({ 0.0f, type == IconType::Ufo ? -7.0f : 0.0f });
    player->m_secondLayer->setDisplayFrame(sfc->spriteFrameByName(fmt::format("{}_2_001.png"_spr, icon).c_str()));
    auto firstCenter = player->m_firstLayer->getContentSize() / 2;
    player->m_secondLayer->setPosition(firstCenter);
    player->m_outlineSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("{}_glow_001.png"_spr, icon).c_str()));
    player->m_outlineSprite->setPosition(firstCenter);
    if (type == IconType::Ufo) {
        player->m_birdDome->setDisplayFrame(sfc->spriteFrameByName(fmt::format("{}_3_001.png"_spr, icon).c_str()));
        player->m_birdDome->setPosition(firstCenter);
    }
    auto extraFrame = sfc->spriteFrameByName(fmt::format("{}_extra_001.png"_spr, icon).c_str());
    auto extraVisible = extraFrame && extraFrame->getTag() != 105871529;
    player->m_detailSprite->setVisible(extraVisible);
    if (extraVisible) {
        player->m_detailSprite->setDisplayFrame(extraFrame);
        player->m_detailSprite->setPosition(firstCenter);
    }
}

void MoreIconsAPI::updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type) {
    if (!sprite || icon.empty() || !hasIcon(icon, type)) return;

    auto info = getIcon(icon, type);
    if (!info) return;

    sprite->setUserObject("name"_spr, CCString::create(icon));

    auto texture = CCTextureCache::get()->textureForKey((info->folderName.empty() ? info->textures[0] : info->folderName).c_str());

    sprite->setBatchNode(nullptr);
    sprite->setTexture(texture);
    sprite->m_paSprite->setBatchNode(nullptr);
    sprite->m_paSprite->setTexture(texture);

    loadIcon(icon, type, sprite->m_iconRequestID);

    auto spriteParts = sprite->m_paSprite->m_spriteParts;
    auto sfc = CCSpriteFrameCache::get();
    for (int i = 0; i < spriteParts->count(); i++) {
        auto spritePart = static_cast<CCSprite*>(spriteParts->objectAtIndex(i));
        auto tag = spritePart->getTag();

        spritePart->setBatchNode(nullptr);
        spritePart->setDisplayFrame(sfc->spriteFrameByName(fmt::format("{}_{:02}_001.png"_spr, icon, tag).c_str()));
        if (auto secondSprite = static_cast<CCSprite*>(sprite->m_secondArray->objectAtIndex(i))) {
            secondSprite->setBatchNode(nullptr);
            secondSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("{}_{:02}_2_001.png"_spr, icon, tag).c_str()));
            secondSprite->setPosition(spritePart->getContentSize() / 2);
        }

        auto glowChild = static_cast<CCSprite*>(sprite->m_glowSprite->getChildren()->objectAtIndex(i));
        glowChild->setBatchNode(nullptr);
        glowChild->setDisplayFrame(sfc->spriteFrameByName(fmt::format("{}_{:02}_glow_001.png"_spr, icon, tag).c_str()));

        if (spritePart == sprite->m_headSprite) {
            auto extraFrame = sfc->spriteFrameByName(fmt::format("{}_{:02}_extra_001.png"_spr, icon, tag).c_str());
            auto hasExtra = extraFrame && extraFrame->getTag() != 105871529;
            if (hasExtra) {
                if (sprite->m_extraSprite) {
                    sprite->m_extraSprite->setBatchNode(nullptr);
                    sprite->m_extraSprite->setDisplayFrame(extraFrame);
                }
                else {
                    sprite->m_extraSprite = CCSprite::createWithSpriteFrame(extraFrame);
                    sprite->m_headSprite->addChild(sprite->m_extraSprite, 2);
                }
                sprite->m_extraSprite->setPosition(spritePart->getContentSize() / 2);
            }
            sprite->m_extraSprite->setVisible(hasExtra);
        }
    }
}

void MoreIconsAPI::updatePlayerObject(PlayerObject* object, IconType type, bool dual) {
    updatePlayerObject(object, activeIcon(type, dual), type);
}

void MoreIconsAPI::updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type) {
    if (!object || icon.empty() || !hasIcon(icon, type)) return;

    auto info = getIcon(icon, type);
    if (!info) return;

    object->setUserObject("name"_spr, CCString::create(icon));

    if (type == IconType::Robot || type == IconType::Spider) {
        auto robotSprite = type == IconType::Spider ? object->m_spiderSprite : object->m_robotSprite;
        auto batchNode = type == IconType::Spider ? object->m_spiderBatchNode : object->m_robotBatchNode;

        if (robotSprite->getParent() == batchNode) {
            robotSprite->retain();
            robotSprite->removeFromParentAndCleanup(false);
            updateRobotSprite(robotSprite, icon, type);
            batchNode->setTexture(robotSprite->getTexture());
            batchNode->addChild(robotSprite);
            robotSprite->release();
        }
        return;
    }

    loadIcon(icon, type, object->m_iconRequestID);

    auto isVehicle = type == IconType::Ship || type == IconType::Ufo || type == IconType::Jetpack;
    auto firstLayer = isVehicle ? object->m_vehicleSprite : object->m_iconSprite;
    auto secondLayer = isVehicle ? object->m_vehicleSpriteSecondary : object->m_iconSpriteSecondary;
    auto outlineSprite = isVehicle ? object->m_vehicleGlow : object->m_iconGlow;
    auto detailSprite = isVehicle ? object->m_vehicleSpriteWhitener : object->m_iconSpriteWhitener;

    auto sfc = CCSpriteFrameCache::get();
    firstLayer->setDisplayFrame(sfc->spriteFrameByName(fmt::format("{}_001.png"_spr, icon).c_str()));
    secondLayer->setDisplayFrame(sfc->spriteFrameByName(fmt::format("{}_2_001.png"_spr, icon).c_str()));
    auto firstCenter = firstLayer->getContentSize() / 2;
    secondLayer->setPosition(firstCenter);
    if (type == IconType::Ufo) {
        object->m_birdVehicle->setDisplayFrame(sfc->spriteFrameByName(fmt::format("{}_3_001.png"_spr, icon).c_str()));
        object->m_birdVehicle->setPosition(firstCenter);
    }
    outlineSprite->setDisplayFrame(sfc->spriteFrameByName(fmt::format("{}_glow_001.png"_spr, icon).c_str()));
    auto extraFrame = sfc->spriteFrameByName(fmt::format("{}_extra_001.png"_spr, icon).c_str());
    auto extraVisible = extraFrame && extraFrame->getTag() != 105871529;
    detailSprite->setVisible(extraVisible);
    if (extraVisible) {
        detailSprite->setDisplayFrame(extraFrame);
        detailSprite->setPosition(firstCenter);
    }
}

std::vector<float> floatsFromString(const std::string& str) {
    return ranges::map<std::vector<float>>(string::split(string::replace(string::replace(str, "{", ""), "}", ""), ","),
        [](const std::string& s) { return numFromString<float>(s).unwrapOr(0.0f); });
}

CCPoint pointFromString(const CCString* str) {
    if (!str) return { 0.0f, 0.0f };
    auto floats = floatsFromString(str->m_sString);
    return {
        floats.size() > 0 ? floats[0] : 0.0f,
        floats.size() > 1 ? floats[1] : 0.0f
    };
}

CCSize sizeFromString(const CCString* str) {
    if (!str) return { 0.0f, 0.0f };
    auto floats = floatsFromString(str->m_sString);
    return {
        floats.size() > 0 ? floats[0] : 0.0f,
        floats.size() > 1 ? floats[1] : 0.0f
    };
}

CCRect rectFromString(const CCString* str) {
    if (!str) return { 0.0f, 0.0f, 0.0f, 0.0f };
    auto floats = floatsFromString(str->m_sString);
    return {
        floats.size() > 0 ? floats[0] : 0.0f,
        floats.size() > 1 ? floats[1] : 0.0f,
        floats.size() > 2 ? floats[2] : 0.0f,
        floats.size() > 3 ? floats[3] : 0.0f
    };
}

bool boolValue(const CCString* str) {
    if (!str) return false;
    return str->m_sString != "0" && str->m_sString != "false";
}

int intValue(const CCString* str) {
    if (!str) return 0;
    return numFromString<int>(str->m_sString).unwrapOr(0);
}

float floatValue(const CCString* str) {
    if (!str) return 0.0f;
    return numFromString<float>(str->m_sString).unwrapOr(0.0f);
}

CCSpriteFrame* MoreIconsAPI::createSpriteFrame(CCDictionary* dict, CCTexture2D* texture, int format) {
    if (!dict || format < 0 || format > 3) return nullptr;

    auto spriteFrame = new CCSpriteFrame();
    switch (format) {
        case 0:
            spriteFrame->initWithTexture(
                texture,
                {
                    floatValue(dict->valueForKey("x")),
                    floatValue(dict->valueForKey("y")),
                    floatValue(dict->valueForKey("width")),
                    floatValue(dict->valueForKey("height"))
                },
                false,
                {
                    floatValue(dict->valueForKey("offsetX")),
                    floatValue(dict->valueForKey("offsetY"))
                },
                {
                    (float)abs(intValue(dict->valueForKey("originalWidth"))),
                    (float)abs(intValue(dict->valueForKey("originalHeight")))
                }
            );
            break;
        case 1:
            spriteFrame->initWithTexture(
                texture,
                rectFromString(dict->valueForKey("frame")),
                false,
                pointFromString(dict->valueForKey("offset")),
                sizeFromString(dict->valueForKey("sourceSize"))
            );
            break;
        case 2:
            spriteFrame->createWithTexture(
                texture,
                rectFromString(dict->valueForKey("frame")),
                boolValue(dict->valueForKey("rotated")),
                pointFromString(dict->valueForKey("offset")),
                sizeFromString(dict->valueForKey("sourceSize"))
            );
            break;
        case 3:
            spriteFrame->initWithTexture(
                texture,
                {
                    pointFromString(dict->valueForKey("textureRect")),
                    sizeFromString(dict->valueForKey("spriteSize"))
                },
                boolValue(dict->valueForKey("textureRotated")),
                pointFromString(dict->valueForKey("spriteOffset")),
                sizeFromString(dict->valueForKey("spriteSourceSize"))
            );
            break;
    }

    if (texture) spriteFrame->autorelease();
    return spriteFrame;
}

#ifdef GEODE_IS_ANDROID
std::mutex androidMutex;
AAssetManager* assetManager = nullptr;

$on_mod(Loaded) {
    JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t, "org/fmod/FMOD", "getAssetManager", "()Landroid/content/res/AssetManager;")) {
        auto r = t.env->CallStaticObjectMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
        assetManager = AAssetManager_fromJava(t.env, r);
    }
    if (!assetManager) log::error("Failed to get asset manager");
}

std::vector<uint8_t> getData(const std::string& path) {
    std::unique_lock lock(androidMutex);

    unsigned long size;
    if (auto data = CCFileUtils::get()->getFileData(path.c_str(), "rb", &size)) {
        std::vector<uint8_t> result(data, data + size);
        delete[] data;
        return result;
    }
    return {};
}
#endif

CCDictionary* MoreIconsAPI::createDictionary(const std::filesystem::path& path, bool async) {
    #ifdef GEODE_IS_ANDROID
    if (async && path.native().starts_with("assets/")) {
        std::unique_lock lock(androidMutex);
        return CCDictionary::createWithContentsOfFileThreadSafe(path.c_str());
    }
    #endif
    return CCDictionary::createWithContentsOfFileThreadSafe(GEODE_WINDOWS(string::wideToUtf8)(path.native()).c_str());
}

std::vector<uint8_t> MoreIconsAPI::getFileData(const std::filesystem::path& path) {
    #ifdef GEODE_IS_ANDROID
    if (path.native().starts_with("assets/")) {
        if (!assetManager) return getData(path);

        auto asset = AAssetManager_open(assetManager, path.native().substr(7).c_str(), AASSET_MODE_BUFFER);
        if (!asset) {
            log::error("{}: Failed to open asset", path);
            return getData(path);
        }

        auto size = AAsset_getLength(asset);
        auto data = new uint8_t[size];
        if (AAsset_read(asset, data, size) != size) {
            delete[] data;
            AAsset_close(asset);
            log::error("{}: Failed to read asset", path);
            return getData(path);
        }
        AAsset_close(asset);

        std::vector<uint8_t> result(data, data + size);
        delete[] data;
        return result;
    }
    #endif
    return file::readBinary(path).unwrapOr(std::vector<uint8_t>());
}

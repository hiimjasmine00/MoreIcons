#include <pugixml.hpp>
#include "MoreIconsAPI.hpp"
#include "../classes/misc/ThreadPool.hpp"
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#ifdef GEODE_IS_ANDROID
#include <Geode/cocos/platform/android/jni/Java_org_cocos2dx_lib_Cocos2dxHelper.h>
#endif
#include <MoreIcons.hpp>
#include <texpack.hpp>

using namespace geode::prelude;
using namespace std::string_view_literals;

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
        if (!MoreIconsAPI::iconSpans.contains(type)) return ListenerResult::Propagate;
        auto& iconSpan = MoreIconsAPI::iconSpans[type];
        for (auto it = iconSpan.data(), end = iconSpan.data() + iconSpan.size(); it < end; it++) vec->push_back(it);
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
        if (!MoreIconsAPI::iconSpans.contains(type)) return ListenerResult::Propagate;
        auto& iconSpan = MoreIconsAPI::iconSpans[type];
        for (auto it = iconSpan.data(), end = iconSpan.data() + iconSpan.size(); it < end; it++) vec->push_back(it->name);
        return ListenerResult::Propagate;
    }, DispatchFilter<std::vector<std::string>*, IconType>("all-icons"_spr));
    #endif
}

IconInfo* MoreIconsAPI::getIcon(const std::string& name, IconType type) {
    if (name.empty()) return nullptr;
    auto& iconSpan = iconSpans[type];
    auto found = std::ranges::find_if(iconSpan, [&name](const IconInfo& info) { return info.name == name; });
    return found < iconSpan.end() ? std::to_address(found) : nullptr;
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

std::string MoreIconsAPI::getIconName(cocos2d::CCNode* node) {
    return MoreIcons::getIconName(node);
}

bool MoreIconsAPI::hasIcon(const std::string& icon, IconType type) {
    if (icon.empty() || !iconSpans.contains(type)) return false;
    return std::ranges::any_of(iconSpans[type], [&icon](const IconInfo& info) { return info.name == icon; });
}

bool MoreIconsAPI::hasIcon(IconType type, bool dual) {
    return hasIcon(activeIcon(type, dual), type);
}

int MoreIconsAPI::getCount(IconType type) {
    return iconSpans.contains(type) ? iconSpans[type].size() : 0;
}

std::string MoreIconsAPI::iconName(int id, IconType type) {
    return iconName(id, GameManager::get()->iconTypeToUnlockType(type));
}

std::string MoreIconsAPI::iconName(int id, UnlockType type) {
    constexpr std::array prefixes = {
        "", "player_", "", "", "ship_", "player_ball_", "bird_", "dart_",
        "robot_", "spider_", "streak_", "PlayerExplosion_", "", "swing_", "jetpack_", "shipfire"
    };

    return fmt::format("{}{:02}", prefixes[(int)type], id);
}

CCSpriteFrame* getFrameByName(const std::string& name) {
    auto spriteFrame = static_cast<CCSpriteFrame*>(CCSpriteFrameCache::get()->m_pSpriteFrames->objectForKey(name));
    if (!spriteFrame || spriteFrame->getTag() == 105871529) spriteFrame = nullptr;
    return spriteFrame;
}

CCSpriteFrame* MoreIconsAPI::getFrameInternal(fmt::string_view format, fmt::format_args args) {
    return getFrameByName(fmt::vformat(format, args));
}

CCSprite* MoreIconsAPI::customTrail(const std::string& png) {
    auto square = CCSprite::createWithSpriteFrameName("playerSquare_001.png");
    square->setColor({ 150, 150, 150 });

    auto streak = CCSprite::create(png.c_str());
    limitNodeHeight(streak, 27.0f, 999.0f, 0.001f);
    streak->setRotation(-90.0f);
    streak->setPosition(square->getContentSize() / 2.0f);
    square->addChild(streak);

    return square;
}

CCTexture2D* MoreIconsAPI::loadIcon(const std::string& name, IconType type, int requestID) {
    auto info = getIcon(name, type);
    if (!info) return nullptr;

    auto texture = CCTextureCache::get()->textureForKey(info->textures[0].c_str());
    if (preloadIcons) return texture;

    auto& loadedIcon = loadedIcons[{ name, type }];

    if (loadedIcon < 1) {
        GEODE_UNWRAP_OR_ELSE(image, err, createFrames(info->textures[0], info->sheetName, info->name, info->type, info))
            log::error("{}: {}", info->name, err);
        else {
            addFrames(image);
            texture = image.texture;
        }
    }

    auto& requestedIcon = requestedIcons[requestID][type];
    if (requestedIcon != name) {
        loadedIcon++;
        if (!requestedIcon.empty()) unloadIcon(requestedIcon, type, requestID);
        requestedIcon = name;
    }

    return texture;
}

void MoreIconsAPI::loadIcons(IconType type, bool logs) {
    if (!preloadIcons) return;

    constexpr std::array lowercase = {
        "", "icon", "", "", "ship", "ball", "UFO", "wave",
        "robot", "spider", "trail", "death effect", "", "swing", "jetpack", "ship fire"
    };
    auto name = lowercase[(int)GameManager::get()->iconTypeToUnlockType(type)];

    auto& iconSpan = iconSpans[type];
    if (logs) log::info("Pre-loading {} {} textures", iconSpan.size(), name);

    std::vector<ImageResult> images;
    images.reserve(iconSpan.size());
    std::mutex imageMutex;

    auto& threadPool = ThreadPool::get();
    for (auto info = iconSpan.data(), end = iconSpan.data() + iconSpan.size(); info < end; info++) {
        threadPool.pushTask([info, &images, &imageMutex] {
            GEODE_UNWRAP_OR_ELSE(image, err, createFrames(info->textures[0], info->sheetName, info->name, info->type, info))
                return log::error("{}: {}", info->name, err);

            std::unique_lock lock(imageMutex);

            images.push_back(std::move(image));
        });
    }
    threadPool.wait();

    std::unique_lock lock(imageMutex);

    auto loaded = 0;
    while (!images.empty()) {
        addFrames(images[0]);
        images.erase(images.begin());
        loaded++;
    }

    if (logs) log::info("Finished pre-loading {} {} textures, {} remaining", loaded, name, iconSpan.size() - loaded);
}

void MoreIconsAPI::unloadIcon(const std::string& name, IconType type, int requestID) {
    if (preloadIcons) return;

    auto info = getIcon(name, type);
    if (!info) return;

    auto& loadedIcon = loadedIcons[{ name, type }];

    loadedIcon--;
    if (loadedIcon < 1) {
        auto spriteFrameCache = CCSpriteFrameCache::get();
        for (auto& frame : info->frameNames) {
            spriteFrameCache->m_pSpriteFrames->removeObjectForKey(frame);
        }
        info->frameNames.clear();

        CCTextureCache::get()->m_pTextures->removeObjectForKey(info->textures[0]);
    }

    requestedIcons[requestID].erase(type);
    if (requestedIcons[requestID].empty()) requestedIcons.erase(requestID);
}

void MoreIconsAPI::unloadIcons(int requestID) {
    if (preloadIcons || !requestedIcons.contains(requestID)) return;

    auto& iconRequests = requestedIcons[requestID];
    for (int i = 0; i < 9; i++) {
        auto type = (IconType)i;
        if (!iconRequests.contains(type)) continue;
        auto icon = iconRequests[type];
        if (!icon.empty()) unloadIcon(icon, type, requestID);
    }

    requestedIcons.erase(requestID);
}

std::string getFrameName(const std::string& name, const std::string& prefix, IconType type) {
    auto suffix = "";
    auto isRobot = type == IconType::Robot || type == IconType::Spider;

    if (name.ends_with("_2_001.png")) {
        if (isRobot) {
            if (name.ends_with("_01_2_001.png")) suffix = "_01_2_001.png";
            else if (name.ends_with("_02_2_001.png")) suffix = "_02_2_001.png";
            else if (name.ends_with("_03_2_001.png")) suffix = "_03_2_001.png";
            else if (name.ends_with("_04_2_001.png")) suffix = "_04_2_001.png";
        }
        else suffix = "_2_001.png";
    }
    else if (type == IconType::Ufo && name.ends_with("_3_001.png")) suffix = "_3_001.png";
    else if (name.ends_with("_extra_001.png")) {
        if (isRobot) {
            if (name.ends_with("_01_extra_001.png")) suffix = "_01_extra_001.png";
        }
        else suffix = "_extra_001.png";
    }
    else if (name.ends_with("_glow_001.png")) {
        if (isRobot) {
            if (name.ends_with("_01_glow_001.png")) suffix = "_01_glow_001.png";
            else if (name.ends_with("_02_glow_001.png")) suffix = "_02_glow_001.png";
            else if (name.ends_with("_03_glow_001.png")) suffix = "_03_glow_001.png";
            else if (name.ends_with("_04_glow_001.png")) suffix = "_04_glow_001.png";
        }
        else suffix = "_glow_001.png";
    }
    else if (name.ends_with("_001.png")) {
        if (isRobot) {
            if (name.ends_with("_01_001.png")) suffix = "_01_001.png";
            else if (name.ends_with("_02_001.png")) suffix = "_02_001.png";
            else if (name.ends_with("_03_001.png")) suffix = "_03_001.png";
            else if (name.ends_with("_04_001.png")) suffix = "_04_001.png";
        }
        else suffix = "_001.png";
    }

    return *suffix ? prefix.empty() ? suffix : fmt::format("{}{}"_spr, prefix, suffix) : name;
}

bool naturalSort(const std::string& a, const std::string& b) {
    auto aIt = a.begin();
    auto bIt = b.begin();

    while (aIt < a.end() && bIt < b.end()) {
        if (std::isdigit(*aIt) && std::isdigit(*bIt)) {
            std::string aNum, bNum;
            while (std::isdigit(*aIt)) aNum += *aIt++;
            while (std::isdigit(*bIt)) bNum += *bIt++;
            if (aNum != bNum) {
                if (aNum.size() != bNum.size()) return aNum.size() < bNum.size();
                for (int i = 0; i < aNum.size(); i++) {
                    if (aNum[i] != bNum[i]) return aNum[i] < bNum[i];
                }
            }
        }
        else {
            auto aLower = std::tolower(*aIt);
            auto bLower = std::tolower(*bIt);
            if (aLower != bLower) return aLower < bLower;
            aIt++;
            bIt++;
        }
    }

    return a.size() < b.size();
}

bool iconSort(const IconInfo& a, const IconInfo& b) {
    if (a.type != b.type) return a.type < b.type;
    if (a.packID != b.packID) {
        if (a.packID.empty()) return true;
        if (b.packID.empty()) return false;
        return naturalSort(a.packID, b.packID);
    }
    return naturalSort(a.shortName, b.shortName);
}

constexpr std::array types = {
    IconType::Cube, IconType::Ship, IconType::Ball, IconType::Ufo, IconType::Robot,
    IconType::Spider, IconType::Wave, IconType::Swing, IconType::Jetpack, IconType::Special
};

void MoreIconsAPI::addIcon(const IconInfo& info, bool postLoad) {
    size_t indices[types.size()];
    for (int i = 0; i < types.size(); i++) {
        auto type = types[i];
        if (iconSpans.contains(type)) indices[i] = iconSpans[type].data() - icons.data();
        else if (info.type == type) indices[i] = icons.size();
    }

    auto it = icons.insert(std::ranges::upper_bound(icons, info, [](const IconInfo& a, const IconInfo& b) { return iconSort(a, b); }), info);

    for (int i = 0; i < types.size(); i++) {
        auto type = types[i];
        if (!iconSpans.contains(type) && info.type != type) continue;
        auto& iconSpan = iconSpans[type];
        iconSpan = { icons.data() + indices[i] + (info.type < type), iconSpan.size() + (info.type == type) };
    }

    if (!preloadIcons || !postLoad) return;

    GEODE_UNWRAP_OR_ELSE(image, err, createFrames(info.textures[0], info.sheetName, info.name, info.type, std::to_address(it)))
        log::error("{}: {}", info.name, err);
    else addFrames(image);
}

void MoreIconsAPI::moveIcon(IconInfo* info, const std::filesystem::path& path) {
    auto oldPng = info->textures[0];
    auto newPng = string::pathToString(path / std::filesystem::path(oldPng).filename());
    info->textures[0] = newPng;
    info->sheetName = string::pathToString(path / std::filesystem::path(info->sheetName).filename());
    info->vanilla = false;

    auto textureCache = CCTextureCache::get();
    Ref texture = textureCache->textureForKey(oldPng.c_str());
    if (!texture) return;

    textureCache->m_pTextures->removeObjectForKey(oldPng);
    textureCache->m_pTextures->setObject(texture, info->textures[0]);
}

void MoreIconsAPI::removeIcon(IconInfo* info) {
    loadedIcons[{ info->name, info->type }] = 0;

    auto spriteFrameCache = CCSpriteFrameCache::get();
    for (auto& frame : info->frameNames) {
        spriteFrameCache->m_pSpriteFrames->removeObjectForKey(frame);
    }
    CCTextureCache::get()->m_pTextures->removeObjectForKey(info->textures[0]);

    auto type = info->type;
    if (!preloadIcons) {
        auto& name = info->name;
        std::vector<int> requestIDs;
        for (auto& [requestID, iconRequests] : requestedIcons) {
            if (iconRequests.contains(type) && iconRequests[type] == name) {
                iconRequests.erase(type);
                if (iconRequests.empty()) requestIDs.push_back(requestID);
            }
        }
        for (auto& requestID : requestIDs) {
            requestedIcons.erase(requestID);
        }
    }

    icons.erase(icons.begin() + (info - icons.data()));
    auto& iconSpan = iconSpans[type];
    iconSpan = { iconSpan.data(), iconSpan.size() - 1 };

    auto found = std::ranges::find(types, type);
    if (found < types.end() - 1) {
        for (auto it = found + 1; it < types.end(); it++) {
            auto& iconSpan = iconSpans[*it];
            iconSpan = { iconSpan.data() - 1, iconSpan.size() };
        }
    }
}

void MoreIconsAPI::renameIcon(IconInfo* info, const std::string& name) {
    auto oldName = info->name;
    auto newName = info->packID.empty() ? name : fmt::format("{}:{}", info->packID, name);
    info->name = newName;
    info->shortName = name;

    auto quality = "";
    auto oldPng = info->textures[0];
    auto type = info->type;
    if (type <= IconType::Jetpack) {
        if (oldPng.ends_with("-uhd.png")) quality = "-uhd";
        else if (oldPng.ends_with("-hd.png")) quality = "-hd";
    }

    auto newPng = string::pathToString(std::filesystem::path(oldPng).parent_path() / fmt::format("{}{}.png", name, quality));
    info->textures[0] = newPng;
    info->sheetName = string::pathToString(std::filesystem::path(info->sheetName).parent_path() / fmt::format("{}{}.plist", name, quality));

    auto textureCache = CCTextureCache::get();
    if (Ref texture = textureCache->textureForKey(oldPng.c_str())) {
        textureCache->m_pTextures->removeObjectForKey(oldPng);
        textureCache->m_pTextures->setObject(texture, newPng);

        auto spriteFrameCache = CCSpriteFrameCache::get();
        for (auto& frameName : info->frameNames) {
            if (Ref spriteFrame = getFrameByName(frameName)) {
                spriteFrameCache->m_pSpriteFrames->removeObjectForKey(frameName);
                frameName = getFrameName(frameName, newName, type);
                spriteFrameCache->m_pSpriteFrames->setObject(spriteFrame, frameName);
            }
        }
    }

    if (auto it = loadedIcons.find({ oldName, type }); it != loadedIcons.end()) {
        loadedIcons[{ newName, type }] = it->second;
        loadedIcons.erase(it);
    }

    for (auto& [requestID, iconRequests] : requestedIcons) {
        if (iconRequests.contains(type) && iconRequests[type] == oldName) iconRequests[type] = newName;
    }

    if (activeIcon(type, false) == oldName) setIcon(newName, type, false);
    if (activeIcon(type, true) == oldName) setIcon(newName, type, true);

    std::ranges::sort(icons, [](const IconInfo& a, const IconInfo& b) { return iconSort(a, b); });
}

void MoreIconsAPI::updateIcon(IconInfo* info) {
    auto texture = CCTextureCache::get()->textureForKey(info->textures[0].c_str());
    if (!texture) return;

    GEODE_UNWRAP_OR_ELSE(image, err, texpack::fromPNG(info->textures[0], true)) return;

    texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
        (float)image.width,
        (float)image.height
    });
    texture->m_bHasPremultipliedAlpha = true;

    GEODE_UNWRAP_OR_ELSE(frames, err, createFrames(info->sheetName, texture, info->name, info->type, true)) return;

    info->frameNames.clear();
    auto spriteFrameCache = CCSpriteFrameCache::get();
    for (auto [frameName, frame] : CCDictionaryExt<std::string, CCSpriteFrame*>(frames)) {
        if (auto spriteFrame = getFrameByName(frameName)) {
            spriteFrame->m_obOffset = frame->m_obOffset;
            spriteFrame->m_obOriginalSize = frame->m_obOriginalSize;
            spriteFrame->m_obRectInPixels = frame->m_obRectInPixels;
            spriteFrame->m_bRotated = frame->m_bRotated;
            spriteFrame->m_obRect = frame->m_obRect;
            spriteFrame->m_obOffsetInPixels = frame->m_obOffsetInPixels;
            spriteFrame->m_obOriginalSizeInPixels = frame->m_obOriginalSizeInPixels;
        }
        else spriteFrameCache->m_pSpriteFrames->setObject(frame, frameName);
        info->frameNames.push_back(frameName);
    }
}

void MoreIconsAPI::updateSimplePlayer(SimplePlayer* player, IconType type, bool dual, bool load) {
    updateSimplePlayer(player, activeIcon(type, dual), type, load);
}

void MoreIconsAPI::updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type, bool load) {
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
        updateRobotSprite(player->m_robotSprite, icon, type, load);
    }
    else if (player->m_robotSprite) player->m_robotSprite->setVisible(false);

    if (type == IconType::Spider) {
        if (!player->m_spiderSprite) player->createSpiderSprite(1);
        player->m_spiderSprite->setVisible(true);
        player->m_spiderSprite->m_color = player->m_firstLayer->getColor();
        player->m_spiderSprite->m_secondColor = player->m_secondLayer->getColor();
        player->m_spiderSprite->updateColors();
        updateRobotSprite(player->m_spiderSprite, icon, type, load);
    }
    else if (player->m_spiderSprite) player->m_spiderSprite->setVisible(false);

    if (type == IconType::Robot || type == IconType::Spider) return;

    if (load) loadIcon(icon, type, player->m_iconRequestID);

    player->m_firstLayer->setDisplayFrame(getFrame("{}_001.png"_spr, icon));
    player->m_firstLayer->setScale(type == IconType::Ball ? 0.9f : 1.0f);
    player->m_firstLayer->setPosition({ 0.0f, type == IconType::Ufo ? -7.0f : 0.0f });
    player->m_secondLayer->setDisplayFrame(getFrame("{}_2_001.png"_spr, icon));
    auto firstCenter = player->m_firstLayer->getContentSize() / 2.0f;
    player->m_secondLayer->setPosition(firstCenter);
    player->m_outlineSprite->setDisplayFrame(getFrame("{}_glow_001.png"_spr, icon));
    player->m_outlineSprite->setPosition(firstCenter);
    if (type == IconType::Ufo) {
        player->m_birdDome->setDisplayFrame(getFrame("{}_3_001.png"_spr, icon));
        player->m_birdDome->setPosition(firstCenter);
    }
    auto extraFrame = getFrame("{}_extra_001.png"_spr, icon);
    player->m_detailSprite->setVisible(extraFrame != nullptr);
    if (extraFrame) {
        player->m_detailSprite->setDisplayFrame(extraFrame);
        player->m_detailSprite->setPosition(firstCenter);
    }
}

void MoreIconsAPI::updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type, bool load) {
    if (!sprite) return;

    auto info = getIcon(icon, type);
    if (!info) return;

    sprite->setUserObject("name"_spr, CCString::create(icon));

    auto texture = load ? loadIcon(icon, type, sprite->m_iconRequestID) : CCTextureCache::get()->textureForKey(info->textures[0].c_str());

    sprite->setBatchNode(nullptr);
    sprite->setTexture(texture);
    sprite->m_paSprite->setBatchNode(nullptr);
    sprite->m_paSprite->setTexture(texture);

    auto spriteParts = sprite->m_paSprite->m_spriteParts;
    for (int i = 0; i < spriteParts->count(); i++) {
        auto spritePart = static_cast<CCSprite*>(spriteParts->objectAtIndex(i));
        auto tag = spritePart->getTag();

        spritePart->setBatchNode(nullptr);
        spritePart->setDisplayFrame(getFrame("{}_{:02}_001.png"_spr, icon, tag));
        if (auto secondSprite = static_cast<CCSprite*>(sprite->m_secondArray->objectAtIndex(i))) {
            secondSprite->setBatchNode(nullptr);
            secondSprite->setDisplayFrame(getFrame("{}_{:02}_2_001.png"_spr, icon, tag));
            secondSprite->setPosition(spritePart->getContentSize() / 2.0f);
        }

        if (auto glowChild = getChild<CCSprite>(sprite->m_glowSprite, i)) {
            glowChild->setBatchNode(nullptr);
            glowChild->setDisplayFrame(getFrame("{}_{:02}_glow_001.png"_spr, icon, tag));
        }

        if (spritePart == sprite->m_headSprite) {
            auto extraFrame = getFrame("{}_{:02}_extra_001.png"_spr, icon, tag);
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

void MoreIconsAPI::updatePlayerObject(PlayerObject* object, IconType type, bool dual) {
    updatePlayerObject(object, activeIcon(type, dual), type);
}

void MoreIconsAPI::updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type) {
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

    firstLayer->setDisplayFrame(getFrame("{}_001.png"_spr, icon));
    secondLayer->setDisplayFrame(getFrame("{}_2_001.png"_spr, icon));
    auto firstCenter = firstLayer->getContentSize() / 2.0f;
    secondLayer->setPosition(firstCenter);
    if (type == IconType::Ufo) {
        object->m_birdVehicle->setDisplayFrame(getFrame("{}_3_001.png"_spr, icon));
        object->m_birdVehicle->setPosition(firstCenter);
    }
    outlineSprite->setDisplayFrame(getFrame("{}_glow_001.png"_spr, icon));
    auto extraFrame = getFrame("{}_extra_001.png"_spr, icon);
    detailSprite->setVisible(extraFrame != nullptr);
    if (extraFrame) {
        detailSprite->setDisplayFrame(extraFrame);
        detailSprite->setPosition(firstCenter);
    }
}

#ifdef GEODE_IS_ANDROID
Result<std::vector<uint8_t>> getFileData(const std::string& path) {
    static thread_local ZipFile* apkFile = new ZipFile(getApkPath());
    if (path.starts_with("assets/")) {
        auto size = 0ul;
        if (auto data = apkFile->getFileData(path.c_str(), &size)) {
            std::vector vec(data, data + size);
            delete[] data;
            return Ok(vec);
        }
        else return Err("Failed to read file from APK");
    }
    return file::readBinary(path);
}
#else
#define getFileData file::readBinary
#endif

Result<ImageResult> MoreIconsAPI::createFrames(
    const std::string& png, const std::string& plist, const std::string& name, IconType type, IconInfo* info
) {
    GEODE_UNWRAP_INTO(auto data, getFileData(png).mapErr([](const std::string& err) {
        return fmt::format("Failed to read image: {}", err);
    }));

    GEODE_UNWRAP_INTO(auto image, texpack::fromPNG(data, true).mapErr([](const std::string& err) {
        return fmt::format("Failed to parse image: {}", err);
    }));

    auto texture = createRef<CCTexture2D>();
    GEODE_UNWRAP_INTO(auto frames, createFrames(plist, texture, name, type, !name.empty()).mapErr([](const std::string& err) {
        return fmt::format("Failed to load frames: {}", err);
    }));

    return Ok<ImageResult>({ png, std::move(image.data), std::move(texture), std::move(frames), info, image.width, image.height });
}

matjson::Value parseNode(const pugi::xml_node& node) {
    std::string_view name = node.name();
    if (name == "dict") {
        auto json = matjson::Value::object();
        for (auto child = node.child("key"); child; child = child.next_sibling("key")) {
            json[child.text().as_string()] = parseNode(child.next_sibling());
        }
        return json;
    }
    else if (name == "array") {
        auto json = matjson::Value::array();
        for (auto child = node.first_child(); child; child = child.next_sibling()) {
            json.push(parseNode(child));
        }
        return json;
    }
    else if (name == "string" || name == "data" || name == "date") return node.text().as_string();
    else if (name == "real") return node.text().as_double();
    else if (name == "integer") return node.text().as_llong();
    else if (name == "true") return true;
    else if (name == "false") return false;
    else return nullptr;
}

Result<Ref<CCDictionary>> MoreIconsAPI::createFrames(
    const std::string& path, CCTexture2D* texture, const std::string& name, IconType type, bool fixNames
) {
    if (path.empty()) return Ok(nullptr);

    GEODE_UNWRAP_INTO(auto data, getFileData(path).mapErr([](const std::string& err) {
        return fmt::format("Failed to read file: {}", err);
    }));

    pugi::xml_document doc;
    auto result = doc.load_buffer(data.data(), data.size());
    if (!result) return Err("Failed to parse XML: {}", result.description());

    auto root = doc.child("plist");
    if (!root) return Err("No root <plist> element found");

    auto json = parseNode(root.first_child());
    if (!json.isObject()) return Err("No root <dict> element found");

    GEODE_UNWRAP_INTO(auto jsonFrames, json.get("frames").mapErr([] { return "No frames <dict> element found"; }));

    auto format = json.get("metadata").andThen([](const matjson::Value& v) {
        return v.get("format").andThen([](const matjson::Value& v) { return v.asInt(); });
    }).unwrapOr(0);

    auto frames = createRef<CCDictionary>();
    for (auto& [frameName, obj] : jsonFrames) {
        if (!obj.isObject()) continue;

        auto frame = createRef<CCSpriteFrame>();
        frames->setObject(frame, fixNames ? getFrameName(frameName, name, type) : frameName);

        CCRect rect;
        CCPoint offset;
        CCSize originalSize;
        auto rotated = false;

        switch (format) {
            case 0: {
                GEODE_UNWRAP_INTO_IF_OK(rect.origin.x, obj.get("x").andThen([](const matjson::Value& v) {
                    return v.asDouble();
                }));
                GEODE_UNWRAP_INTO_IF_OK(rect.origin.y, obj.get("y").andThen([](const matjson::Value& v) {
                    return v.asDouble();
                }));
                GEODE_UNWRAP_INTO_IF_OK(rect.size.width, obj.get("width").andThen([](const matjson::Value& v) {
                    return v.asDouble();
                }));
                GEODE_UNWRAP_INTO_IF_OK(rect.size.height, obj.get("height").andThen([](const matjson::Value& v) {
                    return v.asDouble();
                }));
                GEODE_UNWRAP_INTO_IF_OK(offset.x, obj.get("offsetX").andThen([](const matjson::Value& v) {
                    return v.asDouble();
                }));
                GEODE_UNWRAP_INTO_IF_OK(offset.y, obj.get("offsetY").andThen([](const matjson::Value& v) {
                    return v.asDouble();
                }));
                GEODE_UNWRAP_INTO_IF_OK(originalSize.width, obj.get("originalWidth").andThen([](const matjson::Value& v) {
                    return v.asInt().map([](intmax_t v) -> float { return abs(v); });
                }));
                GEODE_UNWRAP_INTO_IF_OK(originalSize.height, obj.get("originalHeight").andThen([](const matjson::Value& v) {
                    return v.asInt().map([](intmax_t v) -> float { return abs(v); });
                }));
                break;
            }
            case 1: case 2: case 3: {
                if (format == 3) {
                    GEODE_UNWRAP_INTO_IF_OK(rect.origin, obj.get("textureRect").andThen([](const matjson::Value& v) {
                        return v.asString().map([](const std::string& s) { return CCRectFromString(s.c_str()).origin; });
                    }));
                    GEODE_UNWRAP_INTO_IF_OK(rect.size, obj.get("spriteSize").andThen([](const matjson::Value& v) {
                        return v.asString().map([](const std::string& s) { return CCSizeFromString(s.c_str()); });
                    }));
                }
                else {
                    GEODE_UNWRAP_INTO_IF_OK(rect, obj.get("frame").andThen([](const matjson::Value& v) {
                        return v.asString().map([](const std::string& s) { return CCRectFromString(s.c_str()); });
                    }));
                }
                GEODE_UNWRAP_INTO_IF_OK(offset, obj.get(format == 3 ? "spriteOffset" : "offset").andThen([](const matjson::Value& v) {
                    return v.asString().map([](const std::string& s) { return CCPointFromString(s.c_str()); });
                }));
                GEODE_UNWRAP_INTO_IF_OK(originalSize, obj.get(format == 3 ? "spriteSourceSize" : "sourceSize").andThen([](const matjson::Value& v) {
                    return v.asString().map([](const std::string& s) { return CCSizeFromString(s.c_str()); });
                }));
                if (format > 1) {
                    GEODE_UNWRAP_INTO_IF_OK(rotated, obj.get(format == 3 ? "textureRotated" : "rotated").andThen([](const matjson::Value& v) {
                        return v.asBool();
                    }));
                }
                break;
            }
        }

        frame->initWithTexture(texture, rect, rotated, offset, originalSize);
    }

    return Ok(frames);
}

std::vector<std::string> MoreIconsAPI::addFrames(const ImageResult& image) {
    auto& [name, data, texture, frames, info, width, height] = image;

    if (texture) {
        texture->initWithData(data.data(), kCCTexture2DPixelFormat_RGBA8888, width, height, { (float)width, (float)height });
        texture->m_bHasPremultipliedAlpha = true;
        CCTextureCache::get()->m_pTextures->setObject(texture, name);
    }

    std::vector<std::string> frameNames;
    frameNames.reserve(frames ? frames->count() : 0);
    if (frames) {
        auto spriteFrameCache = CCSpriteFrameCache::get();
        for (auto [frameName, frame] : CCDictionaryExt<std::string, CCSpriteFrame*>(frames)) {
            spriteFrameCache->m_pSpriteFrames->setObject(frame, frameName);
            frameNames.push_back(frameName);
        }
    }
    if (info) info->frameNames = frameNames;
    return frameNames;
}

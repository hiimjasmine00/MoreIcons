#include <pugixml.hpp>
#include "MoreIconsAPI.hpp"
#include "../classes/misc/ThreadPool.hpp"
#include <Geode/binding/CCAnimateFrameCache.hpp>
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/ObjectManager.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#ifdef GEODE_IS_ANDROID
#include <Geode/cocos/platform/android/jni/Java_org_cocos2dx_lib_Cocos2dxHelper.h>
#endif
#include <MoreIcons.hpp>
#include <ranges>
#include <texpack.hpp>

using namespace geode::prelude;

std::map<IconType, std::vector<IconInfo>> MoreIconsAPI::icons = {
    { IconType::Cube, {} },
    { IconType::Ship, {} },
    { IconType::Ball, {} },
    { IconType::Ufo, {} },
    { IconType::Wave, {} },
    { IconType::Robot, {} },
    { IconType::Spider, {} },
    { IconType::Swing, {} },
    { IconType::Jetpack, {} },
    { IconType::Special, {} }
};
std::map<int, std::map<IconType, std::string>> MoreIconsAPI::requestedIcons;
std::map<std::pair<std::string, IconType>, int> MoreIconsAPI::loadedIcons;
bool MoreIconsAPI::preloadIcons = false;

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
        for (auto& icons : std::views::values(MoreIconsAPI::icons)) {
            auto size = icons.size();
            vec->reserve(size);
            for (int i = 0; i < size; i++) vec->push_back(icons.data() + i);
        }
        return ListenerResult::Propagate;
    }, MoreIcons::AllIconsFilter("all-icons"_spr));

    new EventListener(+[](std::vector<IconInfo*>* vec, IconType type) {
        vec->clear();
        auto& icons = MoreIconsAPI::icons[type];
        auto size = icons.size();
        vec->reserve(size);
        for (int i = 0; i < size; i++) vec->push_back(icons.data() + i);
        return ListenerResult::Propagate;
    }, MoreIcons::GetIconsFilter("get-icons"_spr));

    new EventListener(+[](IconInfo** info, const std::string& name, IconType type) {
        *info = MoreIconsAPI::getIcon(name, type);
        return ListenerResult::Propagate;
    }, MoreIcons::GetIconFilter("get-icon"_spr));

    new EventListener(+[](const std::string& icon, IconType type, int requestID) {
        MoreIconsAPI::loadIcon(icon, type, requestID);
        return ListenerResult::Propagate;
    }, MoreIcons::LoadIconFilter("load-icon"_spr));

    new EventListener(+[](const std::string& icon, IconType type, int requestID) {
        MoreIconsAPI::unloadIcon(icon, type, requestID);
        return ListenerResult::Propagate;
    }, MoreIcons::UnloadIconFilter("unload-icon"_spr));

    new EventListener(+[](int requestID) {
        MoreIconsAPI::unloadIcons(requestID);
        return ListenerResult::Propagate;
    }, MoreIcons::UnloadIconsFilter("unload-icons"_spr));
}

IconInfo* MoreIconsAPI::getIcon(const std::string& name, IconType type) {
    if (name.empty()) return nullptr;
    auto& iconsVec = icons[type];
    auto found = std::ranges::find_if(iconsVec, [&name](const IconInfo& info) {
        return info.name == name;
    });
    return found < iconsVec.end() ? std::to_address(found) : nullptr;
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
    if (icon.empty()) return false;
    return std::ranges::any_of(icons[type], [&icon](const IconInfo& info) {
        return info.name == icon;
    });
}

bool MoreIconsAPI::hasIcon(IconType type, bool dual) {
    return hasIcon(activeIcon(type, dual), type);
}

CCAnimateFrameCache* globalAnimateFrameCache = nullptr;
CCAnimateFrameCache* MoreIconsAPI::getAnimateFrameCache() {
    if (!globalAnimateFrameCache) globalAnimateFrameCache = CCAnimateFrameCache::sharedSpriteFrameCache();
    return globalAnimateFrameCache;
}

CCAnimationCache* globalAnimationCache = nullptr;
CCAnimationCache* MoreIconsAPI::getAnimationCache() {
    if (!globalAnimationCache) globalAnimationCache = CCAnimationCache::sharedAnimationCache();
    return globalAnimationCache;
}

CCDirector* globalDirector = nullptr;
CCDirector* MoreIconsAPI::getDirector() {
    if (!globalDirector) globalDirector = CCDirector::sharedDirector();
    return globalDirector;
}

CCFileUtils* globalFileUtils = nullptr;
CCFileUtils* MoreIconsAPI::getFileUtils() {
    if (!globalFileUtils) globalFileUtils = CCFileUtils::sharedFileUtils();
    return globalFileUtils;
}

CCShaderCache* globalShaderCache = nullptr;
CCShaderCache* MoreIconsAPI::getShaderCache() {
    if (!globalShaderCache) globalShaderCache = CCShaderCache::sharedShaderCache();
    return globalShaderCache;
}

CCSpriteFrameCache* globalSpriteFrameCache = nullptr;
CCSpriteFrameCache* MoreIconsAPI::getSpriteFrameCache() {
    if (!globalSpriteFrameCache) globalSpriteFrameCache = CCSpriteFrameCache::sharedSpriteFrameCache();
    return globalSpriteFrameCache;
}

CCTextureCache* globalTextureCache = nullptr;
CCTextureCache* MoreIconsAPI::getTextureCache() {
    if (!globalTextureCache) globalTextureCache = CCTextureCache::sharedTextureCache();
    return globalTextureCache;
}

GameManager* globalGameManager = nullptr;
GameManager* MoreIconsAPI::getGameManager() {
    if (!globalGameManager) globalGameManager = GameManager::sharedState();
    return globalGameManager;
}

ObjectManager* globalObjectManager = nullptr;
ObjectManager* MoreIconsAPI::getObjectManager() {
    if (!globalObjectManager) globalObjectManager = ObjectManager::instance();
    return globalObjectManager;
}

void MoreIconsAPI::reset() {
    globalFileUtils = nullptr;
    globalSpriteFrameCache = nullptr;
    globalTextureCache = nullptr;
    globalObjectManager = nullptr;

    for (auto& iconsVec : std::views::values(icons)) {
        iconsVec.clear();
    }
    requestedIcons.clear();
    loadedIcons.clear();
}

CCSpriteFrame* MoreIconsAPI::getFrame(std::string_view name) {
    auto spriteFrame = static_cast<CCSpriteFrame*>(getSpriteFrameCache()->m_pSpriteFrames->objectForKey({ name.data(), name.size() }));
    if (!spriteFrame || spriteFrame->getTag() == 105871529) spriteFrame = nullptr;
    return spriteFrame;
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

    auto& png = info->textures[0];
    auto texture = getTextureCache()->textureForKey(png.c_str());
    if (preloadIcons) return texture;

    auto& loadedIcon = loadedIcons[{ name, type }];

    if (loadedIcon < 1) {
        if (auto res = createFrames(info->textures[0], info->sheetName, info->name, info->type)) {
            addFrames(res.unwrap(), info->frameNames);
        }
        else {
            log::error("{}: {}", info->name, res.unwrapErr());
        }
    }

    auto& requestedIcon = requestedIcons[requestID][type];
    if (requestedIcon != name) {
        loadedIcon++;
        if (!requestedIcon.empty()) unloadIcon(requestedIcon, type, requestID);
        requestedIcons[requestID][type] = name;
    }

    return texture;
}

void MoreIconsAPI::loadIcons(IconType type) {
    if (!preloadIcons) return;

    auto name = lowercase[convertType(type)];

    auto& iconsVec = icons[type];
    auto size = iconsVec.size();
    log::info("Pre-loading {} {} textures", size, name);

    std::vector<std::pair<ImageResult, IconInfo*>> images;
    images.reserve(size);
    std::mutex imageMutex;

    auto& threadPool = ThreadPool::get();
    for (int i = 0; i < size; i++) {
        threadPool.pushTask([info = iconsVec.data() + i, &images, &imageMutex] {
            if (auto res = createFrames(info->textures[0], info->sheetName, info->name, info->type)) {
                std::unique_lock lock(imageMutex);

                images.emplace_back(std::move(res).unwrap(), info);
            }
            else log::error("{}: {}", info->name, res.unwrapErr());
        });
    }
    threadPool.wait();

    std::unique_lock lock(imageMutex);

    auto loaded = 0;
    for (; !images.empty(); loaded++) {
        auto& [image, info] = images.front();
        addFrames(image, info->frameNames);
        images.erase(images.begin());
    }

    log::info("Finished pre-loading {} {} textures, {} remaining", loaded, name, size - loaded);
}

void MoreIconsAPI::unloadIcon(const std::string& name, IconType type, int requestID) {
    if (preloadIcons || name.empty()) return;

    auto info = getIcon(name, type);
    if (!info) return;

    auto& loadedIcon = loadedIcons[{ name, type }];

    loadedIcon--;
    if (loadedIcon < 1) {
        auto spriteFrameCache = getSpriteFrameCache();
        for (auto& frame : info->frameNames) {
            spriteFrameCache->removeSpriteFrameByName(frame.c_str());
        }
        info->frameNames.clear();

        getTextureCache()->removeTextureForKey(info->textures[0].c_str());
    }

    requestedIcons[requestID].erase(type);
    if (requestedIcons[requestID].empty()) requestedIcons.erase(requestID);
}

void MoreIconsAPI::unloadIcons(int requestID) {
    if (preloadIcons) return;

    auto foundRequests = requestedIcons.find(requestID);
    if (foundRequests == requestedIcons.end()) return;

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

    return *suffix ? prefix.empty() ? suffix : GEODE_MOD_ID "/" + prefix + suffix : name;
}

IconInfo* MoreIconsAPI::addIcon(
    const std::string& name, const std::string& shortName, IconType type, const std::string& png, const std::string& plist,
    const std::string& packID, const std::string& packName, int trailID, const TrailInfo& trailInfo, bool vanilla, bool zipped
) {
    auto& iconsVec = icons[type];
    auto it = std::ranges::find_if(iconsVec, [&packID, &shortName, type](const IconInfo& icon) {
        return icon.compare(packID, shortName, type) >= 0;
    });
    if (it != iconsVec.end() && it->type == type && it->name == name) iconsVec.erase(it);
    return std::to_address(iconsVec.emplace(
        it,
        name,
        std::vector<std::string>({ png }),
        std::vector<std::string>(),
        plist,
        packName,
        packID,
        type,
        trailID,
        trailInfo,
        shortName,
        vanilla,
        zipped
    ));
}

void MoreIconsAPI::moveIcon(IconInfo* info, const std::filesystem::path& path) {
    auto oldPng = info->textures[0];
    auto newPng = string::pathToString(path / std::filesystem::path(oldPng).filename());
    info->textures[0] = newPng;
    info->sheetName = string::pathToString(path / std::filesystem::path(info->sheetName).filename());
    info->vanilla = false;

    auto textureCache = getTextureCache();
    if (Ref texture = textureCache->textureForKey(oldPng.c_str())) {
        textureCache->removeTextureForKey(oldPng.c_str());
        textureCache->m_pTextures->setObject(texture, newPng);
    }
}

void MoreIconsAPI::removeIcon(IconInfo* info) {
    auto& name = info->name;
    auto type = info->type;
    loadedIcons[{ name, type }] = 0;

    auto spriteFrameCache = getSpriteFrameCache();
    for (auto& frame : info->frameNames) {
        spriteFrameCache->removeSpriteFrameByName(frame.c_str());
    }
    getTextureCache()->removeTextureForKey(info->textures[0].c_str());

    if (!preloadIcons) {
        for (auto it = requestedIcons.begin(); it != requestedIcons.end();) {
            auto& iconRequests = it->second;
            auto iconRequest = iconRequests.find(type);
            if (iconRequest != iconRequests.end() && iconRequest->second == name) {
                iconRequests.erase(iconRequest);
                if (iconRequests.empty()) it = requestedIcons.erase(it);
                else ++it;
            }
        }
    }

    auto& iconsVec = icons[type];
    iconsVec.erase(iconsVec.begin() + (info - iconsVec.data()));
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

    auto textureCache = getTextureCache();
    if (Ref texture = textureCache->textureForKey(oldPng.c_str())) {
        textureCache->removeTextureForKey(oldPng.c_str());
        textureCache->m_pTextures->setObject(texture, newPng);

        auto spriteFrameCache = getSpriteFrameCache();
        for (auto& frameName : info->frameNames) {
            if (Ref spriteFrame = getFrame(frameName)) {
                spriteFrameCache->removeSpriteFrameByName(frameName.c_str());
                frameName = getFrameName(frameName, newName, type);
                spriteFrameCache->addSpriteFrame(spriteFrame, frameName.c_str());
            }
        }
    }

    if (auto it = loadedIcons.find({ oldName, type }); it != loadedIcons.end()) {
        loadedIcons.emplace(std::make_pair(newName, type), it->second);
        loadedIcons.erase(it);
    }

    for (auto& iconRequests : std::views::values(requestedIcons)) {
        if (auto found = iconRequests.find(type); found != iconRequests.end() && found->second == oldName) {
            found->second = newName;
        }
    }

    if (activeIcon(type, false) == oldName) setIcon(newName, type, false);
    if (activeIcon(type, true) == oldName) setIcon(newName, type, true);

    auto& iconsVec = icons[type];
    auto it = std::ranges::find_if(iconsVec, [info](const IconInfo& icon) {
        return icon.compare(info->packID, info->shortName, info->type) >= 0;
    });
    if (std::to_address(it) == info) return;

    auto icon = std::move(*info);
    iconsVec.erase(iconsVec.begin() + (info - iconsVec.data()));
    iconsVec.insert(it, std::move(icon));
}

void MoreIconsAPI::updateIcon(IconInfo* info) {
    auto texture = getTextureCache()->textureForKey(info->textures[0].c_str());
    if (!texture) return;

    auto imageRes = texpack::fromPNG(info->textures[0], true);
    if (!imageRes.isOk()) return;

    auto image = std::move(imageRes).unwrap();

    texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
        (float)image.width,
        (float)image.height
    });
    texture->m_bHasPremultipliedAlpha = true;

    auto framesRes = createFrames(info->sheetName, texture, info->name, info->type, true);
    if (!framesRes.isOk()) return;

    auto frames = std::move(framesRes).unwrap();
    if (!frames) return;

    auto spriteFrameCache = getSpriteFrameCache();
    for (auto& frameName : info->frameNames) {
        if (!frames->objectForKey(frameName)) spriteFrameCache->removeSpriteFrameByName(frameName.c_str());
    }

    info->frameNames.clear();
    for (auto [frameName, frame] : CCDictionaryExt<std::string, CCSpriteFrame*>(frames)) {
        if (auto spriteFrame = getFrame(frameName)) {
            spriteFrame->m_obOffset = frame->m_obOffset;
            spriteFrame->m_obOriginalSize = frame->m_obOriginalSize;
            spriteFrame->m_obRectInPixels = frame->m_obRectInPixels;
            spriteFrame->m_bRotated = frame->m_bRotated;
            spriteFrame->m_obRect = frame->m_obRect;
            spriteFrame->m_obOffsetInPixels = frame->m_obOffsetInPixels;
            spriteFrame->m_obOriginalSizeInPixels = frame->m_obOriginalSizeInPixels;
        }
        else spriteFrameCache->addSpriteFrame(frame, frameName.c_str());
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

    player->m_firstLayer->setDisplayFrame(getFrame(fmt::format("{}_001.png"_spr, icon)));
    player->m_firstLayer->setScale(type == IconType::Ball ? 0.9f : 1.0f);
    player->m_firstLayer->setPosition({ 0.0f, type == IconType::Ufo ? -7.0f : 0.0f });
    player->m_secondLayer->setDisplayFrame(getFrame(fmt::format("{}_2_001.png"_spr, icon)));
    auto firstCenter = player->m_firstLayer->getContentSize() / 2.0f;
    player->m_secondLayer->setPosition(firstCenter);
    player->m_outlineSprite->setDisplayFrame(getFrame(fmt::format("{}_glow_001.png"_spr, icon)));
    player->m_outlineSprite->setPosition(firstCenter);
    if (type == IconType::Ufo) {
        player->m_birdDome->setDisplayFrame(getFrame(fmt::format("{}_3_001.png"_spr, icon)));
        player->m_birdDome->setPosition(firstCenter);
    }
    auto extraFrame = getFrame(fmt::format("{}_extra_001.png"_spr, icon));
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

    auto texture = load ? loadIcon(icon, type, sprite->m_iconRequestID) : getTextureCache()->textureForKey(info->textures[0].c_str());

    sprite->setBatchNode(nullptr);
    sprite->setTexture(texture);
    sprite->m_paSprite->setBatchNode(nullptr);
    sprite->m_paSprite->setTexture(texture);

    auto spriteParts = sprite->m_paSprite->m_spriteParts;
    for (int i = 0; i < spriteParts->count(); i++) {
        auto spritePart = static_cast<CCSprite*>(spriteParts->objectAtIndex(i));
        auto tag = spritePart->getTag();

        spritePart->setBatchNode(nullptr);
        spritePart->setDisplayFrame(getFrame(fmt::format("{}_{:02}_001.png"_spr, icon, tag)));
        if (auto secondSprite = static_cast<CCSprite*>(sprite->m_secondArray->objectAtIndex(i))) {
            secondSprite->setBatchNode(nullptr);
            secondSprite->setDisplayFrame(getFrame(fmt::format("{}_{:02}_2_001.png"_spr, icon, tag)));
            secondSprite->setPosition(spritePart->getContentSize() / 2.0f);
        }

        if (auto glowChild = sprite->m_glowSprite->getChildByIndex<CCSprite>(i)) {
            glowChild->setBatchNode(nullptr);
            glowChild->setDisplayFrame(getFrame(fmt::format("{}_{:02}_glow_001.png"_spr, icon, tag)));
        }

        if (spritePart == sprite->m_headSprite) {
            auto extraFrame = getFrame(fmt::format("{}_{:02}_extra_001.png"_spr, icon, tag));
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

    firstLayer->setDisplayFrame(getFrame(fmt::format("{}_001.png"_spr, icon)));
    secondLayer->setDisplayFrame(getFrame(fmt::format("{}_2_001.png"_spr, icon)));
    auto firstCenter = firstLayer->getContentSize() / 2.0f;
    secondLayer->setPosition(firstCenter);
    if (type == IconType::Ufo) {
        object->m_birdVehicle->setDisplayFrame(getFrame(fmt::format("{}_3_001.png"_spr, icon)));
        object->m_birdVehicle->setPosition(firstCenter);
    }
    outlineSprite->setDisplayFrame(getFrame(fmt::format("{}_glow_001.png"_spr, icon)));
    auto extraFrame = getFrame(fmt::format("{}_extra_001.png"_spr, icon));
    detailSprite->setVisible(extraFrame != nullptr);
    if (extraFrame) {
        detailSprite->setDisplayFrame(extraFrame);
        detailSprite->setPosition(firstCenter);
    }
}

#ifdef GEODE_IS_ANDROID
Result<std::vector<uint8_t>> readBinary(const std::string& path) {
    static thread_local ZipFile* apkFile = new ZipFile(getApkPath());
    if (path.starts_with("assets/")) {
        auto size = 0ul;
        if (auto data = apkFile->getFileData(path.c_str(), &size)) {
            std::vector<uint8_t> vec(data, data + size);
            delete[] data;
            return Ok(vec);
        }
        else return Err("Failed to read file from APK");
    }
    return file::readBinary(path);
}
#else
using file::readBinary;
#endif

Result<ImageResult> MoreIconsAPI::createFrames(const std::string& png, const std::string& plist, const std::string& name, IconType type) {
    GEODE_UNWRAP_INTO(auto data, readBinary(png).mapErr([](const std::string& err) {
        return fmt::format("Failed to read image: {}", err);
    }));

    GEODE_UNWRAP_INTO(auto image, texpack::fromPNG(data, true).mapErr([](const std::string& err) {
        return fmt::format("Failed to parse image: {}", err);
    }));

    Autorelease texture = new CCTexture2D();
    GEODE_UNWRAP_INTO(auto frames, createFrames(plist, texture, name, type, !name.empty()).mapErr([](const std::string& err) {
        return fmt::format("Failed to load frames: {}", err);
    }));

    ImageResult result;
    result.name = png;
    result.data = std::move(image.data);
    result.texture = std::move(texture);
    result.frames = std::move(frames);
    result.width = image.width;
    result.height = image.height;
    return Ok(std::move(result));
}

matjson::Value parseNode(const pugi::xml_node& node) {
    std::string_view name = node.name();
    if (name == "dict") {
        auto json = matjson::Value::object();
        for (auto child = node.child("key"); !child.empty(); child = child.next_sibling("key")) {
            json[child.text().as_string()] = parseNode(child.next_sibling());
        }
        return json;
    }
    else if (name == "array") {
        auto json = matjson::Value::array();
        for (auto child = node.first_child(); !child.empty(); child = child.next_sibling()) {
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

Result<Autorelease<CCDictionary>> MoreIconsAPI::createFrames(
    const std::string& path, CCTexture2D* texture, const std::string& name, IconType type, bool fixNames
) {
    if (path.empty()) return Ok(nullptr);

    GEODE_UNWRAP_INTO(auto data, readBinary(path).mapErr([](const std::string& err) {
        return fmt::format("Failed to read file: {}", err);
    }));

    pugi::xml_document doc;
    auto result = doc.load_buffer(data.data(), data.size());
    if (!result) return Err("Failed to parse XML: {}", result.description());

    auto root = doc.child("plist");
    if (!root) return Err("No root <plist> element found");

    auto json = parseNode(root.first_child());
    if (!json.isObject()) return Err("No root <dict> element found");
    if (!json.contains("frames")) return Err("No frames <dict> element found");

    auto format = json.get("metadata").andThen([](const matjson::Value& v) {
        return v.get("format").andThen([](const matjson::Value& v) {
            return v.asInt();
        });
    }).unwrapOr(0);

    Autorelease frames = new CCDictionary();
    for (auto& [frameName, obj] : json["frames"]) {
        if (!obj.isObject()) continue;

        Autorelease frame = new CCSpriteFrame();
        frames->setObject(frame, fixNames ? getFrameName(frameName, name, type) : frameName);

        CCRect rect;
        CCPoint offset;
        CCSize originalSize;
        auto rotated = false;

        switch (format) {
            case 0: {
                if (auto x = obj.get("x").andThen([](const matjson::Value& v) {
                    return v.as<float>();
                }).ok()) rect.origin.x = *x;

                if (auto y = obj.get("y").andThen([](const matjson::Value& v) {
                    return v.as<float>();
                }).ok()) rect.origin.y = *y;

                if (auto w = obj.get("width").andThen([](const matjson::Value& v) {
                    return v.as<float>();
                }).ok()) rect.size.width = *w;

                if (auto h = obj.get("height").andThen([](const matjson::Value& v) {
                    return v.as<float>();
                }).ok()) rect.size.height = *h;

                if (auto offsetX = obj.get("offsetX").andThen([](const matjson::Value& v) {
                    return v.as<float>();
                }).ok()) offset.x = *offsetX;

                if (auto offsetY = obj.get("offsetY").andThen([](const matjson::Value& v) {
                    return v.as<float>();
                }).ok()) offset.y = *offsetY;

                if (auto originalWidth = obj.get("originalWidth").andThen([](const matjson::Value& v) {
                    return v.as<float>().map([](float v) {
                        return abs(floor(v));
                    });
                }).ok()) originalSize.width = *originalWidth;

                if (auto originalHeight = obj.get("originalHeight").andThen([](const matjson::Value& v) {
                    return v.as<float>().map([](float v) {
                        return abs(floor(v));
                    });
                }).ok()) originalSize.height = *originalHeight;
                break;
            }
            case 1: case 2: case 3: {
                if (format == 3) {
                    if (auto textureRect = obj.get("textureRect").andThen([](const matjson::Value& v) {
                        return v.asString().map([](const std::string& s) {
                            return CCRectFromString(s.c_str()).origin;
                        });
                    }).ok()) rect.origin = *textureRect;

                    if (auto spriteSize = obj.get("spriteSize").andThen([](const matjson::Value& v) {
                        return v.asString().map([](const std::string& s) {
                            return CCSizeFromString(s.c_str());
                        });
                    }).ok()) rect.size = *spriteSize;
                }
                else {
                    if (auto textureRect = obj.get("frame").andThen([](const matjson::Value& v) {
                        return v.asString().map([](const std::string& s) {
                            return CCRectFromString(s.c_str());
                        });
                    }).ok()) rect = *textureRect;
                }

                if (auto spriteOffset = obj.get(format == 3 ? "spriteOffset" : "offset").andThen([](const matjson::Value& v) {
                    return v.asString().map([](const std::string& s) {
                        return CCPointFromString(s.c_str());
                    });
                }).ok()) offset = *spriteOffset;

                if (auto spriteSourceSize = obj.get(format == 3 ? "spriteSourceSize" : "sourceSize").andThen([](const matjson::Value& v) {
                    return v.asString().map([](const std::string& s) {
                        return CCSizeFromString(s.c_str());
                    });
                }).ok()) originalSize = *spriteSourceSize;

                if (format > 1) {
                    if (auto textureRotated = obj.get(format == 3 ? "textureRotated" : "rotated").andThen([](const matjson::Value& v) {
                        return v.asBool();
                    }).ok()) rotated = *textureRotated;
                }
                break;
            }
        }

        frame->initWithTexture(texture, rect, rotated, offset, originalSize);
    }

    return Ok(std::move(frames));
}

CCTexture2D* MoreIconsAPI::addFrames(const ImageResult& image, std::vector<std::string>& frameNames) {
    if (auto texture = image.texture.data) {
        texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
            (float)image.width,
            (float)image.height
        });
        texture->m_bHasPremultipliedAlpha = true;
        getTextureCache()->m_pTextures->setObject(texture, image.name);
    }

    frameNames.clear();
    if (auto frames = image.frames.data) {
        frameNames.reserve(frames->count());
        auto spriteFrameCache = getSpriteFrameCache();
        for (auto [frameName, frame] : CCDictionaryExt<std::string, CCSpriteFrame*>(frames)) {
            spriteFrameCache->addSpriteFrame(frame, frameName.c_str());
            frameNames.push_back(frameName);
        }
    }

    return image.texture;
}

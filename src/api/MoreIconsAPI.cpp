#include <pugixml.hpp>
#include "MoreIconsAPI.hpp"
#include "../classes/misc/ThreadPool.hpp"
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
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
    auto found = std::ranges::find_if(icons, [&name, type](const IconInfo& info) { return info.name == name && info.type == type; });
    return found < icons.end() ? std::to_address(found) : nullptr;
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
    return !icon.empty() && std::ranges::any_of(icons, [&icon, type](const IconInfo& info) { return info.name == icon && info.type == type; });
}

int MoreIconsAPI::getCount(IconType type) {
    return std::ranges::count_if(icons, [type](const IconInfo& info) { return info.type == type; });
}

Result<ImageResult> createIconFrames(IconInfo* info) {
    if (!info) return Err("No icon info provided");

    GEODE_UNWRAP_INTO(auto image, MoreIconsAPI::createFrames(info->textures[0], info->sheetName, info->name, info->type));
    image.info = info;
    return Ok<ImageResult>(std::move(image));
}

CCTexture2D* MoreIconsAPI::loadIcon(const std::string& name, IconType type, int requestID) {
    auto info = getIcon(name, type);
    if (!info) return nullptr;

    auto texture = CCTextureCache::get()->textureForKey(info->textures[0].c_str());
    if (preloadIcons) return texture;

    auto& loadedIcon = loadedIcons[{ name, type }];

    if (loadedIcon < 1) {
        auto imageRes = createIconFrames(info);
        if (imageRes.isOk()) {
            auto result = imageRes.unwrap();
            addFrames(result);
            texture = result.texture;
        }
        else log::error("{}: {}", info->name, imageRes.unwrapErr());
    }

    auto& requestedIcon = requestedIcons[requestID][type];
    if (requestedIcon != name) {
        loadedIcon++;
        if (!requestedIcon.empty()) unloadIcon(requestedIcon, type, requestID);
        requestedIcon = name;
    }

    return texture;
}

void MoreIconsAPI::loadIcons(std::vector<IconInfo>& icons, std::string_view name, bool logs) {
    if (!preloadIcons) return;

    auto size = icons.size();
    if (logs) log::info("Pre-loading {} {} textures", size, name);

    std::vector<ImageResult> images;
    std::mutex imageMutex;

    auto& threadPool = ThreadPool::get();
    for (auto info = icons.data(); info < std::to_address(icons.end()); info++) {
        threadPool.pushTask([info, &images, &imageMutex] {
            auto imageRes = createIconFrames(info);
            if (imageRes.isErr()) return log::error("{}: {}", info->name, imageRes.unwrapErr());

            std::unique_lock lock(imageMutex);

            images.push_back(imageRes.unwrap());
        });
    }
    threadPool.wait();

    std::unique_lock lock(imageMutex);

    auto loaded = 0;
    for (auto& image : images) {
        addFrames(image);
        loaded++;
    }

    images.clear();
    if (logs) log::info("Finished pre-loading {} {} textures, {} remaining", loaded, name, size - loaded);
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

    sprite->setUserObject("name"_spr, CCString::create(icon));

    auto texture = loadIcon(icon, type, sprite->m_iconRequestID);

    sprite->setBatchNode(nullptr);
    sprite->setTexture(texture);
    sprite->m_paSprite->setBatchNode(nullptr);
    sprite->m_paSprite->setTexture(texture);

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

        if (auto glowChild = getChild<CCSprite>(sprite->m_glowSprite, i)) {
            glowChild->setBatchNode(nullptr);
            glowChild->setDisplayFrame(sfc->spriteFrameByName(fmt::format("{}_{:02}_glow_001.png"_spr, icon, tag).c_str()));
        }

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
                    spritePart->addChild(sprite->m_extraSprite, 2);
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

    object->setUserObject("name"_spr, CCString::create(icon));

    if (type == IconType::Robot || type == IconType::Spider) {
        auto robotSprite = type == IconType::Spider ? object->m_spiderSprite : object->m_robotSprite;
        auto batchNode = type == IconType::Spider ? object->m_spiderBatchNode : object->m_robotBatchNode;
        auto useBatchNode = batchNode && robotSprite && robotSprite->getParent() == batchNode;

        if (useBatchNode) {
            robotSprite->retain();
            robotSprite->removeFromParentAndCleanup(false);
        }

        updateRobotSprite(robotSprite, icon, type);

        if (useBatchNode) {
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

Result<std::vector<uint8_t>> getFileData(const std::string& path) {
    #ifdef GEODE_IS_ANDROID
    static thread_local ZipFile* apkFile = new ZipFile(getApkPath());
    if (path.starts_with("assets/")) {
        auto size = 0ul;
        if (auto data = apkFile->getFileData(path.c_str(), &size)) return Ok(std::vector(data, data + size));
        else return Err("Failed to read file from APK");
    }
    #endif
    return file::readBinary(path);
}

std::vector<float> floatsFromString(const std::string& str, int count) {
    std::vector<float> result;
    std::string temp;
    for (auto& c : str) {
        if (c == '{' || c == '}') continue;
        else if (c == ',') {
            if (!temp.empty()) {
                result.push_back(numFromString<float>(temp).unwrapOr(0.0f));
                temp.clear();
                if (result.size() >= count) break;
            }
        }
        else temp += c;
    }
    if (!temp.empty()) result.push_back(numFromString<float>(temp).unwrapOr(0.0f));
    if (result.size() < count) result.resize(count, 0.0f);
    return result;
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

Result<ImageResult> MoreIconsAPI::createFrames(const std::string& png, const std::string& plist, const std::string& name, IconType type) {
    GEODE_UNWRAP_INTO(auto data, getFileData(png).mapErr([](const std::string& err) {
        return fmt::format("Failed to read image: {}", err);
    }));

    GEODE_UNWRAP_INTO(auto image, texpack::fromPNG(data, true).mapErr([](const std::string& err) {
        return fmt::format("Failed to parse image: {}", err);
    }));

    CCTexture2D* texture = nullptr;
    CCDictionary* frames = nullptr;

    if (!plist.empty()) {
        GEODE_UNWRAP_INTO(auto pair, createFrames(plist, nullptr, name, type, !name.empty())
            .mapErr([](const std::string& err) { return fmt::format("Failed to load frames: {}", err); }));
        texture = pair.first;
        frames = pair.second;
    }
    else texture = new CCTexture2D();

    return Ok<ImageResult>({ png, std::move(image.data), texture, frames, nullptr, image.width, image.height });
}

Result<std::pair<CCTexture2D*, CCDictionary*>> MoreIconsAPI::createFrames(
    const std::string& path, CCTexture2D* texture, const std::string& name, IconType type, bool fixNames
) {
    GEODE_UNWRAP_INTO(auto data, getFileData(path).mapErr([](const std::string& err) {
        return fmt::format("Failed to read file: {}", err);
    }));

    pugi::xml_document doc;
    auto result = doc.load_buffer(data.data(), data.size());
    if (!result) return Err("Failed to parse XML: {}", result.description());

    auto root = doc.child("plist").child("dict");
    if (!root) return Err("No root <dict> element found");

    pugi::xml_node framesNode;
    pugi::xml_node metadataNode;
    for (auto child = root.child("key"); child; child = child.next_sibling("key")) {
        std::string_view key = child.text().as_string();
        if (key == "frames") framesNode = child.next_sibling();
        else if (key == "metadata") metadataNode = child.next_sibling();
    }

    auto format = 0;
    if (metadataNode.name() == "dict"sv) {
        for (auto child = metadataNode.child("key"); child; child = child.next_sibling("key")) {
            if (child.text().as_string() == "format"sv) {
                format = child.next_sibling().text().as_int();
                break;
            }
        }
    }

    if (framesNode.name() != "dict"sv) return Err("No frames <dict> element found");

    if (!texture) texture = new CCTexture2D();
    auto frames = new CCDictionary();
    auto factor = CCDirector::get()->getContentScaleFactor();
    for (auto child = framesNode.child("key"); child; child = child.next_sibling("key")) {
        auto frameName = child.text().as_string();
        auto frame = new CCSpriteFrame();
        frame->m_bRotated = false;
        frame->m_pobTexture = nullptr;
        frames->setObject(frame, fixNames ? getFrameName(frameName, name, type) : frameName);
        frame->release();

        auto frameNode = child.next_sibling();
        if (frameNode.name() != "dict"sv) continue;

        auto x = 0.0f;
        auto y = 0.0f;
        auto width = 0.0f;
        auto height = 0.0f;
        auto offsetX = 0.0f;
        auto offsetY = 0.0f;
        auto originalWidth = 0.0f;
        auto originalHeight = 0.0f;
        auto rotated = false;

        for (auto next = frameNode.child("key"); next; next = next.next_sibling("key")) {
            std::string_view k = next.text().as_string();
            auto value = next.next_sibling();
            auto val = value.text();
            std::string v = val.as_string();

            switch (format) {
                case 0: {
                    if (k == "x") x = val.as_float();
                    else if (k == "y") y = val.as_float();
                    else if (k == "width") width = val.as_float();
                    else if (k == "height") height = val.as_float();
                    else if (k == "offsetX") offsetX = val.as_float();
                    else if (k == "offsetY") offsetY = val.as_float();
                    else if (k == "originalWidth") originalWidth = abs(val.as_int());
                    else if (k == "originalHeight") originalHeight = abs(val.as_int());
                    break;
                }
                case 1: {
                    if (k == "frame") {
                        auto floats = floatsFromString(v, 4);
                        x = floats[0];
                        y = floats[1];
                        width = floats[2];
                        height = floats[3];
                    }
                    else if (k == "offset") {
                        auto floats = floatsFromString(v, 2);
                        offsetX = floats[0];
                        offsetY = floats[1];
                    }
                    else if (k == "sourceSize") {
                        auto floats = floatsFromString(v, 2);
                        originalWidth = floats[0];
                        originalHeight = floats[1];
                    }
                    break;
                }
                case 2: {
                    if (k == "frame") {
                        auto floats = floatsFromString(v, 4);
                        x = floats[0];
                        y = floats[1];
                        width = floats[2];
                        height = floats[3];
                    }
                    else if (k == "offset") {
                        auto floats = floatsFromString(v, 2);
                        offsetX = floats[0];
                        offsetY = floats[1];
                    }
                    else if (k == "sourceSize") {
                        auto floats = floatsFromString(v, 2);
                        originalWidth = floats[0];
                        originalHeight = floats[1];
                    }
                    else if (k == "rotated") rotated = value.name() == "true"sv;
                    break;
                }
                case 3: {
                    if (k == "textureRect") {
                        auto floats = floatsFromString(v, 2);
                        x = floats[0];
                        y = floats[1];
                    }
                    else if (k == "spriteSize") {
                        auto floats = floatsFromString(v, 2);
                        width = floats[0];
                        height = floats[1];
                    }
                    else if (k == "spriteOffset") {
                        auto floats = floatsFromString(v, 2);
                        offsetX = floats[0];
                        offsetY = floats[1];
                    }
                    else if (k == "spriteSourceSize") {
                        auto floats = floatsFromString(v, 2);
                        originalWidth = floats[0];
                        originalHeight = floats[1];
                    }
                    else if (k == "textureRotated") rotated = value.name() == "true"sv;
                    break;
                }
            }
        }

        frame->m_obOffset.x = offsetX / factor;
        frame->m_obOffset.y = offsetY / factor;
        frame->m_obOriginalSize.width = originalWidth / factor;
        frame->m_obOriginalSize.height = originalHeight / factor;
        frame->m_obRectInPixels.origin.x = x;
        frame->m_obRectInPixels.origin.y = y;
        frame->m_obRectInPixels.size.width = width;
        frame->m_obRectInPixels.size.height = height;
        frame->m_bRotated = rotated;
        frame->m_obRect.origin.x = x / factor;
        frame->m_obRect.origin.y = y / factor;
        frame->m_obRect.size.width = width / factor;
        frame->m_obRect.size.height = height / factor;
        frame->m_obOffsetInPixels.x = offsetX;
        frame->m_obOffsetInPixels.y = offsetY;
        frame->m_obOriginalSizeInPixels.width = originalWidth;
        frame->m_obOriginalSizeInPixels.height = originalHeight;
        frame->m_pobTexture = texture;
        texture->retain();
    }

    return Ok(std::make_pair(texture, frames));
}

std::vector<std::string> MoreIconsAPI::addFrames(const ImageResult& image) {
    auto& [name, data, texture, frames, info, width, height] = image;

    if (texture) {
        texture->initWithData(data.data(), kCCTexture2DPixelFormat_RGBA8888, width, height, { (float)width, (float)height });
        texture->m_bHasPremultipliedAlpha = true;
        CCTextureCache::get()->m_pTextures->setObject(texture, name);
        texture->release();
    }

    std::vector<std::string> frameNames;
    if (frames) {
        auto spriteFrameCache = CCSpriteFrameCache::get();
        for (auto [frameName, frame] : CCDictionaryExt<gd::string, CCSpriteFrame*>(frames)) {
            spriteFrameCache->m_pSpriteFrames->setObject(frame, frameName);
            frameNames.push_back(frameName);
        }
        frames->release();
    }
    if (info) info->frameNames = frameNames;
    return frameNames;
}

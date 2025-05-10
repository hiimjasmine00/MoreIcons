#include "MoreIconsAPI.hpp"
#include "../classes/DummyNode.hpp"
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/utils/ranges.hpp>

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

bool MoreIconsAPI::hasIcon(const std::string& icon, IconType type) {
    return !icon.empty() && std::ranges::any_of(icons, [icon, type](const IconInfo& info) { return info.name == icon && info.type == type; });
}

int MoreIconsAPI::getCount(IconType type) {
    return std::ranges::count_if(icons, [type](const IconInfo& info) { return info.type == type; });
}

std::string MoreIconsAPI::getFrameName(const std::string& name, const std::string& prefix, IconType type) {
    if (type != IconType::Robot && type != IconType::Spider) {
        if (name.ends_with("_2_001.png")) return fmt::format("{}_2_001.png"_spr, prefix);
        else if (name.ends_with("_3_001.png")) return fmt::format("{}_3_001.png"_spr, prefix);
        else if (name.ends_with("_extra_001.png")) return fmt::format("{}_extra_001.png"_spr, prefix);
        else if (name.ends_with("_glow_001.png")) return fmt::format("{}_glow_001.png"_spr, prefix);
        else if (name.ends_with("_001.png")) return fmt::format("{}_001.png"_spr, prefix);
    }
    else for (int i = 1; i < 5; i++) {
        if (name.ends_with(fmt::format("_{:02}_2_001.png", i))) return fmt::format("{}_{:02}_2_001.png"_spr, prefix, i);
        else if (i == 1 && name.ends_with(fmt::format("_{:02}_extra_001.png", i))) return fmt::format("{}_{:02}_extra_001.png"_spr, prefix, i);
        else if (name.ends_with(fmt::format("_{:02}_glow_001.png", i))) return fmt::format("{}_{:02}_glow_001.png"_spr, prefix, i);
        else if (name.ends_with(fmt::format("_{:02}_001.png", i))) return fmt::format("{}_{:02}_001.png"_spr, prefix, i);
    }

    return name;
}

void MoreIconsAPI::loadIcon(const std::string& name, IconType type, int requestID) {
    if (!hasIcon(name, type)) return;

    auto& loadedIcon = loadedIcons[{ name, type }];

    if (loadedIcon < 1) {
        if (auto info = getIcon(name, type)) {
            auto textureCache = CCTextureCache::get();
            auto spriteFrameCache = CCSpriteFrameCache::get();

            CCTexture2D* texture = nullptr;
            for (int i = 0; i < info->textures.size(); i++) {
                texture = textureCache->addImage(info->textures[i].c_str(), true);
                if (info->frameNames.size() > i) spriteFrameCache->addSpriteFrame(
                    CCSpriteFrame::createWithTexture(texture, { { 0.0f, 0.0f }, texture->getContentSize() }), info->frameNames[i].c_str());
            }

            if (!info->sheetName.empty()) {
                if (auto dict = CCDictionary::createWithContentsOfFileThreadSafe(info->sheetName.c_str())) {
                    auto metadata = static_cast<CCDictionary*>(dict->objectForKey("metadata"));
                    auto formatStr = metadata ? metadata->valueForKey("format") : nullptr;
                    auto format = formatStr ? numFromString<int>(formatStr->m_sString).unwrapOr(0) : 0;
                    for (auto [frameName, frame] : CCDictionaryExt<std::string, CCDictionary*>(static_cast<CCDictionary*>(dict->objectForKey("frames")))) {
                        if (auto spriteFrame = createSpriteFrame(frame, texture, format))
                            spriteFrameCache->addSpriteFrame(spriteFrame, getFrameName(frameName, name, type).c_str());
                    }
                    dict->release();
                }
            }
        }
    }

    auto& requestedIcon = requestedIcons[requestID][type];
    if (requestedIcon != name) {
        loadedIcon++;
        if (!requestedIcon.empty()) unloadIcon(requestedIcon, type, requestID);
        requestedIcon = name;
    }
}

void MoreIconsAPI::unloadIcon(const std::string& name, IconType type, int requestID) {
    if (!hasIcon(name, type)) return;

    auto& loadedIcon = loadedIcons[{ name, type }];

    loadedIcon--;
    if (loadedIcon < 1) {
        if (auto info = getIcon(name, type)) {
            auto textureCache = CCTextureCache::get();
            auto spriteFrameCache = CCSpriteFrameCache::get();

            if (!info->sheetName.empty()) {
                if (auto dict = CCDictionary::createWithContentsOfFileThreadSafe(info->sheetName.c_str())) {
                    for (auto [frameName, frame] : CCDictionaryExt<std::string, CCDictionary*>(static_cast<CCDictionary*>(dict->objectForKey("frames")))) {
                        spriteFrameCache->removeSpriteFrameByName(getFrameName(frameName, name, type).c_str());
                    }
                    dict->release();
                }
            }

            for (int i = 0; i < info->textures.size(); i++) {
                textureCache->removeTextureForKey(info->textures[i].c_str());
                if (info->frameNames.size() > i) spriteFrameCache->removeSpriteFrameByName(info->frameNames[i].c_str());
            }
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
    updateSimplePlayer(player, MoreIcons::activeIcon(type, dual), type);
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

    sprite->setUserObject("name"_spr, CCString::create(icon));

    sprite->setBatchNode(nullptr);
    sprite->m_paSprite->setBatchNode(nullptr);

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
    updatePlayerObject(object, MoreIcons::activeIcon(type, dual), type);
}

void MoreIconsAPI::updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type) {
    if (!object || icon.empty() || !hasIcon(icon, type)) return;

    object->setUserObject("name"_spr, CCString::create(icon));

    if (type == IconType::Robot || type == IconType::Spider) {
        auto robotSprite = type == IconType::Robot ? object->m_robotSprite : object->m_spiderSprite;
        updateRobotSprite(robotSprite, icon, type);

        auto batchNode = type == IconType::Robot ? object->m_robotBatchNode : object->m_spiderBatchNode;
        auto hasExisted = batchNode && batchNode->getParent() == object->m_mainLayer;
        robotSprite->retain();
        if (batchNode) {
            robotSprite->removeFromParentAndCleanup(false);
            batchNode->removeFromParent();
            batchNode->release();
        }
        if (type == IconType::Robot) object->m_robotBatchNode = DummyNode::createWithTexture(robotSprite->getTexture(), 25);
        else object->m_spiderBatchNode = DummyNode::createWithTexture(robotSprite->getTexture(), 25);
        batchNode = type == IconType::Robot ? object->m_robotBatchNode : object->m_spiderBatchNode;
        batchNode->retain();
        batchNode->addChild(robotSprite);
        if (hasExisted && ((type == IconType::Robot && object->m_isRobot) || (type == IconType::Spider && object->m_isSpider)))
            object->m_mainLayer->addChild(batchNode, 2);
        return robotSprite->release();
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
    CCSpriteFrame* spriteFrame = nullptr;
    switch (format) {
        case 0:
            spriteFrame = CCSpriteFrame::createWithTexture(
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
            spriteFrame = CCSpriteFrame::createWithTexture(
                texture,
                rectFromString(dict->valueForKey("frame")),
                false,
                pointFromString(dict->valueForKey("offset")),
                sizeFromString(dict->valueForKey("sourceSize"))
            );
            break;
        case 2:
            spriteFrame = CCSpriteFrame::createWithTexture(
                texture,
                rectFromString(dict->valueForKey("frame")),
                boolValue(dict->valueForKey("rotated")),
                pointFromString(dict->valueForKey("offset")),
                sizeFromString(dict->valueForKey("sourceSize"))
            );
            break;
        case 3:
            spriteFrame = CCSpriteFrame::createWithTexture(
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
    return spriteFrame;
}

#include <MoreIcons.hpp>
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

    new EventListener(+[](std::vector<std::string>* vec, IconType type) {
        vec->clear();
        *vec = MoreIconsAPI::vectorForType(type);
        return ListenerResult::Propagate;
    }, MoreIcons::AllIconsFilter("all-icons"_spr));

    new EventListener(+[](std::string icon, IconType type) {
        MoreIconsAPI::setIcon(icon, type, false);
        return ListenerResult::Propagate;
    }, MoreIcons::LoadIconFilter("load-icon"_spr));

    new EventListener(+[](std::string icon, IconType type) {
        MoreIconsAPI::setIcon(icon, type, false);
        return ListenerResult::Propagate;
    }, MoreIcons::UnloadIconFilter("unload-icon"_spr));

    #if GEODE_COMP_GD_VERSION == 22074 // Keep this until the next Geometry Dash update
    new EventListener(+[](GJRobotSprite* sprite, const std::string& icon) {
        if (!sprite || icon.empty()) return ListenerResult::Propagate;
        MoreIconsAPI::updateRobotSprite(sprite, icon, sprite->m_iconType);
        return ListenerResult::Propagate;
    }, DispatchFilter<GJRobotSprite*, std::string>("robot-sprite"_spr));

    new EventListener(+[](PlayerObject* object, const std::string& icon) {
        if (!object || icon.empty()) return ListenerResult::Propagate;
        MoreIconsAPI::updatePlayerObject(object, icon, MoreIconsAPI::getIconType(object));
        return ListenerResult::Propagate;
    }, DispatchFilter<PlayerObject*, std::string>("player-object"_spr));

    new EventListener(+[](std::string* icon, IconType type, bool dual) {
        *icon = MoreIconsAPI::activeForType(type, dual);
        return ListenerResult::Propagate;
    }, DispatchFilter<std::string*, IconType, bool>("active-icon"_spr));

    new EventListener(+[](const std::string& icon, IconType type, bool dual) {
        MoreIconsAPI::setIcon(icon, type, dual);
        return ListenerResult::Propagate;
    }, DispatchFilter<std::string, IconType, bool>("set-icon"_spr));
    #endif
}

std::vector<std::string> MoreIconsAPI::vectorForType(IconType type) {
    return ranges::map<std::vector<std::string>>(ranges::filter(ICONS, [type](const IconInfo& info) {
        return info.type == type;
    }), [](const IconInfo& info) { return info.name; });
}

std::optional<IconInfo> MoreIconsAPI::infoForIcon(const std::string& name, IconType type) {
    return ranges::find(ICONS, [name, type](const IconInfo& info) { return info.name == name && info.type == type; });
}

std::string MoreIconsAPI::activeForType(IconType type, bool dual) {
    auto savedType = MoreIcons::savedForType(type, dual);
    return !savedType.empty() ? Mod::get()->getSavedValue<std::string>(savedType, "") : "";
}

std::string MoreIconsAPI::setIcon(const std::string& icon, IconType type, bool dual) {
    auto savedType = MoreIcons::savedForType(type, dual);
    return !savedType.empty() ? Mod::get()->setSavedValue(savedType, icon) : "";
}

bool MoreIconsAPI::hasIcon(const std::string& icon, IconType type) {
    return !icon.empty() && std::ranges::any_of(ICONS, [icon, type](const IconInfo& info) { return info.name == icon && info.type == type; });
}

int MoreIconsAPI::countForType(IconType type) {
    return std::ranges::count_if(ICONS, [type](const IconInfo& info) { return info.type == type; });
}

IconType MoreIconsAPI::getIconType(PlayerObject* object) {
    return MoreIcons::getIconType(object);
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

// Adapted from https://github.com/GlobedGD/globed2/blob/v1.7.2/src/util/cocos.cpp#L82

using AddSpriteFramesWithDictionaryType = void(CCSpriteFrameCache::*)(CCDictionary*, CCTexture2D*);
template <AddSpriteFramesWithDictionaryType func>
struct AddSpriteFramesWithDictionaryCaller {
    friend void addSpriteFrames(CCSpriteFrameCache* cache, CCDictionary* dict, CCTexture2D* texture) {
        (cache->*func)(dict, texture);
    }
};

template struct AddSpriteFramesWithDictionaryCaller<&CCSpriteFrameCache::addSpriteFramesWithDictionary>;
void addSpriteFrames(CCSpriteFrameCache* cache, CCDictionary* dict, CCTexture2D* texture);

using RemoveSpriteFramesFromDictionaryType = void(CCSpriteFrameCache::*)(CCDictionary*);
template <RemoveSpriteFramesFromDictionaryType func>
struct RemoveSpriteFramesFromDictionaryCaller {
    friend void removeSpriteFrames(CCSpriteFrameCache* cache, CCDictionary* dict) {
        (cache->*func)(dict);
    }
};

template struct RemoveSpriteFramesFromDictionaryCaller<&CCSpriteFrameCache::removeSpriteFramesFromDictionary>;
void removeSpriteFrames(CCSpriteFrameCache* cache, CCDictionary* dict);

void MoreIconsAPI::loadIcon(const std::string& name, IconType type) {
    if (ranges::contains(LOADED_ICONS, { name, type })) return;

    auto found = infoForIcon(name, type);
    if (!found.has_value()) return;

    auto& info = found.value();
    auto textureCache = CCTextureCache::get();
    auto spriteFrameCache = CCSpriteFrameCache::get();

    for (int i = 0; i < info.textures.size(); i++) {
        auto texture = textureCache->addImage(info.textures[i].c_str(), true);
        if (info.frameNames.size() > i) spriteFrameCache->addSpriteFrame(
            CCSpriteFrame::createWithTexture(texture, { { 0.0f, 0.0f }, texture->getContentSize() }), info.frameNames[i].c_str());
    }

    if (!info.sheetName.empty()) {
        auto dict = CCDictionary::createWithContentsOfFile(info.sheetName.c_str());
        auto frames = CCDictionary::create();
        for (auto [frameName, frame] : CCDictionaryExt<std::string, CCDictionary*>(static_cast<CCDictionary*>(dict->objectForKey("frames")))) {
            frames->setObject(frame, getFrameName(frameName, name, type));
        }
        dict->setObject(frames, "frames");
        addSpriteFrames(spriteFrameCache, dict, textureCache->textureForKey(info.textures[0].c_str()));
    }

    LOADED_ICONS.push_back({ name, type });
}

void MoreIconsAPI::unloadIcon(const std::string& name, IconType type) {
    if (!ranges::contains(LOADED_ICONS, { name, type })) return;

    auto found = infoForIcon(name, type);
    if (!found.has_value()) return;

    auto& info = found.value();
    auto textureCache = CCTextureCache::get();
    auto spriteFrameCache = CCSpriteFrameCache::get();

    for (int i = 0; i < info.textures.size(); i++) {
        textureCache->removeTextureForKey(info.textures[i].c_str());
        if (info.frameNames.size() > i) spriteFrameCache->removeSpriteFrameByName(info.frameNames[i].c_str());
    }

    if (!info.sheetName.empty()) {
        auto dict = CCDictionary::createWithContentsOfFile(info.sheetName.c_str());
        auto frames = CCDictionary::create();
        for (auto [frameName, frame] : CCDictionaryExt<std::string, CCDictionary*>(static_cast<CCDictionary*>(dict->objectForKey("frames")))) {
            frames->setObject(frame, getFrameName(frameName, name, type));
        }
        dict->setObject(frames, "frames");
        removeSpriteFrames(spriteFrameCache, dict);
    }

    ranges::remove(LOADED_ICONS, { name, type });
}

void MoreIconsAPI::updateSimplePlayer(SimplePlayer* player, IconType type, bool dual) {
    updateSimplePlayer(player, activeForType(type, dual), type);
}

void MoreIconsAPI::updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type) {
    if (!player || icon.empty() || !hasIcon(icon, type)) return;

    player->setUserObject("name"_spr, CCString::create(icon));
    player->setUserObject("type"_spr, CCInteger::create((int)type));

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

    loadIcon(icon, type);

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
    sprite->setUserObject("type"_spr, CCInteger::create((int)type));

    sprite->setBatchNode(nullptr);
    sprite->m_paSprite->setBatchNode(nullptr);

    loadIcon(icon, type);

    auto spriteParts = sprite->m_paSprite->m_spriteParts;
    auto spriteFrameCache = CCSpriteFrameCache::get();
    for (int i = 0; i < spriteParts->count(); i++) {
        auto spritePart = static_cast<CCSpritePart*>(spriteParts->objectAtIndex(i));
        auto tag = spritePart->getTag();

        spritePart->setBatchNode(nullptr);
        spritePart->setDisplayFrame(spriteFrameCache->spriteFrameByName(fmt::format("{}_{:02}_001.png"_spr, icon, tag).c_str()));
        if (auto secondSprite = static_cast<CCSprite*>(sprite->m_secondArray->objectAtIndex(i))) {
            secondSprite->setBatchNode(nullptr);
            secondSprite->setDisplayFrame(spriteFrameCache->spriteFrameByName(fmt::format("{}_{:02}_2_001.png"_spr, icon, tag).c_str()));
            secondSprite->setPosition(spritePart->getContentSize() / 2);
        }

        auto glowChild = static_cast<CCSprite*>(sprite->m_glowSprite->getChildren()->objectAtIndex(i));
        glowChild->setBatchNode(nullptr);
        glowChild->setDisplayFrame(spriteFrameCache->spriteFrameByName(fmt::format("{}_{:02}_glow_001.png"_spr, icon, tag).c_str()));

        if (spritePart == sprite->m_headSprite) {
            auto extraFrame = spriteFrameCache->spriteFrameByName(fmt::format("{}_{:02}_extra_001.png"_spr, icon, tag).c_str());
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
    updatePlayerObject(object, activeForType(type, dual), type);
}

void MoreIconsAPI::updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type) {
    if (!object || icon.empty() || !hasIcon(icon, type)) return;

    object->setUserObject("name"_spr, CCString::create(icon));
    object->setUserObject("type"_spr, CCInteger::create((int)type));

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

    loadIcon(icon, type);

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

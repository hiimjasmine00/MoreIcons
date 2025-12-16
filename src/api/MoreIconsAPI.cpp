#include "IconInfoImpl.hpp"
#include "../MoreIcons.hpp"
#include "../classes/popup/info/MoreInfoPopup.hpp"
#include "../utils/Get.hpp"
#include "../utils/Load.hpp"
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <MoreIcons.hpp>
#include <ranges>
#include <texpack.hpp>

using namespace geode::prelude;

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
    auto it = std::ranges::find(*icons, name, &IconInfo::getName);
    return it != icons->end() ? std::to_address(it) : nullptr;
}

CCTexture2D* more_icons::loadIcon(const std::string& name, IconType type, int requestID) {
    auto info = getIcon(name, type);
    if (!info) return nullptr;

    auto texture = Get::TextureCache()->textureForKey(info->getTextureString().c_str());
    if (MoreIcons::preloadIcons) return texture;

    auto& loadedIcon = MoreIcons::loadedIcons[{ name, type }];

    if (loadedIcon < 1) texture = MoreIcons::createAndAddFrames(info);

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
        for (auto& frameName : info->getFrameNames()) {
            spriteFrameCache->removeSpriteFrameByName(frameName.c_str());
        }
        info->moveFrameNames({});

        Get::TextureCache()->removeTextureForKey(info->getTextureString().c_str());
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

IconInfo* addIcon(
    const std::string& name, const std::string& shortName, IconType type,
    const std::filesystem::path& png, const std::filesystem::path& plist,
    const std::filesystem::path& json, const std::filesystem::path& icon, int quality,
    const std::string& packID, const std::string& packName,
    int specialID, const matjson::Value& specialInfo, int fireCount,
    bool vanilla, bool zipped
) {
    auto icons = more_icons::getIcons(type);
    if (!icons) return nullptr;
    auto it = std::ranges::find_if(*icons, [&packID, &shortName, type](const IconInfo& icon) {
        return icon.compare(packID, shortName, type) >= 0;
    });
    if (it != icons->end() && it->equals(name, type)) icons->erase(it);

    auto impl = std::make_shared<IconInfoImpl>();
    impl->m_name = name;
    impl->m_shortName = shortName;
    impl->m_packID = packID;
    impl->m_packName = packName;
    impl->m_texture = png;
    impl->m_sheet = plist;
    impl->m_json = json;
    impl->m_icon = icon;
    impl->m_type = type;
    impl->m_quality = quality;
    impl->m_specialID = specialID;
    impl->m_specialInfo = specialInfo;
    impl->m_fireCount = fireCount;
    impl->m_vanilla = vanilla;
    impl->m_zipped = zipped;
    return std::to_address(icons->insert(it, IconInfo(std::move(impl))));
}

IconInfo* more_icons::addIcon(
    const std::string& name, const std::string& shortName, IconType type,
    const std::filesystem::path& png, const std::filesystem::path& plist, TextureQuality quality,
    const std::string& packID, const std::string& packName,
    bool vanilla, bool zipped
) {
    return ::addIcon(name, shortName, type, png, plist, {}, {}, quality, packID, packName, 0, {}, 0, vanilla, zipped);
}

IconInfo* more_icons::addTrail(
    const std::string& name, const std::string& shortName,
    const std::filesystem::path& png, const std::filesystem::path& json, const std::filesystem::path& icon,
    const std::string& packID, const std::string& packName,
    int specialID, const matjson::Value& specialInfo,
    bool vanilla, bool zipped
) {
    return ::addIcon(name, shortName, IconType::Special, png, {}, json, icon, 0,
        packID, packName, specialID, specialInfo, 0, vanilla, zipped);
}

IconInfo* more_icons::addDeathEffect(
    const std::string& name, const std::string& shortName,
    const std::filesystem::path& png, const std::filesystem::path& plist,
    const std::filesystem::path& json, const std::filesystem::path& icon, TextureQuality quality,
    const std::string& packID, const std::string& packName,
    int specialID, const matjson::Value& specialInfo,
    bool vanilla, bool zipped
) {
    return ::addIcon(name, shortName, IconType::DeathEffect, png, plist, json, icon, quality,
        packID, packName, specialID, specialInfo, 0, vanilla, zipped);
}

IconInfo* more_icons::addShipFire(
    const std::string& name, const std::string& shortName,
    const std::filesystem::path& png, const std::filesystem::path& json, const std::filesystem::path& icon,
    const std::string& packID, const std::string& packName,
    int specialID, const matjson::Value& specialInfo,
    int fireCount, bool vanilla, bool zipped
) {
    return ::addIcon(name, shortName, IconType::ShipFire, png, {}, json, icon, 0,
        packID, packName, specialID, specialInfo, fireCount, vanilla, zipped);
}

void more_icons::moveIcon(IconInfo* info, const std::filesystem::path& path) {
    auto oldPngs = info->getAllTextures();
    auto type = info->getType();
    if (type <= IconType::Jetpack) {
        info->moveTexture(path / info->getTexture().filename());
        info->moveSheet(path / info->getSheet().filename());
    }
    else if (type >= IconType::DeathEffect) {
        if (auto& texture = info->getTexture(); !texture.empty()) {
            info->moveTexture(path / texture.parent_path().filename() / texture.filename());
        }
        if (auto& sheet = info->getSheet(); !sheet.empty()) {
            info->moveSheet(path / sheet.parent_path().filename() / sheet.filename());
        }
        if (auto& json = info->getJSON(); !json.empty()) {
            info->moveJSON(path / json.parent_path().filename() / json.filename());
        }
        if (auto& icon = info->getIcon(); !icon.empty()) {
            info->setIcon(path / icon.parent_path().filename() / icon.filename());
        }
    }

    info->setVanilla(false);

    auto textureCache = Get::TextureCache();
    auto newPngs = info->getAllTextures();
    for (size_t i = 0; i < newPngs.size(); i++) {
        auto& oldPng = oldPngs[i];
        if (Ref texture = textureCache->textureForKey(oldPng.c_str())) {
            textureCache->removeTextureForKey(oldPng.c_str());
            textureCache->m_pTextures->setObject(texture, newPngs[i]);
        }
    }
}

void more_icons::removeIcon(IconInfo* info) {
    auto& name = info->getName();
    auto type = info->getType();
    MoreIcons::loadedIcons[{ name, type }] = 0;

    auto spriteFrameCache = Get::SpriteFrameCache();
    for (auto& frame : info->getFrameNames()) {
        spriteFrameCache->removeSpriteFrameByName(frame.c_str());
    }
    for (auto& textureString : info->getAllTextures()) {
        Get::TextureCache()->removeTextureForKey(textureString.c_str());
    }

    if (!MoreIcons::preloadIcons && type <= IconType::Jetpack) {
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
    auto oldName = info->getName();
    auto newName = info->inTexturePack() ? fmt::format("{}:{}", info->getPackID(), name) : name;
    info->setName(newName);
    info->setShortName(name);

    auto oldPngs = info->getAllTextures();
    auto type = info->getType();

    if (type <= IconType::Jetpack) {
        constexpr std::array qualities = { L(""), L("-hd"), L("-uhd") };

        auto wideName = MoreIcons::strWide(name);
        auto quality = qualities[(int)info->getQuality() - 1];

        info->moveTexture(info->getTexture().parent_path() / fmt::format(L("{}{}.png"), wideName, quality));
        info->moveSheet(info->getSheet().parent_path() / fmt::format(L("{}{}.plist"), wideName, quality));
    }
    else if (type >= IconType::DeathEffect) {
        std::filesystem::path directory = MoreIcons::strWide(name);
        if (auto& texture = info->getTexture(); !texture.empty()) {
            info->moveTexture(texture.parent_path().parent_path() / directory / texture.filename());
        }
        if (auto& sheet = info->getSheet(); !sheet.empty()) {
            info->moveSheet(sheet.parent_path().parent_path() / directory / sheet.filename());
        }
        if (auto& json = info->getJSON(); !json.empty()) {
            info->moveJSON(json.parent_path().parent_path() / directory / json.filename());
        }
        if (auto& icon = info->getIcon(); !icon.empty()) {
            info->setIcon(icon.parent_path().parent_path() / directory / icon.filename());
        }
    }

    auto textureCache = Get::TextureCache();
    auto newPngs = info->getAllTextures();
    for (size_t i = 0; i < newPngs.size(); i++) {
        auto& oldPng = oldPngs[i];
        if (Ref texture = textureCache->textureForKey(oldPng.c_str())) {
            textureCache->removeTextureForKey(oldPng.c_str());
            textureCache->m_pTextures->setObject(texture, newPngs[i]);
        }
    }

    auto& frameNames = const_cast<std::vector<std::string>&>(info->getFrameNames());
    if (!frameNames.empty()) {
        auto spriteFrameCache = Get::SpriteFrameCache();
        for (auto& frameName : frameNames) {
            if (Ref spriteFrame = MoreIcons::getFrame(frameName.c_str())) {
                spriteFrameCache->removeSpriteFrameByName(frameName.c_str());
                frameName = Load::getFrameName(frameName, newName, type);
                spriteFrameCache->addSpriteFrame(spriteFrame, frameName.c_str());
            }
        }
    }

    if (type <= IconType::Jetpack) {
        if (auto it = MoreIcons::loadedIcons.find({ oldName, type }); it != MoreIcons::loadedIcons.end()) {
            MoreIcons::loadedIcons.emplace(std::make_pair(newName, type), it->second);
            MoreIcons::loadedIcons.erase(it);
        }

        for (auto& iconRequests : std::views::values(MoreIcons::requestedIcons)) {
            if (auto found = iconRequests.find(type); found != iconRequests.end() && found->second == oldName) {
                found->second = newName;
            }
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
    CCTexture2D* texture = nullptr;
    for (auto& textureString : info->getAllTextures()) {
        texture = Get::TextureCache()->textureForKey(textureString.c_str());
        if (!texture) continue;

        auto binaryRes = file::readBinary(info->getTexture());
        if (!binaryRes.isOk()) continue;

        auto imageRes = texpack::fromPNG(binaryRes.unwrap(), true);
        if (!imageRes.isOk()) continue;

        auto image = std::move(imageRes).unwrap();

        Load::initTexture(texture, image.data.data(), image.width, image.height);
    }

    if (!texture) return;

    auto framesRes = Load::createFrames(info->getSheet(), texture, info->getName(), info->getType());
    if (!framesRes.isOk()) return;

    auto frames = std::move(framesRes).unwrap();
    if (!frames) return;

    auto spriteFrameCache = Get::SpriteFrameCache();
    auto& frameNames = const_cast<std::vector<std::string>&>(info->getFrameNames());
    for (auto it = frameNames.begin(); it != frameNames.end();) {
        auto& frameName = *it;
        if (frames->objectForKey(frameName)) ++it;
        else {
            spriteFrameCache->removeSpriteFrameByName(frameName.c_str());
            it = frameNames.erase(it);
        }
    }

    for (auto [frameName, frame] : CCDictionaryExt<const char*, CCSpriteFrame*>(frames)) {
        if (auto spriteFrame = MoreIcons::getFrame(frameName)) {
            spriteFrame->m_obOffset = frame->m_obOffset;
            spriteFrame->m_obOriginalSize = frame->m_obOriginalSize;
            spriteFrame->m_obRectInPixels = frame->m_obRectInPixels;
            spriteFrame->m_bRotated = frame->m_bRotated;
            spriteFrame->m_obRect = frame->m_obRect;
            spriteFrame->m_obOffsetInPixels = frame->m_obOffsetInPixels;
            spriteFrame->m_obOriginalSizeInPixels = frame->m_obOriginalSizeInPixels;
        }
        else {
            spriteFrameCache->addSpriteFrame(frame, frameName);
            frameNames.push_back(frameName);
        }
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

    player->m_firstLayer->setDisplayFrame(MoreIcons::getFrame("{}_001.png"_spr, icon));
    player->m_firstLayer->setScale(type == IconType::Ball ? 0.9f : 1.0f);
    player->m_firstLayer->setPosition({ 0.0f, type == IconType::Ufo ? -7.0f : 0.0f });
    player->m_secondLayer->setDisplayFrame(MoreIcons::getFrame("{}_2_001.png"_spr, icon));
    auto firstCenter = player->m_firstLayer->getContentSize() / 2.0f;
    player->m_secondLayer->setPosition(firstCenter);
    player->m_outlineSprite->setDisplayFrame(MoreIcons::getFrame("{}_glow_001.png"_spr, icon));
    player->m_outlineSprite->setPosition(firstCenter);
    if (type == IconType::Ufo) {
        player->m_birdDome->setDisplayFrame(MoreIcons::getFrame("{}_3_001.png"_spr, icon));
        player->m_birdDome->setPosition(firstCenter);
    }
    auto extraFrame = MoreIcons::getFrame("{}_extra_001.png"_spr, icon);
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
        spritePart->setDisplayFrame(MoreIcons::getFrame("{}_{:02}_001.png"_spr, icon, tag));
        if (auto secondSprite = static_cast<CCSprite*>(sprite->m_secondArray->objectAtIndex(i))) {
            secondSprite->setBatchNode(nullptr);
            secondSprite->setDisplayFrame(MoreIcons::getFrame("{}_{:02}_2_001.png"_spr, icon, tag));
            secondSprite->setPosition(spritePart->getContentSize() / 2.0f);
        }

        if (auto glowChild = sprite->m_glowSprite->getChildByIndex<CCSprite>(i)) {
            glowChild->setBatchNode(nullptr);
            glowChild->setDisplayFrame(MoreIcons::getFrame("{}_{:02}_glow_001.png"_spr, icon, tag));
        }

        if (spritePart == sprite->m_headSprite) {
            auto extraFrame = MoreIcons::getFrame("{}_{:02}_extra_001.png"_spr, icon, tag);
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

    firstLayer->setDisplayFrame(MoreIcons::getFrame("{}_001.png"_spr, icon));
    secondLayer->setDisplayFrame(MoreIcons::getFrame("{}_2_001.png"_spr, icon));
    auto firstCenter = firstLayer->getContentSize() / 2.0f;
    secondLayer->setPosition(firstCenter);
    if (type == IconType::Ufo) {
        object->m_birdVehicle->setDisplayFrame(MoreIcons::getFrame("{}_3_001.png"_spr, icon));
        object->m_birdVehicle->setPosition(firstCenter);
    }
    outlineSprite->setDisplayFrame(MoreIcons::getFrame("{}_glow_001.png"_spr, icon));
    auto extraFrame = MoreIcons::getFrame("{}_extra_001.png"_spr, icon);
    detailSprite->setVisible(extraFrame != nullptr);
    if (extraFrame) {
        detailSprite->setDisplayFrame(extraFrame);
        detailSprite->setPosition(firstCenter);
    }
}

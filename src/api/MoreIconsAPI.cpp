#include "IconInfoImpl.hpp"
#include "../MoreIcons.hpp"
#include "../classes/popup/info/MoreInfoPopup.hpp"
#include "../utils/Filesystem.hpp"
#include "../utils/Get.hpp"
#include "../utils/Load.hpp"
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#define GEODE_DEFINE_EVENT_EXPORTS
#include <MoreIcons.hpp>
#include <ranges>
#include <texpack.hpp>

using namespace geode::prelude;
using namespace std::string_literals;

FLAlertLayer* more_icons::createInfoPopup(std::string_view name, IconType type) {
    if (auto info = more_icons::getIcon(name, type)) return MoreInfoPopup::create(info);
    else return nullptr;
}

std::map<IconType, std::vector<IconInfo>>& more_icons::getAllIcons() {
    return MoreIcons::icons;
}

std::vector<IconInfo>* more_icons::getIcons(IconType type) {
    auto it = MoreIcons::icons.find(type);
    return it != MoreIcons::icons.end() ? &it->second : nullptr;
}

IconInfo* more_icons::getIcon(std::string_view name, IconType type) {
    if (name.empty()) return nullptr;
    auto icons = getIcons(type);
    if (!icons) return nullptr;
    auto it = std::ranges::find(*icons, name, &IconInfo::getName);
    return it != icons->end() ? std::to_address(it) : nullptr;
}

CCTexture2D* more_icons::loadIcon(std::string_view name, IconType type, int requestID) {
    auto info = getIcon(name, type);
    if (!info) return nullptr;

    auto texture = Get::TextureCache()->textureForKey(info->getTextureString().c_str());
    if (MoreIcons::preloadIcons) return texture;

    auto& loadedIcon = MoreIcons::loadedIcons[{ info->getName(), type }];

    if (loadedIcon < 1) texture = MoreIcons::createAndAddFrames(info);

    auto& requestedIcon = MoreIcons::requestedIcons[requestID][type];
    if (requestedIcon != name) {
        loadedIcon++;
        if (!requestedIcon.empty()) unloadIcon(requestedIcon, type, requestID);
        MoreIcons::requestedIcons[requestID][type] = name;
    }

    return texture;
}

void more_icons::unloadIcon(std::string_view name, IconType type, int requestID) {
    if (MoreIcons::preloadIcons || name.empty()) return;

    auto info = getIcon(name, type);
    if (!info) return;

    auto& loadedIcon = MoreIcons::loadedIcons[{ info->getName(), type }];

    loadedIcon--;
    if (loadedIcon < 1) {
        auto spriteFrameCache = Get::SpriteFrameCache();
        for (auto& frameName : info->getFrameNames()) {
            spriteFrameCache->removeSpriteFrameByName(frameName.c_str());
        }
        info->setFrameNames({});

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
    std::string name, std::string shortName, IconType type, std::filesystem::path png, std::filesystem::path plist,
    std::filesystem::path json, std::filesystem::path icon, int quality, std::string packID, std::string packName,
    int specialID, matjson::Value specialInfo, int fireCount, bool vanilla, bool zipped
) {
    auto icons = more_icons::getIcons(type);
    if (!icons) return nullptr;
    auto it = std::ranges::find_if(*icons, [&packID, &shortName, type](const IconInfo& icon) {
        return icon.compare(packID, shortName, type) >= 0;
    });
    if (it != icons->end() && it->equals(name, type)) icons->erase(it);

    auto impl = std::make_shared<IconInfoImpl>();
    impl->m_name = std::move(name);
    impl->m_shortName = std::move(shortName);
    impl->m_packID = std::move(packID);
    impl->m_packName = std::move(packName);
    impl->m_texture = std::move(png);
    impl->m_sheet = std::move(plist);
    impl->m_json = std::move(json);
    impl->m_icon = std::move(icon);
    impl->m_type = type;
    impl->m_quality = quality;
    impl->m_specialID = specialID;
    impl->m_specialInfo = std::move(specialInfo);
    impl->m_fireCount = fireCount;
    impl->m_vanilla = vanilla;
    impl->m_zipped = zipped;
    return std::to_address(icons->insert(it, IconInfo(std::move(impl))));
}

IconInfo* more_icons::addIcon(
    std::string name, std::string shortName, IconType type, std::filesystem::path png, std::filesystem::path plist,
    TextureQuality quality, std::string packID, std::string packName, bool vanilla, bool zipped
) {
    return ::addIcon(
        std::move(name), std::move(shortName), type, std::move(png), std::move(plist), {}, {},
        quality, std::move(packID), std::move(packName), 0, {}, 0, vanilla, zipped
    );
}

IconInfo* more_icons::addTrail(
    std::string name, std::string shortName, std::filesystem::path png, std::filesystem::path json, std::filesystem::path icon,
    std::string packID, std::string packName, int specialID, matjson::Value specialInfo, bool vanilla, bool zipped
) {
    return ::addIcon(
        std::move(name), std::move(shortName), IconType::Special, std::move(png), {}, std::move(json), std::move(icon),
        0, std::move(packID), std::move(packName), specialID, std::move(specialInfo), 0, vanilla, zipped
    );
}

IconInfo* more_icons::addDeathEffect(
    std::string name, std::string shortName,
    std::filesystem::path png, std::filesystem::path plist,
    std::filesystem::path json, std::filesystem::path icon, TextureQuality quality,
    std::string packID, std::string packName,
    int specialID, matjson::Value specialInfo,
    bool vanilla, bool zipped
) {
    return ::addIcon(
        std::move(name), std::move(shortName), IconType::DeathEffect, std::move(png), std::move(plist), std::move(json), std::move(icon),
        quality, std::move(packID), std::move(packName), specialID, std::move(specialInfo), 0, vanilla, zipped
    );
}

IconInfo* more_icons::addShipFire(
    std::string name, std::string shortName,
    std::filesystem::path png, std::filesystem::path json, std::filesystem::path icon,
    std::string packID, std::string packName,
    int specialID, matjson::Value specialInfo,
    int fireCount, bool vanilla, bool zipped
) {
    return ::addIcon(
        std::move(name), std::move(shortName), IconType::ShipFire, std::move(png), {}, std::move(json), std::move(icon),
        0, std::move(packID), std::move(packName), specialID, std::move(specialInfo), fireCount, vanilla, zipped
    );
}

void more_icons::moveIcon(IconInfo* info, const std::filesystem::path& path) {
    auto oldPngs = info->getAllTextures();
    auto type = info->getType();
    if (type <= IconType::Jetpack) {
        info->setTexture(path / Filesystem::filenamePath(info->getTexture()));
        info->setSheet(path / Filesystem::filenamePath(info->getSheet()));
    }
    else if (type >= IconType::DeathEffect) {
        if (info->isVanilla()) {
            if (type == IconType::DeathEffect) {
                switch (info->getQuality()) {
                    case kTextureQualityHigh:
                        info->setTexture(path / L("effect-uhd.png"s));
                        info->setSheet(path / L("effect-uhd.plist"s));
                        break;
                    case kTextureQualityMedium:
                        info->setTexture(path / L("effect-hd.png"s));
                        info->setSheet(path / L("effect-hd.plist"s));
                        break;
                    case kTextureQualityLow:
                        info->setTexture(path / L("effect.png"s));
                        info->setSheet(path / L("effect.plist"s));
                        break;
                }
            }
            else if (type == IconType::Special) {
                info->setTexture(path / L("trail.png"s));
            }
            else if (type == IconType::ShipFire) {
                info->setTexture(path / L("fire_001.png"s));
            }

            info->setJSON(path / L("settings.json"s));

            auto factor = Get::Director()->getContentScaleFactor();
            if (factor >= 4.0f) info->setIcon(path / L("icon-uhd.png"s));
            else if (factor >= 2.0f) info->setIcon(path / L("icon-hd.png"s));
            else info->setIcon(path / L("icon.png"s));
        }
        else {
            if (auto& texture = info->getTexture(); !texture.empty()) {
                info->setTexture(path / Filesystem::filenamePath(texture));
            }
            if (auto& sheet = info->getSheet(); !sheet.empty()) {
                info->setSheet(path / Filesystem::filenamePath(sheet));
            }
            if (auto& json = info->getJSON(); !json.empty()) {
                info->setJSON(path / Filesystem::filenamePath(json));
            }
            if (auto& icon = info->getIcon(); !icon.empty()) {
                info->setIcon(path / Filesystem::filenamePath(icon));
            }
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

void more_icons::renameIcon(IconInfo* info, std::string name) {
    auto oldName = info->getName();
    info->setName(info->inTexturePack() ? fmt::format("{}:{}", info->getPackID(), name) : name);
    info->setShortName(std::move(name));

    auto oldPngs = info->getAllTextures();
    auto wideName = Filesystem::strWide(info->getShortName());
    auto& newName = info->getName();
    auto type = info->getType();

    if (type <= IconType::Jetpack) {
        switch (info->getQuality()) {
            case kTextureQualityHigh:
                info->setTexture(Filesystem::parentPath(info->getTexture()) / fmt::format(L("{}-uhd.png"), wideName));
                info->setSheet(Filesystem::parentPath(info->getSheet()) / fmt::format(L("{}-uhd.plist"), wideName));
                break;
            case kTextureQualityMedium:
                info->setTexture(Filesystem::parentPath(info->getTexture()) / fmt::format(L("{}-hd.png"), wideName));
                info->setSheet(Filesystem::parentPath(info->getSheet()) / fmt::format(L("{}-hd.plist"), wideName));
                break;
            case kTextureQualityLow:
                info->setTexture(Filesystem::parentPath(info->getTexture()) / fmt::format(L("{}.png"), wideName));
                info->setSheet(Filesystem::parentPath(info->getSheet()) / fmt::format(L("{}.plist"), wideName));
                break;
        }
    }
    else if (type >= IconType::DeathEffect) {
        std::filesystem::path directory = std::move(wideName);
        if (auto& texture = info->getTexture(); !texture.empty()) {
            info->setTexture(Filesystem::parentPath(Filesystem::parentPath(texture)) / directory / Filesystem::filenamePath(texture));
        }
        if (auto& sheet = info->getSheet(); !sheet.empty()) {
            info->setSheet(Filesystem::parentPath(Filesystem::parentPath(sheet)) / directory / Filesystem::filenamePath(sheet));
        }
        if (auto& json = info->getJSON(); !json.empty()) {
            info->setJSON(Filesystem::parentPath(Filesystem::parentPath(json)) / directory / Filesystem::filenamePath(json));
        }
        if (auto& icon = info->getIcon(); !icon.empty()) {
            info->setIcon(Filesystem::parentPath(Filesystem::parentPath(icon)) / directory / Filesystem::filenamePath(icon));
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
            frameNames.emplace_back(frameName);
        }
    }
}

void more_icons::updateSimplePlayer(SimplePlayer* player, std::string_view icon, IconType type) {
    if (!player || icon.empty() || !hasIcon(icon, type)) return;

    MoreIcons::setName(player, icon);

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

    auto firstLayer = player->m_firstLayer;
    firstLayer->setDisplayFrame(MoreIcons::getFrame("{}_001.png"_spr, icon));
    firstLayer->setScale(type == IconType::Ball ? 0.9f : 1.0f);
    firstLayer->setPosition({ 0.0f, type == IconType::Ufo ? -7.0f : 0.0f });

    auto firstCenter = firstLayer->getContentSize() / 2.0f;

    auto secondLayer = player->m_secondLayer;
    secondLayer->setDisplayFrame(MoreIcons::getFrame("{}_2_001.png"_spr, icon));
    secondLayer->setPosition(firstCenter);

    auto outlineSprite = player->m_outlineSprite;
    outlineSprite->setDisplayFrame(MoreIcons::getFrame("{}_glow_001.png"_spr, icon));
    outlineSprite->setPosition(firstCenter);

    if (type == IconType::Ufo) {
        auto birdDome = player->m_birdDome;
        birdDome->setDisplayFrame(MoreIcons::getFrame("{}_3_001.png"_spr, icon));
        birdDome->setPosition(firstCenter);
    }

    auto extraFrame = MoreIcons::getFrame("{}_extra_001.png"_spr, icon);
    auto detailSprite = player->m_detailSprite;
    detailSprite->setVisible(extraFrame != nullptr);
    if (extraFrame) {
        detailSprite->setDisplayFrame(extraFrame);
        detailSprite->setPosition(firstCenter);
    }
}

void more_icons::updateRobotSprite(GJRobotSprite* sprite, std::string_view icon, IconType type) {
    if (!sprite || icon.empty() || !hasIcon(icon, type)) return;

    MoreIcons::setName(sprite, icon);

    auto texture = loadIcon(icon, type, sprite->m_iconRequestID);
    sprite->setBatchNode(nullptr);
    sprite->setTexture(texture);

    auto paSprite = sprite->m_paSprite;
    paSprite->setBatchNode(nullptr);
    paSprite->setTexture(texture);

    auto spriteParts = paSprite->m_spriteParts;
    auto secondArray = sprite->m_secondArray;
    auto glowArray = sprite->m_glowSprite->getChildren();
    auto headSprite = sprite->m_headSprite;
    auto extraSprite = sprite->m_extraSprite;

    for (int i = 0; i < spriteParts->count(); i++) {
        auto spritePart = static_cast<CCSprite*>(spriteParts->objectAtIndex(i));
        auto tag = spritePart->getTag();

        spritePart->setBatchNode(nullptr);
        spritePart->setDisplayFrame(MoreIcons::getFrame("{}_{:02}_001.png"_spr, icon, tag));
        if (auto secondSprite = static_cast<CCSprite*>(secondArray->objectAtIndex(i))) {
            secondSprite->setBatchNode(nullptr);
            secondSprite->setDisplayFrame(MoreIcons::getFrame("{}_{:02}_2_001.png"_spr, icon, tag));
            secondSprite->setPosition(spritePart->getContentSize() / 2.0f);
        }

        if (auto glowChild = static_cast<CCSprite*>(glowArray->objectAtIndex(i))) {
            glowChild->setBatchNode(nullptr);
            glowChild->setDisplayFrame(MoreIcons::getFrame("{}_{:02}_glow_001.png"_spr, icon, tag));
        }

        if (spritePart == headSprite) {
            auto extraFrame = MoreIcons::getFrame("{}_{:02}_extra_001.png"_spr, icon, tag);
            if (extraFrame) {
                if (extraSprite) {
                    extraSprite->setBatchNode(nullptr);
                    extraSprite->setDisplayFrame(extraFrame);
                }
                else {
                    extraSprite = CCSprite::createWithSpriteFrame(extraFrame);
                    sprite->m_extraSprite = extraSprite;
                    spritePart->addChild(extraSprite, 2);
                }
                extraSprite->setPosition(spritePart->getContentSize() / 2.0f);
            }
            extraSprite->setVisible(extraFrame != nullptr);
        }
    }
}

void more_icons::updatePlayerObject(PlayerObject* object, std::string_view icon, IconType type) {
    if (!object || icon.empty() || !hasIcon(icon, type)) return;

    MoreIcons::setName(object, icon);

    if (type == IconType::Robot) {
        if (Ref robotSprite = object->m_robotSprite) {
            robotSprite->removeFromParentAndCleanup(false);
            updateRobotSprite(robotSprite, icon, type);
            auto batchNode = object->m_robotBatchNode;
            batchNode->setTexture(robotSprite->getTexture());
            batchNode->addChild(robotSprite);
        }
        return;
    }
    else if (type == IconType::Spider) {
        if (Ref spiderSprite = object->m_spiderSprite) {
            spiderSprite->removeFromParentAndCleanup(false);
            updateRobotSprite(spiderSprite, icon, type);
            auto batchNode = object->m_spiderBatchNode;
            batchNode->setTexture(spiderSprite->getTexture());
            batchNode->addChild(spiderSprite);
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

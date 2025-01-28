#include "MoreIconsAPI.hpp"
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

void DummyNode::setBlendFunc(ccBlendFunc blendFunc) {
    CCSpriteBatchNode::setBlendFunc(blendFunc);

    for (auto child : CCArrayExt<CCNode*>(getChildren())) {
        if (typeinfo_cast<CCBlendProtocol*>(child)) recursiveBlend(child, blendFunc);
    }
}

void DummyNode::recursiveBlend(CCNode* node, ccBlendFunc blendFunc) {
    if (!node) return;

    if (auto blendNode = typeinfo_cast<CCBlendProtocol*>(node)) blendNode->setBlendFunc(blendFunc);

    for (auto child : CCArrayExt<CCNode*>(node->getChildren())) {
        if (typeinfo_cast<CCBlendProtocol*>(child)) recursiveBlend(child, blendFunc);
    }
}

std::vector<std::string>& MoreIconsAPI::vectorForType(IconType type) {
    switch (type) {
        case IconType::Cube: return ICONS;
        case IconType::Ship: return SHIPS;
        case IconType::Ball: return BALLS;
        case IconType::Ufo: return UFOS;
        case IconType::Wave: return WAVES;
        case IconType::Robot: return ROBOTS;
        case IconType::Spider: return SPIDERS;
        case IconType::Swing: return SWINGS;
        case IconType::Jetpack: return JETPACKS;
        case IconType::Special: return TRAILS;
        default: {
            static std::vector<std::string> empty;
            return empty;
        }
    }
}

std::string_view MoreIconsAPI::savedForType(IconType type, bool dual) {
    auto isDual = Loader::get()->isModLoaded("weebify.separate_dual_icons") && dual;
    switch (type) {
        case IconType::Cube: return isDual ? "icon-dual" : "icon";
        case IconType::Ship: return isDual ? "ship-dual" : "ship";
        case IconType::Ball: return isDual ? "ball-dual" : "ball";
        case IconType::Ufo: return isDual ? "ufo-dual" : "ufo";
        case IconType::Wave: return isDual ? "wave-dual" : "wave";
        case IconType::Robot: return isDual ? "robot-dual" : "robot";
        case IconType::Spider: return isDual ? "spider-dual" : "spider";
        case IconType::Swing: return isDual ? "swing-dual" : "swing";
        case IconType::Jetpack: return isDual ? "jetpack-dual" : "jetpack";
        case IconType::Special: return isDual ? "trail-dual" : "trail";
        default: return "";
    }
}

void MoreIconsAPI::updateSimplePlayer(SimplePlayer* player, const std::string& icon, IconType type) {
    if (!player || icon.empty() || !hasIcon(icon, type)) return;

    setUserObject(player, icon);

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
    else if (type == IconType::Spider) {
        if (!player->m_spiderSprite) player->createSpiderSprite(1);
        player->m_spiderSprite->setVisible(true);
        player->m_spiderSprite->m_color = player->m_firstLayer->getColor();
        player->m_spiderSprite->m_secondColor = player->m_secondLayer->getColor();
        player->m_spiderSprite->updateColors();
        return updateRobotSprite(player->m_spiderSprite, icon, type);
    }

    auto iconFrame = fmt::format("{}_001.png"_spr, icon);
    auto iconFrame2 = fmt::format("{}_2_001.png"_spr, icon);
    auto iconFrame3 = fmt::format("{}_3_001.png"_spr, icon);
    auto iconFrameExtra = fmt::format("{}_extra_001.png"_spr, icon);
    auto iconFrameGlow = fmt::format("{}_glow_001.png"_spr, icon);

    auto sfc = CCSpriteFrameCache::get();
    player->m_firstLayer->setDisplayFrame(sfc->spriteFrameByName(iconFrame.c_str()));
    player->m_firstLayer->setScale(type == IconType::Ball ? 0.9f : 1.0f);
    player->m_firstLayer->setPosition({ 0.0f, type == IconType::Ufo ? -7.0f : 0.0f });
    player->m_secondLayer->setDisplayFrame(sfc->spriteFrameByName(iconFrame2.c_str()));
    auto firstCenter = player->m_firstLayer->getContentSize() / 2;
    player->m_secondLayer->setPosition(firstCenter);
    player->m_outlineSprite->setDisplayFrame(sfc->spriteFrameByName(iconFrameGlow.c_str()));
    player->m_outlineSprite->setPosition(firstCenter);
    if (type == IconType::Ufo) {
        player->m_birdDome->setDisplayFrame(sfc->spriteFrameByName(iconFrame3.c_str()));
        player->m_birdDome->setPosition(firstCenter);
    }
    auto extraFrame = sfc->spriteFrameByName(iconFrameExtra.c_str());
    auto extraVisible = doesExist(extraFrame);
    player->m_detailSprite->setVisible(extraVisible);
    if (extraVisible) {
        player->m_detailSprite->setDisplayFrame(extraFrame);
        player->m_detailSprite->setPosition(firstCenter);
    }
}

void MoreIconsAPI::updateRobotSprite(GJRobotSprite* sprite, const std::string& icon, IconType type) {
    if (!sprite || icon.empty() || !hasIcon(icon, type)) return;

    setUserObject(sprite, icon);

    sprite->setBatchNode(nullptr);
    sprite->m_paSprite->setBatchNode(nullptr);

    auto spriteParts = sprite->m_paSprite->m_spriteParts;
    auto spriteFrameCache = CCSpriteFrameCache::get();
    for (int i = 0; i < spriteParts->count(); i++) {
        auto spritePart = static_cast<CCSpritePart*>(spriteParts->objectAtIndex(i));
        auto tag = spritePart->getTag();

        auto spriteFrame = fmt::format("{}_{:02}_001.png"_spr, icon, tag);
        auto spriteFrame2 = fmt::format("{}_{:02}_2_001.png"_spr, icon, tag);
        auto spriteFrameExtra = fmt::format("{}_{:02}_extra_001.png"_spr, icon, tag);
        auto spriteFrameGlow = fmt::format("{}_{:02}_glow_001.png"_spr, icon, tag);

        spritePart->setBatchNode(nullptr);
        spritePart->setDisplayFrame(spriteFrameCache->spriteFrameByName(spriteFrame.c_str()));
        if (auto secondSprite = static_cast<CCSprite*>(sprite->m_secondArray->objectAtIndex(i))) {
            secondSprite->setBatchNode(nullptr);
            secondSprite->setDisplayFrame(spriteFrameCache->spriteFrameByName(spriteFrame2.c_str()));
            secondSprite->setPosition(spritePart->getContentSize() / 2);
        }

        auto glowChild = static_cast<CCSprite*>(sprite->m_glowSprite->getChildren()->objectAtIndex(i));
        glowChild->setBatchNode(nullptr);
        glowChild->setDisplayFrame(spriteFrameCache->spriteFrameByName(spriteFrameGlow.c_str()));

        if (spritePart == sprite->m_headSprite) {
            auto extraFrame = spriteFrameCache->spriteFrameByName(spriteFrameExtra.c_str());
            auto hasExtra = doesExist(extraFrame);
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

void MoreIconsAPI::updatePlayerObject(PlayerObject* object, const std::string& icon, IconType type) {
    if (!object || icon.empty() || !hasIcon(icon, type)) return;

    setUserObject(object, icon);

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

    auto iconFrame = fmt::format("{}_001.png"_spr, icon);
    auto iconFrame2 = fmt::format("{}_2_001.png"_spr, icon);
    auto iconFrame3 = fmt::format("{}_3_001.png"_spr, icon);
    auto iconFrameExtra = fmt::format("{}_extra_001.png"_spr, icon);
    auto iconFrameGlow = fmt::format("{}_glow_001.png"_spr, icon);

    auto isVehicle = type == IconType::Ship || type == IconType::Ufo || type == IconType::Jetpack;
    auto firstLayer = isVehicle ? object->m_vehicleSprite : object->m_iconSprite;
    auto secondLayer = isVehicle ? object->m_vehicleSpriteSecondary : object->m_iconSpriteSecondary;
    auto outlineSprite = isVehicle ? object->m_vehicleGlow : object->m_iconGlow;
    auto detailSprite = isVehicle ? object->m_vehicleSpriteWhitener : object->m_iconSpriteWhitener;

    auto sfc = CCSpriteFrameCache::get();
    firstLayer->setDisplayFrame(sfc->spriteFrameByName(iconFrame.c_str()));
    secondLayer->setDisplayFrame(sfc->spriteFrameByName(iconFrame2.c_str()));
    auto firstCenter = firstLayer->getContentSize() / 2;
    secondLayer->setPosition(firstCenter);
    if (type == IconType::Ufo) {
        object->m_birdVehicle->setDisplayFrame(sfc->spriteFrameByName(iconFrame3.c_str()));
        object->m_birdVehicle->setPosition(firstCenter);
    }
    outlineSprite->setDisplayFrame(sfc->spriteFrameByName(iconFrameGlow.c_str()));
    auto extraFrame = sfc->spriteFrameByName(iconFrameExtra.c_str());
    auto extraVisible = doesExist(extraFrame);
    detailSprite->setVisible(extraVisible);
    if (extraVisible) {
        detailSprite->setDisplayFrame(extraFrame);
        detailSprite->setPosition(firstCenter);
    }
}

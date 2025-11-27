#include "LazyIcon.hpp"
#include "ViewIconPopup.hpp"
#include "../../misc/ThreadPool.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/binding/CCAnimateFrameCache.hpp>
#include <Geode/binding/CCSpritePlus.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/ObjectManager.hpp>
#include <Geode/binding/SpriteDescription.hpp>
#include <Geode/loader/Mod.hpp>
#include <jasmine/convert.hpp>

using namespace geode::prelude;

LazyIcon* LazyIcon::create(IconType type, int id, IconInfo* info) {
    auto ret = new LazyIcon();
    if (ret->init(type, id, info)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LazyIcon::init(IconType type, int id, IconInfo* info) {
    auto normalImage = CCNode::create();
    if (!CCMenuItemSpriteExtra::init(normalImage, nullptr, nullptr, nullptr)) return false;

    auto contentSize = MoreIconsAPI::getSpriteFrameCache()->spriteFrameByName("playerSquare_001.png")->getOriginalSize();
    setPosition(contentSize / 2.0f);
    setContentSize(contentSize);
    normalImage->setPosition(contentSize / 2.0f);

    m_type = type;
    m_id = id;
    m_info = info;
    m_name = info ? info->name : fmt::format("{}{:02}", MoreIconsAPI::prefixes[MoreIconsAPI::convertType(type)], id);
    setID(m_name);

    if (type == IconType::Special && !info) {
        m_visited = true;
        auto playerSpecial = CCSprite::createWithSpriteFrameName(fmt::format("player_special_{:02}_001.png", id).c_str());
        playerSpecial->setID("player-special");
        normalImage->addChild(playerSpecial);
        return true;
    }

    if (info) {
        m_texture = info->textures[0];
        m_sheet = info->sheetName;
    }
    else {
        std::string fullName = MoreIconsAPI::getGameManager()->sheetNameForIcon(id, (int)type);
        auto fileUtils = MoreIconsAPI::getFileUtils();
        m_texture = fileUtils->fullPathForFilename(fmt::format("{}.png", fullName).c_str(), false);
        m_sheet = fileUtils->fullPathForFilename(fmt::format("{}.plist", fullName).c_str(), false);
    }

    if (type <= IconType::Jetpack || (type == IconType::Special && info)) {
        if (MoreIconsAPI::getTextureCache()->textureForKey(m_texture.c_str())) {
            m_visited = true;
            m_texture = "";
            createIcon();
        }
        else {
            setEnabled(false);
            m_loadingSprite = CCSprite::create("loadingCircle.png");
            m_loadingSprite->setScale(0.4f);
            m_loadingSprite->setBlendFunc({ GL_ONE, GL_ONE });
            m_loadingSprite->runAction(CCRepeatForever::create(CCRotateBy::create(1.0f, 360.0f)));
            normalImage->addChild(m_loadingSprite);
        }
    }

    return true;
}

void LazyIcon::createSimpleIcon() {
    auto ufo = m_type == IconType::Ufo;
    auto iconName = m_info ? fmt::format("{}"_spr, m_name) : m_name;
    auto primaryFrame = MoreIconsAPI::getFrame(fmt::format("{}_001.png", iconName));
    auto secondaryFrame = MoreIconsAPI::getFrame(fmt::format("{}_2_001.png", iconName));
    auto tertiaryFrame = ufo ? MoreIconsAPI::getFrame(fmt::format("{}_3_001.png", iconName)) : nullptr;
    auto glowFrame = MoreIconsAPI::getFrame(fmt::format("{}_glow_001.png", iconName));
    auto extraFrame = MoreIconsAPI::getFrame(fmt::format("{}_extra_001.png", iconName));
    auto normalImage = getNormalImage();

    if (primaryFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(primaryFrame);
        if (ufo) sprite->setPositionY(-7.0f);
        sprite->setID("primary-sprite");
        normalImage->addChild(sprite, 0);
    }

    if (secondaryFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(secondaryFrame);
        if (ufo) sprite->setPositionY(-7.0f);
        sprite->setID("secondary-sprite");
        normalImage->addChild(sprite, -1);
    }

    if (tertiaryFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(tertiaryFrame);
        sprite->setPositionY(-7.0f);
        sprite->setID("tertiary-sprite");
        normalImage->addChild(sprite, -2);
    }

    if (glowFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(glowFrame);
        if (ufo) sprite->setPositionY(-7.0f);
        sprite->setID("glow-sprite");
        normalImage->addChild(sprite, -3);
    }

    if (extraFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(extraFrame);
        if (ufo) sprite->setPositionY(-7.0f);
        sprite->setID("extra-sprite");
        normalImage->addChild(sprite, 1);
    }
}

void LazyIcon::createComplexIcon() {
    auto spider = m_type == IconType::Spider;
    auto def = MoreIconsAPI::uppercase[(int)m_type];
    auto anim = "idle";

    auto definition = MoreIconsAPI::getObjectManager()->getDefinition(def);
    if (!definition) return;

    auto object = static_cast<CCObject*>(MoreIconsAPI::getAnimationCache()->animationByName(fmt::format("{}_{}", def, anim).c_str()));
    if (!object) return;

    auto animations = static_cast<CCDictionary*>(definition->objectForKey("animations"));
    if (!animations) return;

    auto animation = static_cast<CCDictionary*>(animations->objectForKey(anim));
    if (!animation) return;

    auto usedTextures = static_cast<CCDictionary*>(MoreIconsAPI::getAnimateFrameCache()->addSpriteFramesWithFile(
        definition->valueForKey("animDesc")->getCString())->objectForKey("usedTextures"));
    if (!usedTextures) return;

    auto iconName = m_info ? fmt::format("{}"_spr, m_name) : m_name;
    auto normalImage = getNormalImage();

    auto glowNode = CCNode::create();
    glowNode->setAnchorPoint({ 0.5f, 0.5f });
    glowNode->setID("glow-node");

    m_spriteParts = CCArray::create();

    for (int i = 0; i < usedTextures->count(); i++) {
        auto usedTexture = static_cast<CCDictionary*>(usedTextures->objectForKey(fmt::format("texture_{}", i)));
        if (!usedTexture) continue;

        std::string_view texture = usedTexture->valueForKey("texture")->m_sString;
        if (texture.size() < spider + 11) continue;

        auto index = jasmine::convert::getInt<int>(texture.substr(spider + 9, 2)).value_or(0);
        if (index <= 0) continue;

        std::string_view customID = usedTexture->valueForKey("customID")->m_sString;
        uint8_t spriteColor = customID == "back01" || customID == "back02" || customID == "back03" ? (spider ? 127 : 178) : 255;
        ccColor3B spriteColor3B = { spriteColor, spriteColor, spriteColor };

        auto partNode = new CCSpritePlus();
        partNode->init();
        partNode->autorelease();
        partNode->m_propagateScaleChanges = true;
        partNode->m_propagateFlipChanges = true;
        partNode->setID(fmt::format("part-node-{}", i + 1));

        if (auto primaryFrame = MoreIconsAPI::getFrame(fmt::format("{}_{:02}_001.png", iconName, index))) {
            auto sprite = CCSprite::createWithSpriteFrame(primaryFrame);
            sprite->setColor(spriteColor3B);
            sprite->setID(fmt::format("primary-sprite-{}", i + 1));
            partNode->addChild(sprite, 0);
        }

        if (auto secondaryFrame = MoreIconsAPI::getFrame(fmt::format("{}_{:02}_2_001.png", iconName, index))) {
            auto sprite = CCSprite::createWithSpriteFrame(secondaryFrame);
            sprite->setColor(spriteColor3B);
            sprite->setID(fmt::format("secondary-sprite-{}", i + 1));
            partNode->addChild(sprite, -1);
        }

        if (auto glowFrame = MoreIconsAPI::getFrame(fmt::format("{}_{:02}_glow_001.png", iconName, index))) {
            auto sprite = CCSprite::createWithSpriteFrame(glowFrame);
            sprite->setID(fmt::format("glow-sprite-{}", i + 1));
            glowNode->addChild(sprite, -1);
            partNode->addFollower(sprite);
        }

        if (index == 1) {
            if (auto extraFrame = MoreIconsAPI::getFrame(fmt::format("{}_01_extra_001.png", iconName))) {
                auto sprite = CCSprite::createWithSpriteFrame(extraFrame);
                sprite->setID(fmt::format("extra-sprite-{}", i + 1));
                partNode->addChild(sprite, 1);
            }
        }

        m_spriteParts->addObject(partNode);
        normalImage->addChild(partNode);
    }

    normalImage->addChild(glowNode, -1);

    if (animation->objectForKey("singleFrame")) updateComplexSprite(static_cast<CCString*>(object));
    else {
        m_animation = static_cast<CCAnimation*>(object);
        m_looped = animation->valueForKey("looped")->boolValue();
        m_elapsed = 0.0f;
        scheduleUpdate();
    }
}

void LazyIcon::updateComplexSprite(CCString* frame) {
    for (auto spritePart : CCArrayExt<CCSprite*>(m_spriteParts)) {
        spritePart->setVisible(false);
    }
    auto normalImage = getNormalImage();
    for (auto description : CCArrayExt<SpriteDescription*>(MoreIconsAPI::getAnimateFrameCache()->spriteFrameByName(frame->getCString()))) {
        auto spritePart = static_cast<CCSpritePlus*>(m_spriteParts->objectAtIndex(description->m_tag));
        spritePart->setPosition(description->m_position);
        spritePart->setScaleX(description->m_scale.x);
        spritePart->setScaleY(description->m_scale.y);
        spritePart->setRotation(description->m_rotation);
        spritePart->setFlipX(description->m_flipped.x != 0.0f);
        spritePart->setFlipY(description->m_flipped.y != 0.0f);
        if (spritePart->getZOrder() != description->m_zValue) normalImage->reorderChild(spritePart, description->m_zValue);
        spritePart->setVisible(true);
    }
}

void LazyIcon::update(float dt) {
    m_elapsed += dt;
    auto interval = m_elapsed / (m_animation->m_fDelayPerUnit * m_animation->m_fTotalDelayUnits);
    if (!m_looped && interval >= 1.0f) {
        m_elapsed = 0.0f;
        return unscheduleUpdate();
    }

    auto frames = m_animation->m_pFrames;
    updateComplexSprite(static_cast<CCString*>(static_cast<CCObject*>(static_cast<CCAnimationFrame*>(
        frames->objectAtIndex((int)(fmodf(interval, 1.0f) * frames->count())))->m_pSpriteFrame)));
}

void LazyIcon::createIcon() {
    setEnabled(true);
    if (m_error.empty()) {
        if (m_loadingSprite) {
            m_loadingSprite->removeFromParent();
            m_loadingSprite = nullptr;
        }
        if (m_type == IconType::Robot || m_type == IconType::Spider) createComplexIcon();
        else if (m_type <= IconType::Jetpack) createSimpleIcon();
        else if (m_info && m_type == IconType::Special) {
            auto normalImage = getNormalImage();
            auto square = MoreIconsAPI::customTrail(m_info->textures[0]);
            square->setID("player-square");
            normalImage->addChild(square, 0);
        }
    }
    else {
        m_loadingSprite->stopAllActions();
        m_loadingSprite->setDisplayFrame(MoreIconsAPI::getSpriteFrameCache()->spriteFrameByName("GJ_deleteIcon_001.png"));
        m_loadingSprite->setScale(1.1f);
        m_loadingSprite->setRotation(0.0f);
    }
}

void LazyIcon::activate() {
    if (!m_bEnabled) return;
    CCMenuItemSpriteExtra::activate();
    if (m_error.empty()) ViewIconPopup::create(m_type, m_id, m_info)->show();
    else FLAlertLayer::create("Error", m_error, "OK")->show();
}

void LazyIcon::visit() {
    CCNode::visit();

    if (!m_bVisible || m_visited) return;

    m_visited = true;

    ThreadPool::get().pushTask([selfref = WeakRef(this), texture = m_texture, sheet = m_sheet, name = m_info ? m_info->name : "", type = m_type] {
        auto image = MoreIconsAPI::createFrames(texture, sheet, name, type);
        queueInMainThread([selfref = std::move(selfref), image = std::move(image)] mutable {
            if (auto self = selfref.lock()) {
                if (image.isOk()) MoreIconsAPI::addFrames(image.unwrap(), self->m_frames);
                else if (image.isErr()) {
                    self->m_error = std::move(image).unwrapErr();
                    log::error("{}: {}", self->m_name, self->m_error);
                }
                self->createIcon();
            }
        });
    });
}

LazyIcon::~LazyIcon() {
    auto spriteFrameCache = MoreIconsAPI::getSpriteFrameCache();
    for (auto& frame : m_frames) {
        spriteFrameCache->removeSpriteFrameByName(frame.c_str());
    }

    if (!m_texture.empty()) MoreIconsAPI::getTextureCache()->removeTextureForKey(m_texture.c_str());
}

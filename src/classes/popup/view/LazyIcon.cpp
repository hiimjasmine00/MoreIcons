#include "LazyIcon.hpp"
#include "../EditIconPopup.hpp"
#include "../../misc/ThreadPool.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/binding/CCAnimateFrameCache.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SpriteDescription.hpp>

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
    normalImage->setContentSize(CCSpriteFrameCache::get()->spriteFrameByName("playerSquare_001.png")->getOriginalSize());
    if (!CCMenuItemSpriteExtra::init(normalImage, nullptr, nullptr, nullptr)) return false;

    auto gameManager = GameManager::get();

    m_type = type;
    m_id = id;
    m_info = info;
    m_name = info ? info->name : fmt::format("{}{:02}", MoreIcons::prefixes[(int)gameManager->iconTypeToUnlockType(type)], id);
    setID(m_name);

    if (type == IconType::Special && !info) {
        m_visited = true;
        auto playerSpecial = CCSprite::createWithSpriteFrameName(fmt::format("player_special_{:02}_001.png", id).c_str());
        playerSpecial->setPosition(normalImage->getContentSize() / 2.0f);
        playerSpecial->setID("player-special");
        normalImage->addChild(playerSpecial);
        return true;
    }

    if (info) {
        m_texture = info->folderName.empty() ? info->textures[0] : info->folderName;
        m_sheet = info->sheetName;
    }
    else {
        std::string fullName = gameManager->sheetNameForIcon(id, (int)type);
        auto fileUtils = CCFileUtils::get();
        m_texture = fileUtils->fullPathForFilename(fmt::format("{}.png", fullName).c_str(), false);
        m_sheet = fileUtils->fullPathForFilename(fmt::format("{}.plist", fullName).c_str(), false);
    }

    if (type <= IconType::Jetpack || (type == IconType::Special && info)) {
        if (CCTextureCache::get()->textureForKey(m_texture.c_str())) {
            m_visited = true;
            m_texture = "";
            createIcon("", {});
        }
        else {
            setEnabled(false);
            m_loadingSprite = CCSprite::create("loadingCircle.png");
            m_loadingSprite->setPosition(normalImage->getContentSize() / 2.0f);
            m_loadingSprite->setScale(0.4f);
            m_loadingSprite->setBlendFunc({ GL_ONE, GL_ONE });
            m_loadingSprite->runAction(CCRepeatForever::create(CCRotateBy::create(1.0f, 360.0f)));
            normalImage->addChild(m_loadingSprite);
        }
    }

    return true;
}

void LazyIcon::createSimpleIcon() {
    auto iconName = fmt::format("{}{}", m_info ? GEODE_MOD_ID "/" : "", m_name);
    auto spriteFrameCache = CCSpriteFrameCache::get();

    auto primaryFrame = spriteFrameCache->spriteFrameByName(fmt::format("{}_001.png", iconName).c_str());
    if (primaryFrame && primaryFrame->getTag() == 105871529) primaryFrame = nullptr;

    auto secondaryFrame = spriteFrameCache->spriteFrameByName(fmt::format("{}_2_001.png", iconName).c_str());
    if (secondaryFrame && secondaryFrame->getTag() == 105871529) secondaryFrame = nullptr;

    auto tertiaryFrame = m_type == IconType::Ufo ? spriteFrameCache->spriteFrameByName(fmt::format("{}_3_001.png", iconName).c_str()) : nullptr;
    if (tertiaryFrame && tertiaryFrame->getTag() == 105871529) tertiaryFrame = nullptr;

    auto glowFrame = spriteFrameCache->spriteFrameByName(fmt::format("{}_glow_001.png", iconName).c_str());
    if (glowFrame && glowFrame->getTag() == 105871529) glowFrame = nullptr;

    auto extraFrame = spriteFrameCache->spriteFrameByName(fmt::format("{}_extra_001.png", iconName).c_str());
    if (extraFrame && extraFrame->getTag() == 105871529) extraFrame = nullptr;

    auto normalImage = getNormalImage();
    auto center = normalImage->getContentSize() / 2.0f - CCPoint { 0.0f, (m_type == IconType::Ufo) * 7.0f };

    if (primaryFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(primaryFrame);
        sprite->setPosition(center);
        sprite->setID("primary-sprite");
        normalImage->addChild(sprite, 0);
    }

    if (secondaryFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(secondaryFrame);
        sprite->setPosition(center);
        sprite->setID("secondary-sprite");
        normalImage->addChild(sprite, -1);
    }

    if (tertiaryFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(tertiaryFrame);
        sprite->setPosition(center);
        sprite->setID("tertiary-sprite");
        normalImage->addChild(sprite, -2);
    }

    if (glowFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(glowFrame);
        sprite->setPosition(center);
        sprite->setID("glow-sprite");
        normalImage->addChild(sprite, -3);
    }

    if (extraFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(extraFrame);
        sprite->setPosition(center);
        sprite->setID("extra-sprite");
        normalImage->addChild(sprite, 1);
    }
}

void LazyIcon::createComplexIcon() {
    auto spider = m_type == IconType::Spider;
    auto afc = CCAnimateFrameCache::get();
    auto type = spider ? "Spider" : "Robot";
    auto idleFrames = afc->spriteFrameByName(fmt::format("{}_idle_001.png", type).c_str());
    auto iconName = fmt::format("{}{}", m_info ? GEODE_MOD_ID "/" : "", m_name);
    auto sfc = CCSpriteFrameCache::get();
    auto normalImage = getNormalImage();
    auto center = normalImage->getContentSize() / 2.0f;

    auto glowNode = CCNode::create();
    glowNode->setPosition(center);
    glowNode->setAnchorPoint({ 0.5f, 0.5f });
    glowNode->setID("glow-node");

    auto usedTextures = static_cast<CCDictionary*>(afc->addSpriteFramesWithFile(
        fmt::format("{}_AnimDesc.plist", type).c_str())->objectForKey("usedTextures"));
    for (int i = 0; i < usedTextures->count(); i++) {
        auto usedTexture = static_cast<CCDictionary*>(usedTextures->objectForKey(fmt::format("texture_{}", i)));
        if (!usedTexture) continue;

        auto textureString = usedTexture->valueForKey("texture");
        auto texture = textureString ? textureString->m_sString : "";
        auto index = texture.size() >= spider + 11 ? numFromString<int>(std::string(texture, spider + 9, 2)).unwrapOr(0) : 0;

        if (index <= 0) continue;

        auto primaryFrame = sfc->spriteFrameByName(fmt::format("{}_{:02}_001.png", iconName, index).c_str());
        if (primaryFrame && primaryFrame->getTag() == 105871529) primaryFrame = nullptr;

        auto secondaryFrame = sfc->spriteFrameByName(fmt::format("{}_{:02}_2_001.png", iconName, index).c_str());
        if (secondaryFrame && secondaryFrame->getTag() == 105871529) secondaryFrame = nullptr;

        auto glowFrame = sfc->spriteFrameByName(fmt::format("{}_{:02}_glow_001.png", iconName, index).c_str());
        if (glowFrame && glowFrame->getTag() == 105871529) glowFrame = nullptr;

        auto extraFrame = index == 1 ? sfc->spriteFrameByName(fmt::format("{}_{:02}_extra_001.png", iconName, index).c_str()) : nullptr;
        if (extraFrame && extraFrame->getTag() == 105871529) extraFrame = nullptr;

        auto customIDString = usedTexture->valueForKey("customID");
        auto customID = customIDString ? customIDString->m_sString : "";
        uint8_t spriteColor = customID == "back01" || customID == "back02" || customID == "back03" ? 178 - (spider * 51) : 255;
        ccColor3B spriteColor3B = { spriteColor, spriteColor, spriteColor };

        auto description = static_cast<SpriteDescription*>(idleFrames->objectAtIndex(i));
        auto& position = description->m_position;
        auto rotation = description->m_rotation;
        auto scaleX = description->m_scale.x;
        auto scaleY = description->m_scale.y;
        auto flipX = description->m_flipped.x != 0.0f;
        auto flipY = description->m_flipped.y != 0.0f;

        auto partNode = CCNode::create();
        partNode->setPosition(position + center);
        partNode->setRotation(rotation);
        partNode->setScaleX(scaleX);
        partNode->setScaleY(scaleY);
        partNode->setAnchorPoint({ 0.5f, 0.5f });
        partNode->setID(fmt::format("part-node-{}", i + 1));

        if (primaryFrame) {
            auto sprite = CCSprite::createWithSpriteFrame(primaryFrame);
            sprite->setFlipX(flipX);
            sprite->setFlipY(flipY);
            sprite->setColor(spriteColor3B);
            sprite->setID(fmt::format("primary-sprite-{}", i + 1));
            partNode->addChild(sprite, 0);
        }

        if (secondaryFrame) {
            auto sprite = CCSprite::createWithSpriteFrame(secondaryFrame);
            sprite->setFlipX(flipX);
            sprite->setFlipY(flipY);
            sprite->setColor(spriteColor3B);
            sprite->setID(fmt::format("secondary-sprite-{}", i + 1));
            partNode->addChild(sprite, -1);
        }

        if (glowFrame) {
            auto sprite = CCSprite::createWithSpriteFrame(glowFrame);
            sprite->setPosition(position);
            sprite->setRotation(rotation);
            sprite->setScaleX(scaleX);
            sprite->setScaleY(scaleY);
            sprite->setFlipX(flipX);
            sprite->setFlipY(flipY);
            sprite->setID(fmt::format("glow-sprite-{}", i + 1));
            glowNode->addChild(sprite, -1);
        }

        if (extraFrame) {
            auto sprite = CCSprite::createWithSpriteFrame(extraFrame);
            sprite->setFlipX(flipX);
            sprite->setFlipY(flipY);
            sprite->setID(fmt::format("extra-sprite-{}", i + 1));
            partNode->addChild(sprite, 1);
        }

        normalImage->addChild(partNode, description->m_zValue);
    }

    normalImage->addChild(glowNode, -1);
}

void LazyIcon::createIcon(const std::string& err, const std::vector<std::string>& frames) {
    m_error = err;
    setEnabled(true);
    if (err.empty()) {
        m_frames = frames;
        if (m_loadingSprite) {
            m_loadingSprite->removeFromParent();
            m_loadingSprite = nullptr;
        }
        if (m_type == IconType::Robot || m_type == IconType::Spider) createComplexIcon();
        else if (m_type <= IconType::Jetpack) createSimpleIcon();
        else if (m_info && m_type == IconType::Special) {
            auto normalImage = getNormalImage();

            auto square = CCSprite::createWithSpriteFrameName("playerSquare_001.png");
            square->setPosition(normalImage->getContentSize() / 2.0f);
            square->setColor({ 150, 150, 150 });
            square->setID("player-square");
            normalImage->addChild(square, 0);

            auto streak = CCSprite::create(m_info->textures[0].c_str());
            limitNodeHeight(streak, 27.0f, 999.0f, 0.001f);
            streak->setRotation(-90.0f);
            streak->setPosition(normalImage->getContentSize() / 2);
            streak->setID("player-streak");
            normalImage->addChild(streak, 1);
        }
    }
    else {
        log::error("{}: {}", m_name, err);
        m_loadingSprite->stopAllActions();
        m_loadingSprite->setDisplayFrame(CCSpriteFrameCache::get()->spriteFrameByName("GJ_deleteIcon_001.png"));
        m_loadingSprite->setScale(1.1f);
        m_loadingSprite->setRotation(0.0f);
    }
}

void LazyIcon::activate() {
    if (!m_bEnabled) return;
    CCMenuItemSpriteExtra::activate();
    if (m_error.empty()) EditIconPopup::create(m_type, m_id, m_info ? m_info->name : "", true)->show();
    else FLAlertLayer::create("Error", m_error, "OK")->show();
}

void LazyIcon::visit() {
    CCNode::visit();

    if (!m_bVisible || m_visited) return;

    m_visited = true;

    if (!m_info || !m_info->sheetName.empty() || m_type == IconType::Special || !m_info->folderName.empty()) {
        ThreadPool::get().pushTask([
            selfref = WeakRef(this),
            sheet = m_sheet.empty() ? m_info ? m_info->folderName : "" : m_sheet,
            name = m_info ? m_info->name : "",
            type = m_type,
            textures = m_info ? m_info->textures : std::vector<std::string>({ m_texture })
        ] {
            auto image = MoreIconsAPI::createFrames(textures, sheet, name, type);
            queueInMainThread([selfref = std::move(selfref), image = std::move(image)] {
                auto self = selfref.lock();
                if (image.isErr()) {
                    if (self) self->createIcon(image.unwrapErr(), {});
                    return;
                }

                auto result = image.unwrap();
                if (self) self->createIcon("", MoreIconsAPI::addFrames(result));
                else {
                    result.texture->release();
                    result.frames->release();
                }
            });
        });
    }
}

LazyIcon::~LazyIcon() {
    auto spriteFrameCache = CCSpriteFrameCache::get();
    for (auto& frame : m_frames) {
        spriteFrameCache->m_pSpriteFrames->removeObjectForKey(frame);
    }

    if (!m_texture.empty()) CCTextureCache::get()->m_pTextures->removeObjectForKey(m_texture);
}

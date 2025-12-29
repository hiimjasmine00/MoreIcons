#include "LazyIcon.hpp"
#include "ThreadPool.hpp"
#include "../../MoreIcons.hpp"
#include "../../utils/Filesystem.hpp"
#include "../../utils/Get.hpp"
#include "../../utils/Icons.hpp"
#include "../../utils/Load.hpp"
#include <fast_float/fast_float.h>
#include <Geode/binding/CCAnimateFrameCache.hpp>
#include <Geode/binding/CCSpritePlus.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/ObjectManager.hpp>
#include <Geode/binding/SpriteDescription.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Loader.hpp>
#include <jasmine/mod.hpp>

using namespace geode::prelude;
using namespace jasmine::mod;

LazyIcon* LazyIcon::create(IconType type, int id, IconInfo* info, std::string_view suffix, std23::move_only_function<void()> callback) {
    auto ret = new LazyIcon();
    if (ret->init(type, id, info, suffix, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LazyIcon::init(IconType type, int id, IconInfo* info, std::string_view suffix, std23::move_only_function<void()> callback) {
    auto normalImage = CCNode::create();
    if (!CCMenuItemSpriteExtra::init(normalImage, nullptr, nullptr, nullptr)) return false;

    setContentSize({ 30.0f, 30.0f });
    normalImage->setPosition({ 15.0f, 15.0f });

    m_callback = std::move(callback);
    m_suffix = suffix;
    m_type = type;
    m_id = id;
    m_info = info;
    m_name = info ? fmt::format("{}"_spr, info->getName()) : MoreIcons::getIconName(id, type);
    setID(m_name);

    if (type >= IconType::DeathEffect && !info) {
        m_visited = true;
        auto playerSpecial = CCSprite::createWithSpriteFrameName(
            type == IconType::DeathEffect ? fmt::format("explosionIcon_{:02}_001.png", id).c_str() :
            type == IconType::Special ? fmt::format("player_special_{:02}_001.png", id).c_str() :
            type == IconType::ShipFire ? fmt::format("shipfireIcon_{:02}_001.png", id).c_str() : "cc_2x2_white_image"
        );
        playerSpecial->setID("player-special");
        normalImage->addChild(playerSpecial);
        return true;
    }

    if (info) {
        if (type <= IconType::Jetpack) {
            m_texture = info->getTextureString();
            m_sheet = info->getSheetString();
        }
        else {
            m_visited = true;
            auto playerSpecial = MoreIcons::customIcon(m_info);
            playerSpecial->setID("player-special");
            normalImage->addChild(playerSpecial);
            return true;
        }
    }
    else {
        auto [texture, sheet] = MoreIcons::getIconPaths(id, type);
        m_texture = std::move(texture);
        m_sheet = std::move(sheet);
    }

    if (type <= IconType::Jetpack || (type >= IconType::DeathEffect && info)) {
        if (Get::TextureCache()->textureForKey(m_texture.c_str())) {
            m_visited = true;
            m_texture.clear();
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
    auto primaryFrame = Icons::getFrame("{}_001.png", m_name);
    auto secondaryFrame = Icons::getFrame("{}_2_001.png", m_name);
    auto tertiaryFrame = m_type == IconType::Ufo ? Icons::getFrame("{}_3_001.png", m_name) : nullptr;
    auto glowFrame = Icons::getFrame("{}_glow_001.png", m_name);
    auto extraFrame = Icons::getFrame("{}_extra_001.png", m_name);
    auto normalImage = getNormalImage();
    auto yOffset = m_type == IconType::Ufo ? -7.0f : 0.0f;
    auto scale = m_type == IconType::Ball ? 0.9f : 1.0f;

    if (primaryFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(primaryFrame);
        sprite->setPositionY(yOffset);
        sprite->setScale(scale);
        sprite->setID("sprite_001");
        normalImage->addChild(sprite, 0);
    }

    if (secondaryFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(secondaryFrame);
        sprite->setPositionY(yOffset);
        sprite->setScale(scale);
        sprite->setID("sprite_2_001");
        normalImage->addChild(sprite, -1);
    }

    if (tertiaryFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(tertiaryFrame);
        sprite->setPositionY(yOffset);
        sprite->setScale(scale);
        sprite->setID("sprite_3_001");
        normalImage->addChild(sprite, -2);
    }

    if (glowFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(glowFrame);
        sprite->setPositionY(yOffset);
        sprite->setScale(scale);
        sprite->setID("sprite_glow_001");
        normalImage->addChild(sprite, -3);
    }

    if (extraFrame) {
        auto sprite = CCSprite::createWithSpriteFrame(extraFrame);
        sprite->setPositionY(yOffset);
        sprite->setScale(scale);
        sprite->setID("sprite_extra_001");
        normalImage->addChild(sprite, 1);
    }
}
matjson::Value getJSON(Filesystem::PathView filename, std::string_view filenameNarrow) {
    if (auto def = Load::readPlist(dirs::getResourcesDir() / filename)) {
        return std::move(def).unwrap();
    }
    else {
        log::error("Failed to load {}: {}", filenameNarrow, def.unwrapErr());
        return nullptr;
    }
}

const matjson::Value definitions = [] {
    auto json = getJSON(L("objectDefinitions.plist"), "objectDefinitions.plist");
    for (auto& value : json) {
        if (auto keyOpt = value.getKey()) {
            auto key = std::move(keyOpt).value();
            if (key == "Robot" || key == "Spider") {
                if (auto animDesc = value["animDesc"].asString()) {
                    auto filename = std::move(animDesc).unwrap();
                    value["animDesc"] = getJSON(Filesystem::strWide(filename), filename);
                }
            }
            else json.erase(key);
        }
    }
    return json;
}();

std::vector<SpriteDefinition> parseDefinition(const matjson::Value& definition) {
    std::vector<SpriteDefinition> definitions;
    for (size_t i = 0; i < definition.size(); i++) {
        auto& value = definition[fmt::format("sprite_{}", i)];
        if (!value.isObject()) continue;

        auto& def = definitions.emplace_back();
        def.position = CCPointFromString(value["position"].asString().unwrapOrDefault().c_str());
        def.scale = CCPointFromString(value["scale"].asString().unwrapOrDefault().c_str());
        def.flipped = CCPointFromString(value["flipped"].asString().unwrapOrDefault().c_str());
        def.rotation = value["rotation"].asDouble().unwrapOrDefault();
        def.zValue = value["zValue"].asInt().unwrapOrDefault();
        def.tag = value["tag"].asInt().unwrapOrDefault();
    }
    return definitions;
}

void LazyIcon::createComplexIcon() {
    auto spider = m_type == IconType::Spider;

    std::string_view anim = "idle";
    std::string_view key = spider ? "Spider" : "Robot";

    auto& definition = definitions[key];
    if (!definition.isObject()) return;

    auto& animations = definition["animations"];
    if (!animations.isObject()) return;

    auto& animation = animations[anim];
    if (!animation.isObject()) return;

    auto& animDesc = definition["animDesc"];
    if (!animDesc.isObject()) return;

    auto& container = animDesc["animationContainer"];
    if (!container.isObject()) return;

    auto& usedTextures = animDesc["usedTextures"];
    if (!usedTextures.isObject()) return;

    auto normalImage = getNormalImage();

    auto glowNode = CCNode::create();
    glowNode->setAnchorPoint({ 0.5f, 0.5f });
    glowNode->setID("glow-node");

    for (size_t i = 0; i < usedTextures.size(); i++) {
        auto& usedTexture = usedTextures[fmt::format("texture_{}", i)];
        if (!usedTexture.isObject()) continue;

        auto texture = usedTexture["texture"].asString().unwrapOrDefault();
        if ((spider && texture.size() < 12) || (!spider && texture.size() < 11)) continue;

        auto index = 0;
        auto start = texture.data() + (spider ? 10 : 9);
        fast_float::from_chars(start, start + 2, index);
        if (index <= 0) continue;

        auto customID = usedTexture["customID"].asString().unwrapOrDefault();
        ccColor3B spriteColor;
        if (customID == "back01" || customID == "back02" || customID == "back03") {
            spriteColor = spider ? ccColor3B { 127, 127, 127 } : ccColor3B { 178, 178, 178 };
        }
        else {
            spriteColor = { 255, 255, 255 };
        }

        auto partNode = new CCSpritePlus();
        partNode->init();
        partNode->autorelease();
        partNode->m_propagateScaleChanges = true;
        partNode->m_propagateFlipChanges = true;
        partNode->setID(fmt::format("sprite_{:02}", index));

        if (auto primaryFrame = Icons::getFrame("{}_{:02}_001.png", m_name, index)) {
            auto sprite = CCSprite::createWithSpriteFrame(primaryFrame);
            sprite->setColor(spriteColor);
            sprite->setID(fmt::format("sprite_{:02}_001", index));
            partNode->addChild(sprite, 0);
        }

        if (auto secondaryFrame = Icons::getFrame("{}_{:02}_2_001.png", m_name, index)) {
            auto sprite = CCSprite::createWithSpriteFrame(secondaryFrame);
            sprite->setColor(spriteColor);
            sprite->setID(fmt::format("sprite_{:02}_2_001", index));
            partNode->addChild(sprite, -1);
        }

        if (auto glowFrame = Icons::getFrame("{}_{:02}_glow_001.png", m_name, index)) {
            auto sprite = CCSprite::createWithSpriteFrame(glowFrame);
            sprite->setID(fmt::format("sprite_{:02}_glow_001", index));
            glowNode->addChild(sprite, -1);
            partNode->addFollower(sprite);
        }

        if (index == 1) {
            if (auto extraFrame = Icons::getFrame("{}_01_extra_001.png", m_name)) {
                auto sprite = CCSprite::createWithSpriteFrame(extraFrame);
                sprite->setID("sprite_01_extra_001");
                partNode->addChild(sprite, 1);
            }
        }

        m_spriteParts.emplace_back(partNode);
        normalImage->addChild(partNode);
    }

    normalImage->addChild(glowNode, -1);

    if (auto singleFrame = animation.get("singleFrame")) {
        updateComplexSprite(parseDefinition(container[singleFrame.unwrap().asString().unwrapOrDefault()]));
    }
    else {
        auto frames = animation["frames"].asInt().unwrapOrDefault();
        m_divisor = std::max(0.01, animation["delay"].asDouble().unwrapOrDefault() * frames);
        auto prefix = fmt::format("{}_{}_", key, anim);
        for (int i = 1; i <= frames; i++) {
            m_definitions.push_back(parseDefinition(container[fmt::format("{}{:03}.png", prefix, i)]));
        }
        auto looped = animation["looped"].asString().unwrapOrDefault();
        m_looped = !looped.empty() && looped != "0" && looped != "false";
        m_elapsed = 0.0f;
        scheduleUpdate();
    }
}

void LazyIcon::updateComplexSprite(const std::vector<SpriteDefinition>& definitions) {
    for (auto& spritePart : m_spriteParts) {
        spritePart->setVisible(false);
    }

    for (auto& definition : definitions) {
        auto spritePart = m_spriteParts[definition.tag].data();
        spritePart->setPosition(definition.position);
        spritePart->setScaleX(definition.scale.x);
        spritePart->setScaleY(definition.scale.y);
        spritePart->setRotation(definition.rotation);
        spritePart->setFlipX(definition.flipped.x != 0.0f);
        spritePart->setFlipY(definition.flipped.y != 0.0f);
        if (spritePart->getZOrder() != definition.zValue) {
            spritePart->getParent()->reorderChild(spritePart, definition.zValue);
        }
        spritePart->setVisible(true);
    }
}

void LazyIcon::update(float dt) {
    m_elapsed += dt;
    auto interval = m_elapsed / m_divisor;
    if (!m_looped && interval >= 1.0) {
        m_elapsed = 0.0;
        return unscheduleUpdate();
    }

    updateComplexSprite(m_definitions[fmod(interval, 1.0) * m_definitions.size()]);
}

void LazyIcon::createIcon() {
    setEnabled(true);
    auto spriteFrameCache = Get::SpriteFrameCache();
    if (m_error.empty()) {
        if (m_loadingSprite) {
            m_loadingSprite->removeFromParent();
            m_loadingSprite = nullptr;
        }
        if (!m_suffix.empty()) {
            auto spriteFrame = Icons::getFrame("{}{}.png", m_name, m_suffix);
            if (!spriteFrame) spriteFrame = spriteFrameCache->spriteFrameByName("GJ_deleteIcon_001.png");
            auto sprite = CCSprite::createWithSpriteFrame(spriteFrame);
            sprite->setID("custom-sprite");
            getNormalImage()->addChild(sprite);
        }
        else if (m_type == IconType::Robot || m_type == IconType::Spider) createComplexIcon();
        else if (m_type <= IconType::Jetpack) createSimpleIcon();
        else if (m_info && m_type >= IconType::DeathEffect) {
            auto playerSpecial = MoreIcons::customIcon(m_info);
            playerSpecial->setID("player-special");
            getNormalImage()->addChild(playerSpecial);
        }
    }
    else {
        m_loadingSprite->stopAllActions();
        m_loadingSprite->setDisplayFrame(spriteFrameCache->spriteFrameByName("GJ_deleteIcon_001.png"));
        m_loadingSprite->setScale(1.1f);
        m_loadingSprite->setRotation(0.0f);
    }
}

void LazyIcon::activate() {
    if (!m_bEnabled) return;
    CCMenuItemSpriteExtra::activate();
    if (m_error.empty()) {
        if (m_callback) m_callback();
    }
    else FLAlertLayer::create("Error", m_error, "OK")->show();
}

void LazyIcon::visit() {
    CCNode::visit();

    if (!m_bVisible || m_visited) return;

    m_visited = true;

    ThreadPool::get().pushTask([
        selfref = WeakRef(this), texture = Filesystem::strPath(m_texture), sheet = Filesystem::strPath(m_sheet),
        name = m_info ? m_info->getName() : std::string(), type = m_type,
        frameName = m_suffix.empty() ? std::string() : fmt::format("{}{}.png", m_name, m_suffix)
    ] {
        auto image = Load::createFrames(texture, sheet, name, type);
        queueInMainThread([selfref = std::move(selfref), image = std::move(image), frameName = std::move(frameName)] mutable {
            if (auto self = selfref.lock()) {
                if (image.isOk()) {
                    Load::addFrames(image.unwrap(), self->m_frames, frameName);
                }
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
    auto spriteFrameCache = Get::SpriteFrameCache();
    for (auto& frame : m_frames) {
        spriteFrameCache->removeSpriteFrameByName(frame.c_str());
    }

    if (!m_texture.empty()) Get::TextureCache()->removeTextureForKey(m_texture.c_str());
}

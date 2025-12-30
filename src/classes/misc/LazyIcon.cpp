#include "LazyIcon.hpp"
#include "SimpleIcon.hpp"
#include "ThreadPool.hpp"
#include "../../MoreIcons.hpp"
#include "../../utils/Filesystem.hpp"
#include "../../utils/Get.hpp"
#include "../../utils/Icons.hpp"
#include "../../utils/Load.hpp"
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/utils/cocos.hpp>
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
            m_loadingSprite->runAction(CCRepeatForever::create(CCRotateBy::create(1.0f, 360.0f)));
            normalImage->addChild(m_loadingSprite);
        }
    }

    return true;
}

void LazyIcon::createIcon() {
    setEnabled(true);
    auto spriteFrameCache = Get::SpriteFrameCache();
    auto normalImage = getNormalImage();
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
            normalImage->addChild(sprite);
        }
        else if (m_type <= IconType::Jetpack) {
            auto icon = SimpleIcon::create(m_type, m_name);
            icon->setID(fmt::format("{}-icon", m_name));
            normalImage->addChild(icon);
        }
        else if (m_info && m_type >= IconType::DeathEffect) {
            auto playerSpecial = MoreIcons::customIcon(m_info);
            playerSpecial->setID("player-special");
            normalImage->addChild(playerSpecial);
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

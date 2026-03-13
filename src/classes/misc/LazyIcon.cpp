#include "LazyIcon.hpp"
#include "SimpleIcon.hpp"
#include "../../MoreIcons.hpp"
#include "../../utils/Filesystem.hpp"
#include "../../utils/Get.hpp"
#include "../../utils/Icons.hpp"
#include "../../utils/Load.hpp"
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/utils/async.hpp>
#include <jasmine/mod.hpp>

using namespace geode::prelude;
using namespace jasmine::mod;

LazyIcon* LazyIcon::create(IconType type, int id, IconInfo* info, std::string_view suffix, Function<void()> callback) {
    auto ret = new LazyIcon();
    if (ret->init(type, id, info, suffix, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LazyIcon::init(IconType type, int id, IconInfo* info, std::string_view suffix, Function<void()> callback) {
    setAnchorPoint({ 0.5f, 0.5f });
    setContentSize({ 30.0f, 30.0f });

    m_callback = std::move(callback);
    m_type = type;
    m_info = info;
    m_name = info ? fmt::format("{}"_spr, info->getName()) : MoreIcons::getIconName(id, type);
    if (!suffix.empty()) m_frameName = fmt::format("{}{}.png", m_name, suffix);
    setID(m_name);

    if (type >= IconType::DeathEffect) {
        m_visited = true;
        setEnabled(true);
        if (suffix.empty()) {
            setMainNode(info ? MoreIcons::customIcon(info) : CCSprite::createWithSpriteFrameName(
                type == IconType::DeathEffect ? fmt::format("explosionIcon_{:02}_001.png", id).c_str() :
                type == IconType::Special ? fmt::format("player_special_{:02}_001.png", id).c_str() :
                type == IconType::ShipFire ? fmt::format("shipfireIcon_{:02}_001.png", id).c_str() : "cc_2x2_white_image"
            ));
        }
        else if (type == IconType::DeathEffect) {
            setMainNode(CCSprite::createWithSpriteFrameName(fmt::format("playerExplosion_{:02}{}.png", id, suffix).c_str()));
        }
        else if (type == IconType::ShipFire) {
            auto firePath = MoreIcons::getFirePath(info, id, suffix);
            auto sprite = CCSprite::create(Filesystem::strNarrow(firePath.native()).data());
            if (!sprite || sprite->getUserObject("geode.texture-loader/fallback")) {
                m_error = fmt::format("Failed to load {}", firePath);
                log::error("{}: {}", m_name, m_error);
                setMainNode(CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png"));
            }
            else {
                limitNodeHeight(sprite, 30.0f, 1.0f, 0.0f);
                sprite->setPosition({ 15.0f, 15.0f });
                setMainNode(sprite);
            }
        }
        return true;
    }

    MoreIcons::getIconPaths(info, id, type, m_texture, m_sheet);

    if (Get::textureCache->textureForKey(Filesystem::strNarrow(m_texture.native()).data())) {
        m_visited = true;
        createIcon();
    }
    else {
        auto loadingSprite = CCSprite::create("loadingCircle.png");
        loadingSprite->setScale(0.4f);
        loadingSprite->runAction(CCRepeatForever::create(CCRotateBy::create(1.0f, 360.0f)));
        setMainNode(loadingSprite);
    }

    return true;
}

void LazyIcon::createIcon() {
    setEnabled(true);
    if (m_error.empty()) {
        if (!m_frameName.empty()) {
            auto spriteFrame = Icons::getFrame(m_frameName);
            if (!spriteFrame) spriteFrame = Get::spriteFrameCache->spriteFrameByName("GJ_deleteIcon_001.png");
            setMainNode(CCSprite::createWithSpriteFrame(spriteFrame));
        }
        else {
            setMainNode(SimpleIcon::create(m_type, m_name));
        }
    }
    else {
        setMainNode(CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png"));
    }
}

void LazyIcon::setMainNode(CCNode* image) {
    if (m_mainNode) {
        m_mainNode->removeFromParent();
    }
    if (image) {
        image->setPosition({ 15.0f, 15.0f });
        image->setTag(1);
        addChild(image);
    }
    m_mainNode = image;
}

void LazyIcon::selected() {
    CCMenuItem::selected();
    stopActionByTag(0);
    auto action = CCEaseBounceOut::create(CCScaleTo::create(0.3f, 1.26f));
    action->setTag(0);
    runAction(action);
}

void LazyIcon::unselected() {
    CCMenuItem::unselected();
    stopActionByTag(0);
    auto action = CCEaseBounceOut::create(CCScaleTo::create(0.4f, 1.0f));
    action->setTag(0);
    runAction(action);
}

void LazyIcon::activate() {
    CCMenuItem::activate();
    stopActionByTag(0);
    setScale(1.0f);
    if (m_error.empty()) m_callback();
    else FLAlertLayer::create("Error", m_error, "OK")->show();
}

void LazyIcon::visit() {
    CCNode::visit();

    if (!m_bVisible || m_visited) return;

    m_visited = true;

    runtime().spawnBlocking<void>([
        selfref = WeakRef(this), texture = m_texture, sheet = m_sheet,
        name = m_info ? m_info->getName() : std::string_view(), type = m_type, frameName = m_frameName
    ] {
        auto image = Load::createFrames(texture, sheet, name, type, frameName);
        queueInMainThread([selfref = std::move(selfref), image = std::move(image)] mutable {
            if (auto self = selfref.lock()) {
                if (image.isOk()) {
                    auto result = std::move(image).unwrap();
                    Load::addFrames(result, self->m_frames);
                    self->m_key = std::move(result.name);
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
    if (!m_frames.empty()) {
        for (auto& frame : m_frames) {
            Get::spriteFrameCache->removeSpriteFrameByName(frame.c_str());
        }
    }
    if (!m_key.empty()) {
        Get::textureCache->removeTextureForKey(m_key.c_str());
    }
}

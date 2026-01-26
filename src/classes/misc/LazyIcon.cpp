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
        setNormalImage(info ? MoreIcons::customIcon(info) : CCSprite::createWithSpriteFrameName(
            type == IconType::DeathEffect ? fmt::format("explosionIcon_{:02}_001.png", id).c_str() :
            type == IconType::Special ? fmt::format("player_special_{:02}_001.png", id).c_str() :
            type == IconType::ShipFire ? fmt::format("shipfireIcon_{:02}_001.png", id).c_str() : "cc_2x2_white_image"
        ));
        return true;
    }

    MoreIcons::getIconPaths(info, id, type, m_texture, m_sheet);

    if (Get::TextureCache()->textureForKey(Filesystem::strNarrow(m_texture.native()).data())) {
        m_visited = true;
        createIcon();
    }
    else {
        auto loadingSprite = CCSprite::create("loadingCircle.png");
        loadingSprite->setScale(0.4f);
        loadingSprite->runAction(CCRepeatForever::create(CCRotateBy::create(1.0f, 360.0f)));
        setNormalImage(loadingSprite);
    }

    return true;
}

void LazyIcon::createIcon() {
    setEnabled(true);
    if (m_error.empty()) {
        if (!m_frameName.empty()) {
            auto spriteFrame = Icons::getFrame(m_frameName);
            if (!spriteFrame) spriteFrame = Get::SpriteFrameCache()->spriteFrameByName("GJ_deleteIcon_001.png");
            setNormalImage(CCSprite::createWithSpriteFrame(spriteFrame));
        }
        else {
            setNormalImage(SimpleIcon::create(m_type, m_name));
        }
    }
    else {
        setNormalImage(CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png"));
    }
}

void LazyIcon::setNormalImage(CCNode* image) {
    if (m_pNormalImage) {
        m_pNormalImage->removeFromParent();
    }
    if (image) {
        image->setPosition({ 15.0f, 15.0f });
        image->setTag(1);
        addChild(image);
    }
    m_pNormalImage = image;
}

void LazyIcon::activate() {
    CCMenuItemSpriteExtra::activate();
    if (m_error.empty()) m_callback();
    else FLAlertLayer::create("Error", m_error, "OK")->show();
}

void LazyIcon::visit() {
    CCNode::visit();

    if (!m_bVisible || m_visited) return;

    m_visited = true;

    ThreadPool::get().pushTask([
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
        auto spriteFrameCache = Get::SpriteFrameCache();
        for (auto& frame : m_frames) {
            spriteFrameCache->removeSpriteFrameByName(frame.c_str());
        }
    }
    if (!m_key.empty()) {
        Get::TextureCache()->removeTextureForKey(m_key.c_str());
    }
}

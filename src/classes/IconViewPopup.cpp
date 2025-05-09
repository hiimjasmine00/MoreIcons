#include "IconViewPopup.hpp"
#include "EditIconPopup.hpp"
#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include <BS_thread_pool.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/ui/Scrollbar.hpp>
#include <Geode/utils/ranges.hpp>

using namespace geode::prelude;

IconViewPopup* IconViewPopup::create(IconType type, bool custom) {
    auto ret = new IconViewPopup();
    if (ret->initAnchored(440.0f, 290.0f, type, custom)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool IconViewPopup::setup(IconType type, bool custom) {
    auto title = "";
    switch (type) {
        case IconType::Cube: title = custom ? "Custom Icons" : "Vanilla Icons"; break;
        case IconType::Ship: title = custom ? "Custom Ships" : "Vanilla Ships"; break;
        case IconType::Ball: title = custom ? "Custom Balls" : "Vanilla Balls"; break;
        case IconType::Ufo: title = custom ? "Custom UFOs" : "Vanilla UFOs"; break;
        case IconType::Wave: title = custom ? "Custom Waves" : "Vanilla Waves"; break;
        case IconType::Robot: title = custom ? "Custom Robots" : "Vanilla Robots"; break;
        case IconType::Spider: title = custom ? "Custom Spiders" : "Vanilla Spiders"; break;
        case IconType::Swing: title = custom ? "Custom Swings" : "Vanilla Swings"; break;
        case IconType::Jetpack: title = custom ? "Custom Jetpacks" : "Vanilla Jetpacks"; break;
        case IconType::Special: title = custom ? "Custom Trails" : "Vanilla Trails"; break;
        default: title = ""; break;
    }

    setID("IconViewPopup");
    setTitle(title);
    m_title->setID("icon-view-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_iconType = type;
    m_custom = custom;

    auto scrollBackground = CCScale9Sprite::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    scrollBackground->setPosition({ 215.0f, 135.0f });
    scrollBackground->setContentSize({ 400.0f, 240.0f });
    scrollBackground->setOpacity(105);
    scrollBackground->setID("scroll-background");
    m_mainLayer->addChild(scrollBackground);

    m_scrollLayer = BiggerScrollLayer::create(400.0f, 230.0f, 5.0f);
    m_scrollLayer->m_contentLayer->setLayout(RowLayout::create()
        ->setGap(roundf(7.5f / GJItemIcon::scaleForType(GameManager::get()->iconTypeToUnlockType(m_iconType))))
        ->setGrowCrossAxis(true));
    m_scrollLayer->ignoreAnchorPointForPosition(false);
    m_scrollLayer->setPosition({ 215.0f, 135.0f });
    m_scrollLayer->setID("scroll-layer");
    m_mainLayer->addChild(m_scrollLayer);

    auto scrollbar = Scrollbar::create(m_scrollLayer);
    scrollbar->setPosition({ 425.0f, 135.0f });
    scrollbar->setID("scrollbar");
    m_mainLayer->addChild(scrollbar);

    if (m_custom) loadCustomIcons();
    else loadVanillaIcons();

    return true;
}

BS::thread_pool<BS::tp::none>& sharedPool() {
    static BS::thread_pool<BS::tp::none> pool(std::thread::hardware_concurrency());
    return pool;
}

struct LoadedIcon {
    std::vector<std::string> paths;
    std::string name;
    int index;
    IconType type;
    bool custom;
    std::vector<CCImage*> images;
    CCDictionary* frames;
    int format;
};

std::vector<LoadedIcon> loadedIcons;
std::mutex loadedIconsMutex;

void IconViewPopup::loadVanillaIcons() {
    auto type = m_iconType;
    if (m_custom || type > IconType::Jetpack) return setupIcons();

    auto gameManager = GameManager::get();
    auto iconCount = gameManager->countForType(type);
    if (iconCount <= 0) return;

    auto scaleFactor = CCDirector::get()->getContentScaleFactor();
    auto suffix = scaleFactor >= 4.0f ? "-uhd.png" : scaleFactor >= 2.0f ? "-hd.png" : ".png";
    auto textureCache = CCTextureCache::get();
    for (int i = 1; i <= iconCount; i++) {
        auto path = MoreIcons::vanillaTexturePath(fmt::format("icons/{}{:02}{}", MoreIcons::prefixes[(int)type], i, suffix), false);
        if (textureCache->m_pTextures->objectForKey(path)) continue;

        sharedPool().detach_task([i, path, type] {
            auto dict = CCDictionary::createWithContentsOfFileThreadSafe((path.substr(0, path.size() - 4) + ".plist").c_str());
            if (!dict) return;

            auto frames = static_cast<CCDictionary*>(dict->objectForKey("frames"));
            if (!frames) return dict->release();

            auto metadata = static_cast<CCDictionary*>(dict->objectForKey("metadata"));
            auto formatStr = metadata ? metadata->valueForKey("format") : nullptr;
            auto format = formatStr ? numFromString<int>(formatStr->m_sString).unwrapOr(0) : 0;

            auto image = new CCImage();
            if (!image->initWithImageFile(path.c_str())) return dict->release(), image->release();

            {
                std::lock_guard lock(loadedIconsMutex);

                loadedIcons.push_back({
                    .paths = { path },
                    .name = "",
                    .index = i,
                    .type = type,
                    .custom = false,
                    .images = { image },
                    .frames = frames,
                    .format = format
                });
            }
        });
    }

    sharedPool().wait();
    finishLoadIcons();
}

void IconViewPopup::loadCustomIcons() {
    auto type = m_iconType;
    if (!m_custom) return;

    auto gameManager = GameManager::get();
    auto& [start, end] = MoreIconsAPI::iconIndices[type];
    std::vector<IconInfo> icons = { MoreIconsAPI::icons.begin() + start, MoreIconsAPI::icons.begin() + end };
    if (icons.empty()) return;

    auto textureCache = CCTextureCache::get();
    for (auto& icon : icons) {
        auto textures = ranges::filter(icon.textures, [textureCache](const std::string& path) {
            return !textureCache->m_pTextures->objectForKey(path);
        });
        sharedPool().detach_task([icon, textures, type] {
            std::vector<CCImage*> images;
            std::vector<std::string> paths;
            for (auto& path : textures) {
                auto image = new CCImage();
                if (!image->initWithImageFile(path.c_str())) {
                    image->release();
                    continue;
                }
                images.push_back(image);
                paths.push_back(path);
            }

            CCDictionary* frames = nullptr;
            auto format = 0;
            if (!icon.sheetName.empty()) {
                auto dict = CCDictionary::createWithContentsOfFileThreadSafe(icon.sheetName.c_str());
                if (!dict) return;

                frames = new CCDictionary();
                for (auto [frameName, frame] : CCDictionaryExt<std::string, CCDictionary*>(static_cast<CCDictionary*>(dict->objectForKey("frames")))) {
                    frames->setObject(frame, MoreIconsAPI::getFrameName(frameName, icon.name, type));
                }

                auto metadata = static_cast<CCDictionary*>(dict->objectForKey("metadata"));
                auto formatStr = metadata ? metadata->valueForKey("format") : nullptr;
                format = formatStr ? numFromString<int>(formatStr->m_sString).unwrapOr(0) : 0;

                dict->release();
            }

            {
                std::lock_guard lock(loadedIconsMutex);

                loadedIcons.push_back({
                    .paths = paths,
                    .name = icon.name,
                    .index = 0,
                    .type = type,
                    .custom = true,
                    .images = images,
                    .frames = frames,
                    .format = format
                });
            }
        });
    }

    sharedPool().wait();
    finishLoadIcons();
}

void IconViewPopup::finishLoadIcons() {
    std::lock_guard lock(loadedIconsMutex);

    auto textureCache = CCTextureCache::get();
    auto spriteFrameCache = CCSpriteFrameCache::get();
    auto type = m_iconType;
    auto custom = m_custom;
    for (auto& icon : loadedIcons) {
        if (icon.type != type || icon.custom != custom) continue;

        CCTexture2D* texture = nullptr;
        for (int i = 0; i < icon.images.size(); i++) {
            auto image = icon.images[i];
            texture = new CCTexture2D();
            if (texture->initWithImage(image)) {
                auto& path = icon.paths[i];
                textureCache->m_pTextures->setObject(texture, path);
                m_textures.push_back(path);
            }
            texture->release();
            image->release();
        }

        if (icon.frames) {
            for (auto [frameName, frame] : CCDictionaryExt<std::string, CCDictionary*>(icon.frames)) {
                if (auto spriteFrame = MoreIconsAPI::createSpriteFrame(frame, texture, icon.format)) {
                    spriteFrameCache->addSpriteFrame(spriteFrame, frameName.c_str());
                    m_frames.push_back(frameName);
                }
            }
            icon.frames->release();
        }
    }

    if (!loadedIcons.empty()) {
        ranges::remove(loadedIcons, [custom, type](const LoadedIcon& icon) { return icon.type == type && icon.custom == custom; });
        setupIcons();
    }
}

void IconViewPopup::setupIcons() {
    auto gameManager = GameManager::get();
    auto iconType = m_iconType;
    auto isIcon = iconType <= IconType::Jetpack;
    auto unlockType = gameManager->iconTypeToUnlockType(iconType);

    if (m_custom) {
        auto& [start, end] = MoreIconsAPI::iconIndices[iconType];
        std::vector<IconInfo> icons = { MoreIconsAPI::icons.begin() + start, MoreIconsAPI::icons.begin() + end };
        for (auto& icon : icons) {
            CCSprite* sprite = nullptr;
            if (isIcon) {
                auto itemIcon = GJItemIcon::createBrowserItem(unlockType, 1);
                MoreIconsAPI::updateSimplePlayer(static_cast<SimplePlayer*>(itemIcon->m_player), icon.name, iconType);
                sprite = itemIcon;
            }
            else if (iconType == IconType::Special) {
                auto square = CCSprite::createWithSpriteFrameName("playerSquare_001.png");
                square->setColor({ 150, 150, 150 });
                auto streak = CCSprite::create(icon.textures[0].c_str());
                limitNodeHeight(streak, 27.0f, 999.0f, 0.001f);
                streak->setRotation(-90.0f);
                streak->setPosition(square->getContentSize() / 2);
                square->addChild(streak);
                sprite = square;
            }
            auto button = CCMenuItemExt::createSpriteExtra(sprite, [iconType, name = icon.name](auto) {
                EditIconPopup::create(iconType, 0, name, true)->show();
            });
            button->setID(icon.name);
            m_scrollLayer->m_contentLayer->addChild(button);
        }
    }
    else {
        auto iconCount = gameManager->countForType(iconType);
        auto keyStart = isIcon ? gameManager->m_keyStartForIcon[(int)iconType] : 0;
        for (int i = 1; i <= iconCount; i++) {
            auto iconKey = isIcon ? keyStart + i - 1 : 0;
            auto itemIcon = GJItemIcon::createBrowserItem(unlockType, i);
            auto button = CCMenuItemExt::createSpriteExtra(itemIcon, [i, iconType](auto) {
                EditIconPopup::create(iconType, i, "", true)->show();
            });
            button->setID(fmt::to_string(i));
            m_scrollLayer->m_contentLayer->addChild(button);
        }
    }

    m_scrollLayer->m_contentLayer->updateLayout();
    m_scrollLayer->scrollToTop();
}

IconViewPopup::~IconViewPopup() {
    auto textureCache = CCTextureCache::get();
    for (auto& key : m_textures) {
        textureCache->removeTextureForKey(key.c_str());
    }
    auto spriteFrameCache = CCSpriteFrameCache::get();
    for (auto& key : m_frames) {
        spriteFrameCache->removeSpriteFrameByName(key.c_str());
    }
}

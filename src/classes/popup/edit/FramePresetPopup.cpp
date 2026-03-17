#include "FramePresetPopup.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Defaults.hpp"
#include "../../../utils/Filesystem.hpp"
#include "../../../utils/Get.hpp"
#include "../../../utils/Icons.hpp"
#include "../../../utils/Load.hpp"
#include <arc/future/Join.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/ui/Scrollbar.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <jasmine/mod.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;
using namespace jasmine::mod;

FramePresetPopup* FramePresetPopup::create(IconType type, Function<void(CCSpriteFrame*)> callback) {
    auto ret = new FramePresetPopup();
    if (ret->init(type, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

struct ExtendedResult {
    std::vector<uint8_t> data;
    Ref<CCTexture2D> texture;
    StringMap<Ref<CCSpriteFrame>> frames;
    uint32_t width;
    uint32_t height;
    IconInfo* info;
    int id;
    int index;

    ExtendedResult(ImageResult&& image, IconInfo* info, int id, int index)
        : data(std::move(image.data))
        , texture(image.texture)
        , frames(std::move(image.frames))
        , width(image.width)
        , height(image.height)
        , info(info)
        , id(id)
        , index(index) {}
    ExtendedResult(const ExtendedResult& result) = delete;
    ExtendedResult(ExtendedResult&& result) noexcept :
        data(std::move(result.data)),
        texture(result.texture),
        frames(std::move(result.frames)),
        width(result.width),
        height(result.height),
        info(result.info),
        id(result.id),
        index(result.index) {}
    ExtendedResult& operator=(const ExtendedResult& result) = delete;
    ExtendedResult& operator=(ExtendedResult&& result) noexcept {
        data = std::move(result.data);
        texture = result.texture;
        frames = std::move(result.frames);
        width = result.width;
        height = result.height;
        info = result.info;
        id = result.id;
        index = result.index;
        return *this;
    }
};

std::mutex imageMutex2;
std::vector<ExtendedResult> images2;

void loadDeathEffects(std::vector<arc::BlockingTaskHandle<void>>& tasks, Filesystem::PathView suffix) {
    auto& rt = runtime();

    auto count = Get::gameManager->countForType(IconType::DeathEffect);
    for (int i = 1; i < count; i++) {
        auto texture = Icons::vanillaTexturePath(fmt::format(L("PlayerExplosion_{:02}{}.png"), i, suffix), false);
        auto sheet = Icons::vanillaTexturePath(fmt::format(L("PlayerExplosion_{:02}{}.plist"), i, suffix), false);
        tasks.push_back(rt.spawnBlocking<void>([texture = std::move(texture), sheet = std::move(sheet), i] mutable {
            auto res = Load::createFrames(texture, sheet, {}, IconType::DeathEffect, {}, false, true);
            if (res.isErr()) return;

            std::unique_lock lock(imageMutex2);
            images2.emplace_back(std::move(res).unwrap(), nullptr, i, 0);
        }));
    }

    if (auto icons = more_icons::getIcons(IconType::DeathEffect)) {
        for (auto& info : *icons) {
            auto& texture = info.getTexture();
            auto& sheet = info.getSheet();
            tasks.push_back(rt.spawnBlocking<void>([&texture, &sheet, name = info.getName(), info = &info] mutable {
                auto res = Load::createFrames(texture, sheet, {}, IconType::DeathEffect, {}, false);
                if (res.isErr()) return;

                std::unique_lock lock(imageMutex2);
                images2.emplace_back(std::move(res).unwrap(), info, 0, 0);
            }));
        }
    }
}

void loadShipFires(std::vector<arc::BlockingTaskHandle<void>>& tasks) {
    auto& rt = runtime();

    auto count = Get::gameManager->countForType(IconType::ShipFire);
    for (int i = 2; i <= count; i++) {
        for (int j = 1; j <= Defaults::getShipFireCount(i); j++) {
            auto texture = Icons::vanillaTexturePath(fmt::format(L("shipfire{:02}_{:03}.png"), i, j), true);
            auto textureName = fmt::format("shipfire{:02}-{}", i, j);
            tasks.push_back(rt.spawnBlocking<void>([texture = std::move(texture), textureName = std::move(textureName), i, j] mutable {
                auto res = Load::createFrames(texture, {}, {}, IconType::ShipFire, {}, false, false);
                if (res.isErr()) return;

                std::unique_lock lock(imageMutex2);
                images2.emplace_back(std::move(res).unwrap(), nullptr, i, j);
            }));
        }
    }

    if (auto icons = more_icons::getIcons(IconType::ShipFire)) {
        for (auto& info : *icons) {
            for (int i = 1; i <= info.getFireCount(); i++) {
                auto texture = info.getTexture();
                auto& textureString = Filesystem::getPathString(texture);
                textureString.replace(textureString.size() - 7, 3, fmt::format(L("{:03}"), i));
                tasks.push_back(rt.spawnBlocking<void>([texture = std::move(texture), info = &info, i] mutable {
                    auto res = Load::createFrames(texture, {}, {}, IconType::ShipFire, {}, false, false);
                    if (res.isErr()) return;

                    std::unique_lock lock(imageMutex2);
                    images2.emplace_back(std::move(res).unwrap(), info, 0, i);
                }));
            }
        }
    }
}

bool FramePresetPopup::init(IconType type, Function<void(CCSpriteFrame*)> callback) {
    if (!BasePopup::init(440.0f, 290.0f, "geode.loader/GE_square03.png", CircleBaseColor::DarkPurple)) return false;

    setID("FramePresetPopup");
    setTitle(fmt::format("{} Presets", Constants::getSingularUppercase(type)));
    m_title->setID("frame-preset-title");

    m_callback = std::move(callback);
    m_type = type;

    m_loadingSprite = CCSprite::create("loadingCircle.png");
    m_loadingSprite->setPosition({ 215.0f, 135.0f });
    m_loadingSprite->runAction(CCRepeatForever::create(CCRotateBy::create(1.0f, 360.0f)));
    m_loadingSprite->setID("loading-sprite");
    m_mainLayer->addChild(m_loadingSprite);

    images2.clear();

    std::vector<arc::BlockingTaskHandle<void>> tasks;

    auto factor = Get::director->getContentScaleFactor();
    Filesystem::PathView suffix;
    if (factor >= 4.0f) suffix = L("-uhd");
    else if (factor >= 2.0f) suffix = L("-hd");

    if (type == IconType::DeathEffect) loadDeathEffects(tasks, suffix);
    else if (type == IconType::ShipFire) loadShipFires(tasks);

    m_loader.spawn(arc::joinAll(tasks), [this] {
        std::unique_lock lock(imageMutex2);

        std::ranges::sort(images2, [](const ExtendedResult& a, const ExtendedResult& b) {
            if (a.info && b.info) return *a.info == *b.info ? a.index < b.index : *a.info < *b.info;
            else if (a.info) return false;
            else if (b.info) return true;
            else return a.id == b.id ? a.index < b.index : a.id < b.id;
        });

        for (auto it = images2.begin(); it != images2.end(); it = images2.erase(it)) {
            auto& image = *it;
            Load::initTexture(image.texture, image.data.data(), image.width, image.height, false);

            if (image.frames.empty()) {
                m_textures.push_back(std::move(image.texture));
                if (image.info) {
                    m_names.push_back(fmt::format("{}-{}"_spr, image.info->getName(), image.index));
                }
                else {
                    m_names.push_back(fmt::format("shipfire{:02}-{}", image.id, image.index));
                }
            }
            else {
                std::vector<std::string_view> keys;
                for (auto& frame : image.frames) {
                    keys.push_back(frame.first);
                }
                std::ranges::sort(keys);
                for (size_t i = 0; i < keys.size(); i++) {
                    auto it = image.frames.find(keys[i]);
                    if (it != image.frames.end()) {
                        m_frames.push_back(std::move(it->second));
                        if (image.info) {
                            m_names.push_back(fmt::format("{}-{}"_spr, image.info->getName(), i));
                        }
                        else {
                            m_names.push_back(fmt::format("PlayerExplosion_{:02}-{}", image.id, i));
                        }
                        image.frames.erase(it);
                    }
                }
            }
        }
        setupScroll();
    });

    handleTouchPriority(this);

    return true;
}

void FramePresetPopup::setupScroll() {
    m_loadingSprite->removeFromParent();

    auto scrollBackground = CCLayerColor::create({ 0, 0, 0, 105 }, 400.0f, 240.0f);
    scrollBackground->setPosition({ 215.0f, 135.0f });
    scrollBackground->ignoreAnchorPointForPosition(false);
    scrollBackground->setID("scroll-background");
    m_mainLayer->addChild(scrollBackground);

    auto scrollLayer = ScrollLayer::create({ 400.0f, 240.0f });
    auto contentLayer = scrollLayer->m_contentLayer;
    scrollLayer->setPosition({ 215.0f, 135.0f });
    scrollLayer->ignoreAnchorPointForPosition(false);
    contentLayer->ignoreAnchorPointForPosition(false);
    scrollLayer->setID("scroll-layer");
    m_mainLayer->addChild(scrollLayer);

    if (m_type == IconType::ShipFire) {
        for (size_t i = 0; i < m_textures.size(); i++) {
            auto iconSprite = CCSprite::createWithTexture(m_textures[i]);
            limitNodeHeight(iconSprite, 30.0f, 1.0f, 0.0f);
            auto iconButton = CCMenuItemSpriteExtra::create(iconSprite, this, menu_selector(FramePresetPopup::onSelect));
            iconButton->setPosition({ 15.0f, 30.0f });
            iconButton->setID(fmt::format("{}-button", m_names[i]));

            auto iconMenu = CCMenu::create();
            iconMenu->setContentSize({ 30.0f, 30.0f });
            iconMenu->ignoreAnchorPointForPosition(false);
            iconMenu->setID(fmt::format("{}-menu", m_names[i]));
            iconMenu->addChild(iconButton);
            contentLayer->addChild(iconMenu);
        }
    }
    else {
        for (size_t i = 0; i < m_frames.size(); i++) {
            auto iconSprite = CCSprite::createWithSpriteFrame(m_frames[i]);
            limitNodeHeight(iconSprite, 30.0f, 1.0f, 0.0f);
            auto iconButton = CCMenuItemSpriteExtra::create(iconSprite, this, menu_selector(FramePresetPopup::onSelect));
            iconButton->setPosition({ 15.0f, 30.0f });
            iconButton->setID(fmt::format("{}-button", m_names[i]));

            auto iconMenu = CCMenu::create();
            iconMenu->setContentSize({ 30.0f, 30.0f });
            iconMenu->ignoreAnchorPointForPosition(false);
            iconMenu->setID(fmt::format("{}-menu", m_names[i]));
            iconMenu->addChild(iconButton);
            contentLayer->addChild(iconMenu);
        }
    }

    contentLayer->setLayout(RowLayout::create()->setGap(Constants::getIconGap(m_type))->setGrowCrossAxis(true));

    contentLayer->setContentSize(contentLayer->getContentSize() + CCSize { 0.0f, 10.0f });
    for (auto child : CCArrayExt<CCNode, false>(contentLayer->getChildren())) {
        child->setPosition(child->getPosition() + CCPoint { 0.0f, 5.0f });
        child->setContentSize(child->getContentSize() + CCSize { 0.0f, 30.0f });
    }

    scrollLayer->scrollToTop();

    auto scrollable = contentLayer->getContentHeight() > scrollLayer->getContentHeight();
    scrollLayer->enableScrollWheel(scrollable);
    scrollLayer->setTouchEnabled(scrollable);

    auto scrollbar = Scrollbar::create(scrollLayer);
    scrollbar->setPosition({ 425.0f, 135.0f });
    scrollbar->setTouchEnabled(scrollable);
    scrollbar->setID("scrollbar");
    m_mainLayer->addChild(scrollbar);
}

void FramePresetPopup::onSelect(CCObject* sender) {
    if (m_callback) m_callback(static_cast<CCSprite*>(static_cast<CCMenuItemSprite*>(sender)->getNormalImage())->displayFrame());
    close();
}

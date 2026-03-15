#include "FramePresetPopup.hpp"
#include "../../misc/LazyIcon.hpp"
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
#include <MoreIcons.hpp>

using namespace geode::prelude;

FramePresetPopup* FramePresetPopup::create(IconType type, Function<void(CCSpriteFrame*)> callback) {
    auto ret = new FramePresetPopup();
    if (ret->init(type, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

std::mutex imageMutex2;
std::vector<ImageResult> images2;

void loadDeathEffects(std::vector<arc::BlockingTaskHandle<void>>& tasks, std::string_view suffix) {
    auto& rt = runtime();

    auto count = Get::gameManager->countForType(IconType::DeathEffect);
    for (int i = 2; i <= count; i++) {
        auto texture = Icons::vanillaTexturePath(fmt::format(L("PlayerExplosion_{:02}{}.png"), i, suffix), false);
        auto sheet = Icons::vanillaTexturePath(fmt::format(L("PlayerExplosion_{:02}{}.plist"), i, suffix), false);
        tasks.push_back(rt.spawnBlocking<void>([texture = std::move(texture), sheet = std::move(sheet)] {
            auto res = Load::createFrames(texture, sheet, {}, IconType::DeathEffect, {}, false, true);
            if (res.isErr()) return;

            std::unique_lock lock(imageMutex2);
            images2.push_back(std::move(res).unwrap());
        }));
    }

    if (auto icons = more_icons::getIcons(IconType::DeathEffect)) {
        for (auto& info : *icons) {
            auto& texture = info.getTexture();
            auto& sheet = info.getSheet();
            auto& name = info.getName();
            tasks.push_back(rt.spawnBlocking<void>([&texture, &sheet, &name] {
                auto res = Load::createFrames(texture, sheet, name, IconType::DeathEffect, {}, false);
                if (res.isErr()) return;

                std::unique_lock lock(imageMutex2);
                images2.push_back(std::move(res).unwrap());
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
            auto textureName = fmt::format("shipfire{:02}_{:03}", i, j);
            tasks.push_back(rt.spawnBlocking<void>([texture = std::move(texture), textureName = std::move(textureName)] {
                auto res = Load::createFrames(texture, {}, {}, IconType::ShipFire, {}, false, false);
                if (res.isErr()) return;

                auto image = std::move(res).unwrap();
                image.name = std::move(textureName);

                std::unique_lock lock(imageMutex2);
                images2.push_back(std::move(image));
            }));
        }
    }

    if (auto icons = more_icons::getIcons(IconType::ShipFire)) {
        for (auto& info : *icons) {
            std::vector<std::filesystem::path> textures;
            std::vector<std::string> textureNames;
            for (int i = 1; i <= info.getFireCount(); i++) {
                auto texture = info.getTexture();
                auto& textureString = Filesystem::getPathString(texture);
                textureString.replace(textureString.size() - 7, 3, fmt::format(L("{:03}"), i));
                textures.push_back(std::move(texture));
                textureNames.push_back(fmt::format("{}_{:03}", info.getName(), i));
            }
            while (!textures.empty()) {
                tasks.push_back(rt.spawnBlocking<void>([texture = std::move(textures[0]), textureName = std::move(textureNames[0])] {
                    auto res = Load::createFrames(texture, {}, {}, IconType::ShipFire, {}, false, false);
                    if (res.isErr()) return;

                    auto image = std::move(res).unwrap();
                    image.name = std::move(textureName);

                    std::unique_lock lock(imageMutex2);
                    images2.push_back(std::move(image));
                }));
                textures.erase(textures.begin());
                textureNames.erase(textureNames.begin());
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
    std::string_view suffix = factor >= 4.0f ? "-uhd" : factor >= 2.0f ? "-hd" : "";

    if (type == IconType::DeathEffect) loadDeathEffects(tasks, suffix);
    else if (type == IconType::ShipFire) loadShipFires(tasks);

    m_loader.spawn(arc::joinAll(tasks), [this] {
        for (auto it = images2.begin(); it != images2.end(); it = images2.erase(it)) {
            auto& image = *it;
            Load::initTexture(image.texture, image.data.data(), image.width, image.height, false);
            m_textures.push_back(std::move(image.texture));
            m_textureNames.push_back(std::move(image.name));
            for (auto it2 = image.frames.begin(); it2 != image.frames.end(); it2 = image.frames.erase(it2)) {
                m_frames.insert(std::move(*it2));
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
            iconButton->setID(m_textureNames[i]);

            auto iconMenu = CCMenu::create();
            iconMenu->setContentSize({ 30.0f, 30.0f });
            iconMenu->ignoreAnchorPointForPosition(false);
            iconMenu->setID(fmt::format("{}-menu", m_textureNames[i]));
            iconMenu->addChild(iconButton);
            contentLayer->addChild(iconMenu);
        }
    }
    else {
        for (auto& frame : m_frames) {
            auto iconSprite = CCSprite::createWithSpriteFrame(frame.second);
            limitNodeHeight(iconSprite, 30.0f, 1.0f, 0.0f);
            auto iconButton = CCMenuItemSpriteExtra::create(iconSprite, this, menu_selector(FramePresetPopup::onSelect));
            iconButton->setPosition({ 15.0f, 30.0f });
            iconButton->setID(frame.first);

            auto iconMenu = CCMenu::create();
            iconMenu->setContentSize({ 30.0f, 30.0f });
            iconMenu->ignoreAnchorPointForPosition(false);
            iconMenu->setID(fmt::format("{}-menu", frame.first));
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

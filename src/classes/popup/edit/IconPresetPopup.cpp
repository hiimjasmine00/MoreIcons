#include "IconPresetPopup.hpp"
#include "../../misc/LazyIcon.hpp"
#include "../../scroll/BiggerScrollLayer.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/ui/Scrollbar.hpp>

using namespace geode::prelude;

IconPresetPopup* IconPresetPopup::create(IconType type, std::string_view suffix, std23::move_only_function<void(int, IconInfo*)> callback) {
    auto ret = new IconPresetPopup();
    if (ret->initAnchored(440.0f, 290.0f, type, suffix, std::move(callback), "geode.loader/GE_square03.png")) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool IconPresetPopup::setup(IconType type, std::string_view suffix, std23::move_only_function<void(int, IconInfo*)> callback) {
    setID("IconPresetPopup");
    setTitle(fmt::format("{} Presets", MoreIconsAPI::uppercase[MoreIconsAPI::convertType(type)]));
    m_title->setID("icon-preset-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_callback = std::move(callback);

    auto gameManager = MoreIconsAPI::getGameManager();
    auto scrollLayer = BiggerScrollLayer::create(400.0f, 230.0f, 5.0f, 15.0f);
    scrollLayer->m_contentLayer->setLayout(
        RowLayout::create()->setGap(roundf(7.5f / GJItemIcon::scaleForType(gameManager->iconTypeToUnlockType(type))))->setGrowCrossAxis(true));
    scrollLayer->setPosition({ 215.0f, 135.0f });
    scrollLayer->setID("scroll-layer");
    m_mainLayer->addChild(scrollLayer);

    auto scrollbar = Scrollbar::create(scrollLayer);
    scrollbar->setPosition({ 425.0f, 135.0f });
    scrollbar->setID("scrollbar");
    m_mainLayer->addChild(scrollbar);

    auto vanillaCount = gameManager->countForType(type);
    for (int i = 1; i <= vanillaCount; i++) {
        auto lazyIcon = LazyIcon::create(type, i, nullptr, suffix, [this, i] {
            if (m_callback) m_callback(i, nullptr);
            onClose(nullptr);
        });
        auto iconMenu = CCMenu::createWithItem(lazyIcon);
        iconMenu->setContentSize(lazyIcon->getContentSize());
        iconMenu->ignoreAnchorPointForPosition(false);
        iconMenu->setID(fmt::format("{}-menu", lazyIcon->getID()));
        scrollLayer->m_contentLayer->addChild(iconMenu);
    }

    auto& icons = MoreIconsAPI::icons[type];
    auto customCount = icons.size();
    for (int i = 0; i < customCount; i++) {
        auto info = icons.data() + i;
        auto lazyIcon = LazyIcon::create(type, 0, info, suffix, [this, info] {
            if (m_callback) m_callback(0, info);
            onClose(nullptr);
        });
        auto iconMenu = CCMenu::createWithItem(lazyIcon);
        iconMenu->setContentSize(lazyIcon->getContentSize());
        iconMenu->ignoreAnchorPointForPosition(false);
        iconMenu->setID(fmt::format("{}-menu", lazyIcon->getID()));
        scrollLayer->m_contentLayer->addChild(iconMenu);
    }

    scrollLayer->m_contentLayer->updateLayout();
    scrollLayer->scrollToTop();

    handleTouchPriority(this);

    return true;
}

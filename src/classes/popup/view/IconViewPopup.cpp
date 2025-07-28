#include "IconViewPopup.hpp"
#include "LazyIcon.hpp"
#include "../../scroll/BiggerScrollLayer.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/ui/Scrollbar.hpp>

using namespace geode::prelude;

IconViewPopup* IconViewPopup::create(IconType type, bool custom) {
    auto ret = new IconViewPopup();
    if (ret->initAnchored(440.0f, 290.0f, type, custom, "geode.loader/GE_square03.png")) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool IconViewPopup::setup(IconType type, bool custom) {
    setID("IconViewPopup");
    setTitle(fmt::format("{} {}s", custom ? "Custom" : "Vanilla", MoreIconsAPI::uppercase[MoreIconsAPI::convertType(type)]));
    m_title->setID("icon-view-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    auto gameManager = GameManager::get();
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

    if (custom) {
        auto& icons = MoreIconsAPI::icons[type];
        auto count = icons.size();
        for (int i = 0; i < count; i++) {
            scrollLayer->m_contentLayer->addChild(LazyIcon::create(type, 0, icons.data() + i));
        }
    }
    else {
        auto count = gameManager->countForType(type);
        for (int i = 1; i <= count; i++) {
            scrollLayer->m_contentLayer->addChild(LazyIcon::create(type, i, nullptr));
        }
    }

    scrollLayer->m_contentLayer->updateLayout();
    scrollLayer->scrollToTop();

    handleTouchPriority(this);

    return true;
}

#include "IconViewPopup.hpp"
#include "LazyIcon.hpp"
#include "../../scroll/BiggerScrollLayer.hpp"
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
    constexpr std::array titles = {
        "", "Icons", "", "", "Ships", "Balls", "UFOs", "Waves", "Robots",
        "Spiders", "Trails", "Death Effects", "", "Swings", "Jetpacks", "Ship Fires"
    };
    auto gameManager = GameManager::get();
    auto unlock = gameManager->iconTypeToUnlockType(type);

    setID("IconViewPopup");
    setTitle(fmt::format("{} {}", custom ? "Custom" : "Vanilla", titles[(int)unlock]));
    m_title->setID("icon-view-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    auto scrollLayer = BiggerScrollLayer::create(400.0f, 230.0f, 5.0f, 15.0f);
    scrollLayer->m_contentLayer->setLayout(RowLayout::create()->setGap(roundf(7.5f / GJItemIcon::scaleForType(unlock)))->setGrowCrossAxis(true));
    scrollLayer->setPosition({ 215.0f, 135.0f });
    scrollLayer->setID("scroll-layer");
    m_mainLayer->addChild(scrollLayer);

    auto scrollbar = Scrollbar::create(scrollLayer);
    scrollbar->setPosition({ 425.0f, 135.0f });
    scrollbar->setID("scrollbar");
    m_mainLayer->addChild(scrollbar);

    auto start = 0;
    auto end = 0;
    if (custom) {
        auto& [first, second] = MoreIconsAPI::iconIndices[type];
        start = first;
        end = second;
    }
    else {
        start = 1;
        end = gameManager->countForType(type);
    }

    for (int i = start; i < end; i++) {
        scrollLayer->m_contentLayer->addChild(LazyIcon::create(type, custom ? 0 : i, custom ? MoreIconsAPI::icons.data() + i : nullptr));
    }

    scrollLayer->m_contentLayer->updateLayout();
    scrollLayer->scrollToTop();

    return true;
}

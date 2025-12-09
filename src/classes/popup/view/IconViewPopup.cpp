#include "IconViewPopup.hpp"
#include "ViewIconPopup.hpp"
#include "../../misc/LazyIcon.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Get.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/ui/Scrollbar.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <MoreIconsV2.hpp>

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
    setTitle(fmt::format("{} {}s", custom ? "Custom" : "Vanilla", MoreIcons::uppercase[MoreIcons::convertType(type)]));
    m_title->setID("icon-view-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    auto scrollBackground = CCLayerColor::create({ 0, 0, 0, 105 }, 400.0f, 240.0f);
    scrollBackground->setPosition({ 215.0f, 135.0f });
    scrollBackground->ignoreAnchorPointForPosition(false);
    scrollBackground->setID("scroll-background");
    m_mainLayer->addChild(scrollBackground);

    auto gameManager = Get::GameManager();
    auto scrollLayer = ScrollLayer::create({ 400.0f, 240.0f });
    auto contentLayer = scrollLayer->m_contentLayer;
    scrollLayer->setPosition({ 215.0f, 135.0f });
    scrollLayer->ignoreAnchorPointForPosition(false);
    contentLayer->setLayout(
        RowLayout::create()->setGap(roundf(7.5f / GJItemIcon::scaleForType(gameManager->iconTypeToUnlockType(type))))->setGrowCrossAxis(true));
    scrollLayer->setID("scroll-layer");
    m_mainLayer->addChild(scrollLayer);

    if (custom) {
        if (auto icons = more_icons::getIcons(type)) {
            auto end = icons->data() + icons->size();
            for (auto info = icons->data(); info != end; info++) {
                auto iconMenu = CCMenu::create();
                auto lazyIcon = LazyIcon::create(type, 0, info, {}, [this, info, type] {
                    ViewIconPopup::create(type, 0, info)->show();
                });
                lazyIcon->setPosition({ 15.0f, 30.0f });
                iconMenu->setContentSize({ 30.0f, 30.0f });
                iconMenu->ignoreAnchorPointForPosition(false);
                iconMenu->setID(fmt::format("{}-menu", lazyIcon->getID()));
                iconMenu->addChild(lazyIcon);
                contentLayer->addChild(iconMenu);
            }
        }
    }
    else {
        auto count = gameManager->countForType(type);
        for (int i = 1; i <= count; i++) {
            auto iconMenu = CCMenu::create();
            auto lazyIcon = LazyIcon::create(type, i, nullptr, {}, [this, i, type] {
                ViewIconPopup::create(type, i, nullptr)->show();
            });
            lazyIcon->setPosition({ 15.0f, 30.0f });
            iconMenu->setContentSize({ 30.0f, 30.0f });
            iconMenu->ignoreAnchorPointForPosition(false);
            iconMenu->setID(fmt::format("{}-menu", lazyIcon->getID()));
            iconMenu->addChild(lazyIcon);
            contentLayer->addChild(iconMenu);
        }
    }

    contentLayer->updateLayout();

    contentLayer->setContentSize(contentLayer->getContentSize() + CCSize { 0.0f, 10.0f });
    for (auto child : contentLayer->getChildrenExt()) {
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

    handleTouchPriority(this);

    return true;
}

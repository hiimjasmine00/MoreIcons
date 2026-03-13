#include "FramePresetPopup.hpp"
#include "../../misc/LazyIcon.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Defaults.hpp"
#include "../../../utils/Get.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/ui/Scrollbar.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

FramePresetPopup* FramePresetPopup::create(IconType type, Function<void(int, IconInfo*, int)> callback) {
    auto ret = new FramePresetPopup();
    if (ret->init(type, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool FramePresetPopup::init(IconType type, Function<void(int, IconInfo*, int)> callback) {
    if (!BasePopup::init(440.0f, 290.0f, "geode.loader/GE_square03.png", CircleBaseColor::DarkPurple)) return false;

    setID("FramePresetPopup");
    setTitle(fmt::format("{} Presets", Constants::getSingularUppercase(type)));
    m_title->setID("frame-preset-title");

    m_callback = std::move(callback);

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

    auto count = Get::gameManager->countForType(type);
    for (int i = 2; i <= count; i++) {
        for (int j = 1; j <= Defaults::getShipFireCount(i); j++) {
            auto iconMenu = CCMenu::create();
            auto lazyIcon = LazyIcon::create(type, i, nullptr, fmt::format("_{:03}", j), [this, i, j] {
                if (m_callback) m_callback(i, nullptr, j);
                close();
            });
            lazyIcon->setPosition({ 15.0f, 30.0f });
            iconMenu->setContentSize({ 30.0f, 30.0f });
            iconMenu->ignoreAnchorPointForPosition(false);
            iconMenu->setID(fmt::format("{}-menu", lazyIcon->getID()));
            iconMenu->addChild(lazyIcon);
            contentLayer->addChild(iconMenu);
        }
    }

    if (auto icons = more_icons::getIcons(type)) {
        for (auto& info : *icons) {
            for (int j = 1; j <= info.getFireCount(); j++) {
                auto iconMenu = CCMenu::create();
                auto lazyIcon = LazyIcon::create(type, 0, &info, fmt::format("_{:03}", j), [this, info = &info, j] {
                    if (m_callback) m_callback(0, info, j);
                    close();
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

    contentLayer->setLayout(RowLayout::create()->setGap(Constants::getIconGap(type))->setGrowCrossAxis(true));

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

    handleTouchPriority(this);

    return true;
}

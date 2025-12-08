#include "LogLayer.hpp"
#include "LogCell.hpp"
#include "../../../MoreIcons.hpp"
#include <Geode/ui/Scrollbar.hpp>
#include <Geode/ui/ScrollLayer.hpp>

using namespace geode::prelude;

LogLayer* LogLayer::create(IconType type) {
    auto ret = new LogLayer();
    if (ret->initAnchored(440.0f, 290.0f, type, "geode.loader/GE_square03.png")) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LogLayer::setup(IconType type) {
    setID("LogLayer");
    setTitle(fmt::format("{} Logs", MoreIcons::uppercase[MoreIcons::convertType(type)]));
    m_title->setID("more-icons-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    auto scrollBackground = CCLayerColor::create({ 0, 0, 0, 105 }, 400.0f, 230.0f);
    scrollBackground->setPosition({ 215.0f, 135.0f });
    scrollBackground->ignoreAnchorPointForPosition(false);
    scrollBackground->setID("scroll-background");
    m_mainLayer->addChild(scrollBackground);

    auto scrollLayer = ScrollLayer::create({ 400.0f, 230.0f });
    auto contentLayer = scrollLayer->m_contentLayer;
    scrollLayer->setPosition({ 215.0f, 135.0f });
    scrollLayer->ignoreAnchorPointForPosition(false);
    contentLayer->setLayout(ColumnLayout::create()->setAxisReverse(true)
        ->setAxisAlignment(AxisAlignment::End)->setAutoGrowAxis(0.0f)->setGap(0.0f));
    scrollLayer->setID("scroll-layer");
    m_mainLayer->addChild(scrollLayer);

    int i = 0;
    for (auto& log : MoreIcons::logs) {
        if (log.type == type) contentLayer->addChild(LogCell::create(log.name.c_str(), log.message, log.severity, i++));
    }

    contentLayer->updateLayout();
    scrollLayer->scrollToTop();

    auto scrollable = contentLayer->getContentHeight() > scrollLayer->getContentHeight();
    scrollLayer->enableScrollWheel(scrollable);

    auto scrollbar = Scrollbar::create(scrollLayer);
    scrollbar->setPosition({ 425.0f, 135.0f });
    scrollbar->setTouchEnabled(scrollable);
    scrollbar->setID("scrollbar");
    m_mainLayer->addChild(scrollbar);

    handleTouchPriority(this);

    return true;
}

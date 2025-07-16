#include "LogLayer.hpp"
#include "LogCell.hpp"
#include "../../scroll/BiggerScrollLayer.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/ui/Scrollbar.hpp>

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
    setTitle(fmt::format("{} Logs", MoreIcons::uppercase[MoreIconsAPI::convertType(type)]));
    m_title->setID("more-icons-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    auto scrollLayer = BiggerScrollLayer::create(400.0f, 230.0f);
    scrollLayer->m_contentLayer->setLayout(ColumnLayout::create()->setAxisReverse(true)
        ->setAxisAlignment(AxisAlignment::End)->setAutoGrowAxis(230.0f)->setGap(0.0f));
    scrollLayer->m_contentLayer->setTouchEnabled(false);
    scrollLayer->setPosition({ 215.0f, 135.0f });
    scrollLayer->setID("scroll-layer");
    m_mainLayer->addChild(scrollLayer);

    auto scrollbar = Scrollbar::create(scrollLayer);
    scrollbar->setPosition({ 425.0f, 135.0f });
    scrollbar->setID("scrollbar");
    m_mainLayer->addChild(scrollbar);

    int i = 0;
    for (auto& log : MoreIcons::logs) {
        if (log.type != type) continue;
        scrollLayer->m_contentLayer->addChild(LogCell::create(log.name, log.message, log.severity, i++));
    }

    scrollLayer->m_contentLayer->updateLayout();
    scrollLayer->scrollToTop();

    handleTouchPriority(this);

    return true;
}

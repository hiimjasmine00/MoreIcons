#include "LogLayer.hpp"
#include "LogCell.hpp"
#include "BiggerScrollLayer.hpp"
#include "../MoreIcons.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/utils/ranges.hpp>
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
    constexpr std::array titles = {
        "", "Icon", "", "", "Ship", "Ball", "UFO", "Wave", "Robot",
        "Spider", "Trail", "Death Effect", "", "Swing", "Jetpack", "Ship Fire"
    };
    auto unlock = (int)GameManager::get()->iconTypeToUnlockType(type);

    setID("LogLayer");
    setTitle(fmt::format("{} Logs", titles[unlock]));
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

    auto logs = ranges::filter(MoreIcons::logs, [type](const LogData& log) { return log.type == type; });
    std::ranges::sort(logs, [](const LogData& a, const LogData& b) {
        return a.severity == b.severity ? a.name < b.name : a.severity < b.severity;
    });
    for (int i = 0; i < logs.size(); i++) {
        auto& log = logs[i];
        auto cell = LogCell::create(log.name, log.message, log.severity, i);
        cell->setID(fmt::format("log-cell-{}", i + 1));
        scrollLayer->m_contentLayer->addChild(cell);
    }

    scrollLayer->m_contentLayer->updateLayout();
    scrollLayer->scrollToTop();

    return true;
}

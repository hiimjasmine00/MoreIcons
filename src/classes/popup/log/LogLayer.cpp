#include "LogLayer.hpp"
#include "LogCell.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Log.hpp"
#include <Geode/ui/Scrollbar.hpp>
#include <Geode/ui/ScrollLayer.hpp>

using namespace geode::prelude;

LogLayer* LogLayer::create(IconType type) {
    auto ret = new LogLayer();
    if (ret->init(type)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LogLayer::init(IconType type) {
    if (!BasePopup::init(440.0f, 290.0f, "geode.loader/GE_square03.png")) return false;

    setID("LogLayer");
    setTitle(fmt::format("{} Logs", Constants::getSingularUppercase(type)));
    m_title->setID("more-icons-title");

    auto scrollBackground = CCLayerColor::create({ 0, 0, 0, 105 }, 400.0f, 230.0f);
    scrollBackground->setPosition({ 215.0f, 135.0f });
    scrollBackground->ignoreAnchorPointForPosition(false);
    scrollBackground->setID("scroll-background");
    m_mainLayer->addChild(scrollBackground);

    auto scrollLayer = ScrollLayer::create({ 400.0f, 230.0f });
    auto contentLayer = scrollLayer->m_contentLayer;
    scrollLayer->setPosition({ 215.0f, 135.0f });
    scrollLayer->ignoreAnchorPointForPosition(false);
    contentLayer->ignoreAnchorPointForPosition(false);
    scrollLayer->setID("scroll-layer");
    m_mainLayer->addChild(scrollLayer);

    if (auto it = Log::logs.find(type); it != Log::logs.end()) {
        auto light = true;
        for (auto& log : it->second) {
            contentLayer->addChild(LogCell::create(log.name, log.message, log.severity, light));
            light = !light;
        }
    }

    contentLayer->setLayout(ColumnLayout::create()->setAxisReverse(true)->setAxisAlignment(AxisAlignment::End)->setAutoGrowAxis(0.0f)->setGap(0.0f));
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

#include "LogLayer.hpp"
#include "LogCell.hpp"
#include "../MoreIcons.hpp"
#include <Geode/binding/SetIDPopup.hpp>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

LogLayer* LogLayer::create() {
    auto ret = new LogLayer();
    if (ret->initAnchored(440.0f, 290.0f, "GJ_square04.png")) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LogLayer::setup() {
    setID("LogLayer");
    setTitle("More Icons");
    m_title->setID("more-icons-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    auto background = CCScale9Sprite::create("square02_001.png", { 0, 0, 80, 80 });
    background->setContentSize({ 400.0f, 230.0f });
    background->setPosition({ 220.0f, 135.0f });
    background->setOpacity(127);
    background->setID("scroll-background");
    m_mainLayer->addChild(background);

    m_scrollLayer = ScrollLayer::create({ 400.0f, 230.0f });
    m_scrollLayer->setPosition({ 20.0f, 20.0f });
    m_scrollLayer->m_contentLayer->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoGrowAxis(230.0f)
            ->setGap(0.0f)
    );
    m_scrollLayer->setID("scroll-layer");
    m_mainLayer->addChild(m_scrollLayer);

    m_prevButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_01_001.png", 1.0f, [this](auto) {
        page(m_page - 1);
    });
    m_prevButton->setPosition({ -34.5f, 145.0f });
    m_prevButton->setID("prev-button");
    m_buttonMenu->addChild(m_prevButton);

    auto nextButtonSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    nextButtonSprite->setFlipX(true);
    m_nextButton = CCMenuItemExt::createSpriteExtra(nextButtonSprite, [this](auto) {
        page(m_page + 1);
    });
    m_nextButton->setPosition({ 474.5f, 145.0f });
    m_nextButton->setID("next-button");
    m_buttonMenu->addChild(m_nextButton);

    auto countLabel = CCLabelBMFont::create("", "goldFont.fnt");
    countLabel->setScale(0.5f);
    m_pageButton = CCMenuItemExt::createSpriteExtra(countLabel, [this](auto) {
        auto popup = SetIDPopup::create(m_page + 1, 1, (MoreIcons::logs.size() + 24) / 25, "Go To Page", "Go", true, 1, 60.0f, false, false);
        popup->m_delegate = this;
        popup->show();
    });
    m_pageButton->setID("page-button");
    m_buttonMenu->addChild(m_pageButton);

    page(0);

    return true;
}

void LogLayer::page(int page) {
    m_page = page;

    auto& vec = MoreIcons::logs;
    auto size = vec.size();
    m_prevButton->setVisible(page > 0);
    m_nextButton->setVisible(page < (size - 1) / 25);

    m_scrollLayer->m_contentLayer->removeAllChildren();

    auto dark = Loader::get()->isModLoaded("bitz.darkmode_v4");
    for (int i = page * 25; i < page * 25 + 25 && i < size; i++) {
        m_scrollLayer->m_contentLayer->addChild(LogCell::create(vec[i].message, vec[i].severity, i, size, dark));
    }

    m_scrollLayer->m_contentLayer->updateLayout();
    m_scrollLayer->scrollToTop();

    auto countLabel = static_cast<CCLabelBMFont*>(m_pageButton->getNormalImage());
    countLabel->setString(fmt::format("Page {} of {}", page + 1, (size + 24) / 25).c_str());
    auto& countSize = countLabel->getContentSize();
    m_pageButton->setPosition({ 435.0f - countSize.width * 0.25f, 7.0f + countSize.height * 0.25f });
    m_pageButton->updateSprite();
}

void LogLayer::keyDown(enumKeyCodes key) {
    switch (key) {
        case KEY_Left:
        case CONTROLLER_Left:
            if (m_prevButton->isVisible()) page(m_page - 1);
            break;
        case KEY_Right:
        case CONTROLLER_Right:
            if (m_nextButton->isVisible()) page(m_page + 1);
            break;
        default:
            Popup::keyDown(key);
            break;
    }
}

void LogLayer::setIDPopupClosed(SetIDPopup*, int id) {
    page(std::min(std::max(id - 1, 0), (int)(MoreIcons::logs.size() - 1) / 25));
}

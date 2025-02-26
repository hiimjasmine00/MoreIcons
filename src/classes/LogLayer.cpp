#include "LogLayer.hpp"
#include "LogCell.hpp"
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
    setTitle("More Icons");

    auto background = CCScale9Sprite::create("square02_001.png", { 0, 0, 80, 80 });
    background->setContentSize({ 400.0f, 230.0f });
    background->setPosition({ 220.0f, 135.0f });
    background->setOpacity(127);
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
    m_mainLayer->addChild(m_scrollLayer);

    m_prevButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_01_001.png", 1.0f, [this](auto) {
        page(m_page - 1);
    });
    m_prevButton->setPosition({ -34.5f, 145.0f });
    m_buttonMenu->addChild(m_prevButton);

    auto nextButtonSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    nextButtonSprite->setFlipX(true);
    m_nextButton = CCMenuItemExt::createSpriteExtra(nextButtonSprite, [this](auto) {
        page(m_page + 1);
    });
    m_nextButton->setPosition({ 474.5f, 145.0f });
    m_buttonMenu->addChild(m_nextButton);

    page(0);

    auto topButtons = CCMenu::create();
    topButtons->setPosition({ 420.0f, 270.0f });
    topButtons->setContentSize({ 100.0f, 30.0f });
    topButtons->setAnchorPoint({ 1.0f, 0.5f });
    topButtons->setLayout(
        RowLayout::create()
            ->setAxisAlignment(AxisAlignment::End)
            ->setAxisReverse(true)
    );
    m_mainLayer->addChild(topButtons);

    topButtons->addChild(CCMenuItemExt::createSpriteExtraWithFrameName("GJ_infoIcon_001.png", 1.0f, [this](auto) {
        MoreIcons::showInfoPopup();
    }));
    topButtons->addChild(CCMenuItemExt::createSpriteExtraWithFrameName("folderIcon_001.png", 0.7f, [this](auto) {
        file::openFolder(Mod::get()->getConfigDir());
    }));
    topButtons->updateLayout();

    return true;
}

void LogLayer::page(int page) {
    m_page = page;

    auto& vec = MoreIcons::LOGS;
    m_prevButton->setVisible(page > 0);
    m_nextButton->setVisible(page < (vec.size() - 1) / 10);

    m_scrollLayer->m_contentLayer->removeAllChildren();

    auto size = std::min(10, (int)vec.size() - page * 10);
    auto dark = Loader::get()->isModLoaded("bitz.darkmode_v4");
    for (int i = page * 10; i < page * 10 + size; i++) {
        m_scrollLayer->m_contentLayer->addChild(LogCell::create(vec[i], i, size, dark));
    }

    m_scrollLayer->m_contentLayer->updateLayout();
    m_scrollLayer->scrollToTop();
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
            Popup<>::keyDown(key);
            break;
    }
}

#include "ViewShipFirePopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Defaults.hpp"
#include "../../../utils/Filesystem.hpp"
#include "../../../utils/Load.hpp"
#include "../../../utils/Notify.hpp"
#include <MoreIcons.hpp>

using namespace geode::prelude;

ViewShipFirePopup* ViewShipFirePopup::create(int id, IconInfo* info) {
    auto ret = new ViewShipFirePopup();
    if (ret->init(id, info)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ViewShipFirePopup::init(int id, IconInfo* info) {
    if (!BasePopup::init(400.0f, 100.0f, "geode.loader/GE_square03.png", CircleBaseColor::DarkPurple)) return false;

    setID("ViewShipFirePopup");
    setTitle("Ship Fire Viewer");
    m_title->setID("view-ship-fire-title");

    m_streak = CCSprite::create();
    m_streak->setPosition({ 200.0f, 20.0f });
    m_streak->setRotation(-90.0f);
    m_streak->setID("streak-preview");
    m_mainLayer->addChild(m_streak);

    auto framesBackground = NineSlice::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    framesBackground->setPosition({ 200.0f, 50.0f });
    framesBackground->setContentSize({ 300.0f, 30.0f });
    framesBackground->setOpacity(105);
    framesBackground->setID("frames-background");
    m_mainLayer->addChild(framesBackground);

    m_frameMenu = CCMenu::create();
    m_frameMenu->setPosition({ 200.0f, 50.0f });
    m_frameMenu->setContentSize({ 300.0f, 30.0f });
    m_frameMenu->ignoreAnchorPointForPosition(false);
    m_frameMenu->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Even), false);
    m_frameMenu->setID("frame-menu");
    m_mainLayer->addChild(m_frameMenu);

    m_selectSprite = CCSprite::createWithSpriteFrameName("GJ_select_001.png");
    m_selectSprite->setID("select-sprite");
    m_mainLayer->addChild(m_selectSprite);

    auto prevSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    prevSprite->setScale(0.5f);
    auto prevButton = CCMenuItemSpriteExtra::create(prevSprite, this, menu_selector(ViewShipFirePopup::onPrev));
    prevButton->setPosition({ 40.0f, 50.0f });
    prevButton->setID("prev-button");
    m_buttonMenu->addChild(prevButton);

    auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    nextSprite->setScale(0.5f);
    nextSprite->setFlipX(true);
    auto nextButton = CCMenuItemSpriteExtra::create(nextSprite, this, menu_selector(ViewShipFirePopup::onNext));
    nextButton->setPosition({ 360.0f, 50.0f });
    nextButton->setID("next-button");
    m_buttonMenu->addChild(nextButton);

    auto path = MoreIcons::getIconPath(info, id, IconType::ShipFire);
    auto count = info ? info->getFireCount() : Defaults::getShipFireCount(id);

    auto& pathString = Filesystem::getPathString(path);
    CCMenuItemSpriteExtra* selected = nullptr;
    auto failed = false;

    for (int i = 1; i <= count; i++) {
        pathString.replace(pathString.size() - 7, 3, fmt::format(L("{:03}"), i));
        auto textureRes = Load::createTexture(path);
        if (textureRes.isErr()) {
            Notify::error("Failed to load {}: {}", Filesystem::filenameFormat(path), textureRes.unwrapErr());
            failed = true;
            break;
        }
        auto button = addFrameButton(textureRes.unwrap());
        if (!selected) selected = button;
    }

    if (!failed && selected) {
        m_page = m_selectedFrame / 5;
        updateState();
        onFrameSelect(selected);
    }

    handleTouchPriority(this);

    return true;
}

CCMenuItemSpriteExtra* ViewShipFirePopup::addFrameButton(CCTexture2D* texture) {
    auto index = m_frameButtons.size();
    auto frame = CCSprite::createWithTexture(texture, { { 0.0f, 0.0f }, texture ? texture->getContentSize() : CCSize { 0.0f, 0.0f }});
    limitNodeHeight(frame, 30.0f, 1.0f, 0.0f);
    auto button = CCMenuItemSpriteExtra::create(frame, this, menu_selector(ViewShipFirePopup::onFrameSelect));
    button->setContentSize({ 30.0f, 30.0f });
    frame->setPosition({ 15.0f, 15.0f });
    button->setTag(index);
    button->setID(fmt::format("frame-{}-button", index + 1));
    m_frameMenu->addChild(button);
    m_frameButtons.push_back(button);
    return button;
}

void ViewShipFirePopup::updateState() {
    m_selectSprite->setVisible(m_selectedFrame >= m_page * 5 && m_selectedFrame < (m_page + 1) * 5);
    for (size_t i = 0; i < m_frameButtons.size(); i++) {
        m_frameButtons[i]->setVisible(i >= m_page * 5 && i < (m_page + 1) * 5);
    }
    m_frameMenu->updateLayout();
}

void ViewShipFirePopup::onPrev(CCObject* sender) {
    if (m_page <= 0) m_page = (m_frameButtons.size() - 1) / 5;
    else m_page--;
    updateState();
}

void ViewShipFirePopup::onNext(CCObject* sender) {
    if (m_page >= (m_frameButtons.size() - 1) / 5) m_page = 0;
    else m_page++;
    updateState();
}

void ViewShipFirePopup::onFrameSelect(CCObject* sender) {
    m_selectedFrame = sender->getTag();
    m_selectSprite->setVisible(true);
    m_selectSprite->setPosition(m_mainLayer->convertToNodeSpace(m_frameMenu->convertToWorldSpace(static_cast<CCNode*>(sender)->getPosition())));
    MoreIcons::setTexture(m_streak, static_cast<CCSprite*>(static_cast<CCMenuItemSpriteExtra*>(sender)->getNormalImage())->getTexture());
    auto& size = m_streak->getContentSize();
    m_streak->setScaleX(20.0f / size.width);
    m_streak->setScaleY(80.0f / size.height);
}

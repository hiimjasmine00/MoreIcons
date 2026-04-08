#include "ViewDeathEffectPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Load.hpp"
#include "../../../utils/Notify.hpp"
#include <MoreIcons.hpp>

using namespace geode::prelude;

ViewDeathEffectPopup* ViewDeathEffectPopup::create(int id, IconInfo* info) {
    auto ret = new ViewDeathEffectPopup();
    if (ret->init(id, info)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ViewDeathEffectPopup::init(int id, IconInfo* info) {
    if (!BasePopup::init(400.0f, 200.0f, "geode.loader/GE_square03.png", CircleBaseColor::DarkPurple)) return false;

    setID("ViewDeathEffectPopup");
    setTitle("Death Effect Viewer");
    m_title->setID("view-death-effect-title");

    m_previewSprite = CCSprite::create();
    m_previewSprite->setPosition({ 200.0f, 70.0f });
    m_previewSprite->setID("preview-sprite");
    m_mainLayer->addChild(m_previewSprite);

    auto framesBackground = NineSlice::create("square02_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    framesBackground->setPosition({ 200.0f, 150.0f });
    framesBackground->setContentSize({ 300.0f, 30.0f });
    framesBackground->setOpacity(105);
    framesBackground->setID("frames-background");
    m_mainLayer->addChild(framesBackground);

    m_frameMenu = CCMenu::create();
    m_frameMenu->setPosition({ 200.0f, 150.0f });
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
    auto prevButton = CCMenuItemSpriteExtra::create(prevSprite, this, menu_selector(ViewDeathEffectPopup::onPrev));
    prevButton->setPosition({ 40.0f, 150.0f });
    prevButton->setID("prev-button");
    m_buttonMenu->addChild(prevButton);

    auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    nextSprite->setScale(0.5f);
    nextSprite->setFlipX(true);
    auto nextButton = CCMenuItemSpriteExtra::create(nextSprite, this, menu_selector(ViewDeathEffectPopup::onNext));
    nextButton->setPosition({ 360.0f, 150.0f });
    nextButton->setID("next-button");
    m_buttonMenu->addChild(nextButton);

    std::filesystem::path png;
    std::filesystem::path plist;
    MoreIcons::getIconPaths(info, id, IconType::DeathEffect, png, plist);
    if (auto imageRes = Load::createFrames(png, plist, {}, IconType::DeathEffect, {}, true)) {
        auto image = std::move(imageRes).unwrap();
        Load::initTexture(image, true);

        prevButton->setVisible(image.frames.size() > 5);
        nextButton->setVisible(image.frames.size() > 5);

        std::vector<std::string_view> keys;
        for (auto& frame : image.frames) {
            keys.push_back(frame.first);
        }
        std::ranges::sort(keys);
        CCMenuItemSpriteExtra* selected = nullptr;
        for (size_t i = 0; i < keys.size(); i++) {
            auto it = image.frames.find(keys[i]);
            if (it != image.frames.end()) {
                auto button = addFrameButton(it->second);
                if (i == m_selectedFrame) selected = button;
                m_frames.push_back(std::move(it->second));
                image.frames.erase(it);
            }
        }
        if (!selected) selected = m_frameButtons.back();

        updateState();
        onFrameSelect(selected);
    }
    else if (imageRes.isErr()) {
        Notify::error(imageRes.unwrapErr());
    }

    handleTouchPriority(this);

    return true;
}

CCMenuItemSpriteExtra* ViewDeathEffectPopup::addFrameButton(CCSpriteFrame* frame) {
    auto index = m_frameButtons.size();
    auto sprite = CCSprite::createWithSpriteFrame(frame);
    limitNodeSize(sprite, { 30.0f, 30.0f }, 1.0f, 0.0f);
    auto button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(ViewDeathEffectPopup::onFrameSelect));
    button->setContentSize({ 30.0f, 30.0f });
    sprite->setPosition({ 15.0f, 15.0f });
    button->setTag(index);
    button->setID(fmt::format("frame-{}-button", index + 1));
    m_frameMenu->addChild(button);
    m_frameButtons.push_back(button);
    return button;
}

void ViewDeathEffectPopup::updateState() {
    m_selectSprite->setVisible(m_selectedFrame >= m_page * 5 && m_selectedFrame < (m_page + 1) * 5);
    for (size_t i = 0; i < m_frameButtons.size(); i++) {
        m_frameButtons[i]->setVisible(i >= m_page * 5 && i < (m_page + 1) * 5);
    }
    m_frameMenu->updateLayout();
}

void ViewDeathEffectPopup::onPrev(CCObject* sender) {
    if (m_page <= 0) m_page = (m_frameButtons.size() - 1) / 5;
    else m_page--;
    updateState();
}

void ViewDeathEffectPopup::onNext(CCObject* sender) {
    if (m_page >= (m_frameButtons.size() - 1) / 5) m_page = 0;
    else m_page++;
    updateState();
}

void ViewDeathEffectPopup::onFrameSelect(CCObject* sender) {
    m_selectedFrame = sender->getTag();
    m_selectSprite->setVisible(true);
    m_selectSprite->setPosition(m_mainLayer->convertToNodeSpace(m_frameMenu->convertToWorldSpace(static_cast<CCNode*>(sender)->getPosition())));
    m_previewSprite->setDisplayFrame(m_frames[m_selectedFrame]);
    limitNodeHeight(m_previewSprite, 100.0f, 1.0f, 0.0f);
}

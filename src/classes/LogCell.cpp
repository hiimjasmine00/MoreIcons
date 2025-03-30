#include "LogCell.hpp"
#include <Geode/binding/MultilineBitmapFont.hpp>
#include <Geode/binding/TextArea.hpp>

using namespace geode::prelude;

LogCell* LogCell::create(const std::string& message, Severity severity, int index, int total, bool dark) {
    auto ret = new LogCell();
    if (ret->init(message, severity, index, total, dark)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LogCell::init(const std::string& message, Severity severity, int index, int total, bool dark) {
    if (!CCLayer::init()) return false;

    setID("LogCell");
    ignoreAnchorPointForPosition(false);
    setContentSize({ 400.0f, 70.0f });

    m_index = index;
    m_total = total;

    auto bg = CCLayerColor::create({ 0, 0, 0, 255 }, 400.0f, 70.0f);
    if (dark) bg->setColor(index % 2 == 0 ? ccColor3B { 48, 48, 48 } : ccColor3B { 80, 80, 80 });
    else bg->setColor(index % 2 == 0 ? ccColor3B { 161, 88, 44 } : ccColor3B { 194, 114, 62 });
    bg->ignoreAnchorPointForPosition(false);
    if (index % 25 == 0) {
        bg->setContentSize({ 400.0f, 35.0f });
        bg->setPosition({ 200.0f, 17.5f });
    }
    else if (index % 25 == 24 || index == total - 1) {
        bg->setContentSize({ 400.0f, 35.0f });
        bg->setPosition({ 200.0f, 52.5f });
    }
    else {
        bg->setContentSize({ 400.0f, 70.0f });
        bg->setPosition({ 200.0f, 35.0f });
    }
    bg->setID("background");
    addChild(bg, -1);

    if (index % 25 == 0 || index % 25 == 24 || index == total - 1) {
        auto bgBg = CCScale9Sprite::create("square02b_001.png", { 0, 0, 80, 80 });
        bgBg->setContentSize({ 400.0f, 70.0f });
        bgBg->setPosition({ 200.0f, 35.0f });
        bgBg->setColor(bg->getColor());
        bgBg->setID("background-corners");
        addChild(bgBg, -2);
    }

    auto infoIcon = CCSprite::createWithSpriteFrameName(frames[severity - 1]);
    infoIcon->setPosition({ 20.0f, 35.0f });
    infoIcon->setID("info-icon");
    addChild(infoIcon);

    auto textArea = TextArea::create(message, "bigFont.fnt", 0.25f, 350.0f, { 0.0f, 1.0f }, 10.0f, true);
    textArea->setContentSize({ textArea->m_width, textArea->m_height * (textArea->m_label->m_lines ? textArea->m_label->m_lines->count() : 0) });
    textArea->m_label->setPosition({ 0.0f, textArea->getContentHeight() });
    textArea->setPosition({ 40.0f, 35.0f });
    textArea->setAnchorPoint({ 0.0f, 0.5f });
    textArea->setID("text-area");
    addChild(textArea);

    return true;
}

void LogCell::draw() {
    ccDrawColor4B(0, 0, 0, 75);
    glLineWidth(2.0f);
    if (m_index < m_total - 1) ccDrawLine({ 0.0f, 0.0f }, { 400.0f, 0.0f });
    if (m_index > 0) ccDrawLine({ 0.0f, 70.0f }, { 400.0f, 70.0f });
}

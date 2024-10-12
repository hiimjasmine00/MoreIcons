#include "LogCell.hpp"

using namespace geode::prelude;

LogCell* LogCell::create(LogData const& data, int index, int total, bool dark) {
    auto ret = new LogCell();
    if (ret->init(data, index, total, dark)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LogCell::init(LogData const& data, int index, int total, bool dark) {
    if (!CCLayer::init()) return false;

    ignoreAnchorPointForPosition(false);

    setContentSize({ 400.0f, 70.0f });

    m_index = index;
    m_total = total;

    auto bg = CCLayerColor::create({ 0, 0, 0, 255 }, 400.0f, 70.0f);
    if (dark) bg->setColor(index % 2 == 0 ? ccColor3B { 48, 48, 48 } : ccColor3B { 80, 80, 80 });
    else bg->setColor(index % 2 == 0 ? ccColor3B { 161, 88, 44 } : ccColor3B { 194, 114, 62 });
    bg->ignoreAnchorPointForPosition(false);
    if (index == 0) {
        bg->setContentSize({ 400.0f, 35.0f });
        bg->setPosition(200.0f, 17.5f);
    }
    else if (index == total - 1) {
        bg->setContentSize({ 400.0f, 35.0f });
        bg->setPosition(200.0f, 52.5f);
    }
    else {
        bg->setContentSize({ 400.0f, 70.0f });
        bg->setPosition(200.0f, 35.0f);
    }
    addChild(bg, -1);

    if (index == 0 || index == total - 1) {
        auto bgBg = CCScale9Sprite::create("square02b_001.png", { 0, 0, 80, 80 });
        bgBg->setContentSize({ 400.0f, 70.0f });
        bgBg->setPosition(200.0f, 35.0f);
        bgBg->setColor(bg->getColor());
        addChild(bgBg, -2);
    }

    auto infoFrame = "";
    switch (data.type) {
        case LogType::Info: infoFrame = "GJ_infoIcon_001.png"; break;
        case LogType::Warn: infoFrame = "geode.loader/info-warning.png"; break;
        case LogType::Error: infoFrame = "geode.loader/info-alert.png"; break;
    }

    auto infoIcon = CCSprite::createWithSpriteFrameName(infoFrame);
    infoIcon->setPosition({ 20.0f, 35.0f });
    addChild(infoIcon);

    auto textArea = TextArea::create(data.message, "bigFont.fnt", 0.25f, 350.0f, { 0.0f, 1.0f }, 10.0f, true);
    textArea->setContentSize({ textArea->m_width, textArea->m_height * (textArea->m_label->m_lines ? textArea->m_label->m_lines->count() : 0) });
    textArea->m_label->setPosition({ 0.0f, textArea->getContentHeight() });
    textArea->setPosition({ 40.0f, 35.0f });
    textArea->setAnchorPoint({ 0.0f, 0.5f });
    addChild(textArea);

    return true;
}

void LogCell::draw() {
    ccDrawColor4B(0, 0, 0, 75);
    glLineWidth(2.0f);
    if (m_index < m_total - 1) ccDrawLine({ 0.0f, 0.0f }, { 400.0f, 0.0f });
    if (m_index > 0) ccDrawLine({ 0.0f, 70.0f }, { 400.0f, 70.0f });
}

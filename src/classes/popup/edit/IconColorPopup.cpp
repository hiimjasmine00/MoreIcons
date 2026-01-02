#include "IconColorPopup.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Get.hpp"
#include <Geode/binding/ButtonSprite.hpp>

using namespace geode::prelude;

IconColorPopup* IconColorPopup::create(int selected, std23::move_only_function<void(int)> callback) {
    auto ret = new IconColorPopup();
    if (ret->init(selected, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

int colorForIndex(int index) {
    switch (index) {
        case 0: return 51;
        case 1: return 19;
        case 2: return 48;
        case 3: return 9;
        case 4: return 62;
        case 5: return 63;
        case 6: return 10;
        case 7: return 29;
        case 9: return 70;
        case 10: return 42;
        case 11: return 11;
        case 12: return 27;
        case 13: return 72;
        case 14: return 73;
        case 15: return 0;
        case 16: return 1;
        case 18: return 37;
        case 19: return 53;
        case 20: return 54;
        case 21: return 55;
        case 22: return 26;
        case 23: return 59;
        case 24: return 60;
        case 25: return 61;
        case 27: return 71;
        case 28: return 14;
        case 29: return 31;
        case 30: return 45;
        case 31: return 105;
        case 32: return 28;
        case 33: return 32;
        case 34: return 20;
        case 36: return 25;
        case 37: return 56;
        case 38: return 57;
        case 39: return 58;
        case 40: return 30;
        case 41: return 64;
        case 42: return 65;
        case 43: return 66;
        case 45: return 46;
        case 46: return 67;
        case 47: return 68;
        case 48: return 69;
        case 49: return 2;
        case 50: return 38;
        case 51: return 79;
        case 52: return 80;
        case 54: return 74;
        case 55: return 75;
        case 56: return 44;
        case 57: return 3;
        case 58: return 83;
        case 59: return 16;
        case 60: return 4;
        case 61: return 5;
        case 63: return 52;
        case 64: return 41;
        case 65: return 6;
        case 66: return 35;
        case 67: return 98;
        case 68: return 8;
        case 69: return 36;
        case 70: return 103;
        case 72: return 40;
        case 73: return 76;
        case 74: return 77;
        case 75: return 78;
        case 76: return 22;
        case 77: return 39;
        case 78: return 84;
        case 79: return 50;
        case 81: return 47;
        case 82: return 23;
        case 83: return 92;
        case 84: return 93;
        case 85: return 7;
        case 86: return 13;
        case 87: return 24;
        case 88: return 104;
        case 90: return 33;
        case 91: return 21;
        case 92: return 81;
        case 93: return 82;
        case 94: return 34;
        case 95: return 85;
        case 96: return 86;
        case 97: return 87;
        case 99: return 49;
        case 100: return 95;
        case 101: return 96;
        case 102: return 97;
        case 103: return 43;
        case 104: return 99;
        case 105: return 100;
        case 106: return 101;
        case 112: return 106;
        case 113: return 88;
        case 114: return 89;
        case 115: return 90;
        case 136: return 12;
        case 137: return 91;
        case 138: return 17;
        case 139: return 102;
        case 140: return 18;
        case 141: return 94;
        case 142: return 15;
        default: return -1;
    }
}

CCPoint offsetForIndex(int index) {
    float x;
    float y;
    if (index > 53 && index < 136) y = -0.8f;
    else if (index > 135) y = 0.4f;
    else y = 0.0f;
    switch (index) {
        case 4: case 5: case 6: case 7: case 13: case 14: case 15: case 16: case 22: case 23:
        case 24: case 25: case 31: case 32: case 33: case 34: case 40: case 41: case 42: case 43:
        case 49: case 50: case 51: case 52: case 58: case 59: case 60: case 61: case 67: case 68:
        case 69: case 70: case 76: case 77: case 78: case 79: case 85: case 86: case 87: case 88:
        case 94: case 95: case 96: case 97: case 103: case 104: case 105: case 106: case 112: case 113:
        case 114: case 115: case 136: case 137: case 138: case 139: case 140: case 141: case 142:
            x = 1.0f;
            break;
        default:
            x = 0.0f;
            break;
    }
    return { x, y };
}

bool IconColorPopup::init(int selected, std23::move_only_function<void(int)> callback) {
    if (!BasePopup::init(450.0f, 270.0f, "geode.loader/GE_square03.png")) return false;

    setID("IconColorPopup");
    setTitle("Select Preview Color");
    m_title->setID("select-color-title");

    m_selected = selected;
    m_original = selected;
    m_callback = std::move(callback);

    auto selectSprite = CCSprite::createWithSpriteFrameName("GJ_select_001.png");
    selectSprite->setScale(0.7f);
    selectSprite->setID("select-sprite");
    m_mainLayer->addChild(selectSprite);

    auto center = Get::Director()->getWinSize() / 2.0f;
    for (int i = 0; i < 144; i++) {
        auto colorIndex = colorForIndex(i);
        if (colorIndex < 0) continue;
        auto colorSprite = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
        colorSprite->setScale(0.65f);
        colorSprite->setColor(Constants::getColor(colorIndex));
        auto colorButton = CCMenuItemExt::createSpriteExtra(colorSprite, [this, colorIndex, selectSprite](CCMenuItemSpriteExtra* sender) {
            selectSprite->setPosition(sender->getPosition());
            m_selected = colorIndex;
        });
        auto offset = offsetForIndex(i) * 12.0f;
        colorButton->setPosition(m_buttonMenu->convertToNodeSpace(center + offset + CCPoint {
            (i % 18) * 24.0f - 198.0f, 89.0f - floorf(i / 18.0f) * 24.0f
        }));
        colorButton->setID(fmt::to_string(colorIndex));
        m_buttonMenu->addChild(colorButton);
        if (colorIndex == selected) selectSprite->setPosition(colorButton->getPosition());
    }

    auto confirmButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Confirm", "goldFont.fnt", "GJ_button_05.png", 0.8f), [this](auto) {
        m_callback(m_selected);
        Popup::onClose(nullptr);
    });
    confirmButton->setPosition({ 225.0f, 25.0f });
    confirmButton->setID("confirm-button");
    m_buttonMenu->addChild(confirmButton);

    handleTouchPriority(this);

    return true;
}

void IconColorPopup::onClose(CCObject* sender) {
    if (m_selected != m_original) {
        createQuickPopup(
            "Exit Color Selector",
            "Are you sure you want to <cy>exit</c> the <cg>color selector</c>?",
            "No",
            "Yes",
            [this](auto, bool btn2) {
                if (btn2) Popup::onClose(nullptr);
            }
        );
    }
    else Popup::onClose(sender);
}

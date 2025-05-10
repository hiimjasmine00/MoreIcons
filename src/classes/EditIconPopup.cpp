#include "EditIconPopup.hpp"
#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/utils/ranges.hpp>
#include <rectpack2D/finders_interface.h>
#include <RenderTexture.hpp>

using namespace geode::prelude;
using namespace rectpack2D;

EditIconPopup* EditIconPopup::create(IconType type, int id, const std::string& name, bool read) {
    auto ret = new EditIconPopup();
    auto isIcon = type <= IconType::Jetpack;
    if (ret->initAnchored(
        350.0f,
        180.0f + isIcon * 50.0f + (type == IconType::Robot || type == IconType::Spider) * 30.0f - read * 60.0f,
        type,
        id,
        name,
        read
    )) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

constexpr std::array uppercase = {
    "", "Icon", "", "", "Ship", "Ball",
    "UFO", "Wave", "Robot", "Spider", "Trail",
    "Death Effect", "", "Swing", "Jetpack", "Ship Fire"
};

bool EditIconPopup::setup(IconType type, int id, const std::string& name, bool read) {
    auto gameManager = GameManager::get();
    auto unlockName = uppercase[(int)gameManager->iconTypeToUnlockType(type)];

    setID("EditIconPopup");
    setTitle(read ? name.empty() ? fmt::format("{} {:02}", unlockName, id) : name : fmt::format("{} Editor", unlockName));
    m_title->setID("edit-icon-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_frames = CCDictionary::create();
    m_frames->retain();
    m_sprites = CCDictionary::create();
    m_sprites->retain();
    m_frameMenus = CCArray::create();
    m_frameMenus->retain();
    m_iconType = type;

    auto isIcon = type <= IconType::Jetpack;
    if (isIcon) {
        std::vector<std::vector<std::string>> suffixes;
        auto isRobot = type == IconType::Robot || type == IconType::Spider;
        if (isRobot) {
            for (int i = 1; i < 5; i++) {
                std::vector<std::string> subSuffixes;
                subSuffixes.push_back(fmt::format("_{:02}_001.png", i));
                subSuffixes.push_back(fmt::format("_{:02}_2_001.png", i));
                subSuffixes.push_back(fmt::format("_{:02}_glow_001.png", i));
                if (i == 1) subSuffixes.push_back("_01_extra_001.png");
                suffixes.push_back(subSuffixes);
            }
        }
        else {
            std::vector<std::string> subFrames;
            subFrames.push_back("_001.png");
            subFrames.push_back("_2_001.png");
            if (type == IconType::Ufo) subFrames.push_back("_3_001.png");
            subFrames.push_back("_glow_001.png");
            subFrames.push_back("_extra_001.png");
            suffixes.push_back(subFrames);
        }

        auto iconName = name.empty() ? fmt::format("{}{:02}", MoreIcons::prefixes[(int)type], read ? id : 1) : fmt::format("{}"_spr, name);
        auto spriteFrameCache = CCSpriteFrameCache::get();
        for (int i = 0; i < suffixes.size(); i++) {
            auto frameMenu = CCMenu::create();
            frameMenu->setPosition({ 175.0f, 170.0f + isRobot * 40.0f - i * 30.0f + std::max(i - 1, 0) * 10.0f - read * 70.0f });
            frameMenu->setContentSize({ 350.0f, 30.0f });
            frameMenu->setEnabled(!read);
            frameMenu->setLayout(RowLayout::create()->setGap(10.0f * (read + 1.0f) - (type == IconType::Ufo) * 10.0f));
            frameMenu->setID(fmt::format("frame-menu-{}", i + 1));

            auto& subSuffixes = suffixes[i];
            auto crossFrame = spriteFrameCache->spriteFrameByName("GJ_deleteIcon_001.png");
            auto whiteFrame = spriteFrameCache->spriteFrameByName("cc_2x2_white_image");
            auto frameButtons = CCArray::create();
            for (int j = 0; j < subSuffixes.size(); j++) {
                auto& subSuffix = subSuffixes[j];

                auto spriteFrame = spriteFrameCache->spriteFrameByName((iconName + subSuffix).c_str());
                if (spriteFrame && spriteFrame->getTag() == 105871529) spriteFrame = nullptr;
                if (spriteFrame) m_frames->setObject(spriteFrame, subSuffix);

                auto sprite = CCSprite::createWithSpriteFrame(spriteFrame ? spriteFrame : crossFrame);
                m_sprites->setObject(sprite, subSuffix);

                auto button = CCMenuItemExt::createSpriteExtra(sprite, [this, i, isRobot, j](auto) { pickFile(i + isRobot, j + 1, false); });
                button->setID(fmt::format("frame-button-{}", j + 1));
                frameButtons->addObject(button);
                frameMenu->addChild(button);
            }

            frameMenu->updateLayout();

            for (int j = 0; j < frameButtons->count(); j++) {
                auto contentHeight = frameMenu->getContentHeight();
                auto frameButton = static_cast<CCMenuItemSprite*>(frameButtons->objectAtIndex(j));
                frameButton->setContentSize({ contentHeight, contentHeight });
                frameButton->getNormalImage()->setPosition({ contentHeight / 2.0f, contentHeight / 2.0f });
                if (!read) {
                    auto& subSuffix = subSuffixes[j];
                    auto resetSprite = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
                    resetSprite->setScale(i < 1 && type != IconType::Ufo ? contentHeight / resetSprite->getContentHeight() : 0.4f);
                    auto resetButton = CCMenuItemExt::createSpriteExtra(resetSprite, [this, i, j, subSuffix](auto) {
                        createQuickPopup(
                            "Reset Frame",
                            "Are you sure you want to <cy>reset</c> this <cg>frame</c>?",
                            "No",
                            "Yes",
                            [this, i, j, subSuffix](auto, bool btn2) {
                                if (!btn2) return;
                                auto spriteFrame = CCSpriteFrameCache::get()->spriteFrameByName(
                                    fmt::format("{}01{}", MoreIcons::prefixes[(int)m_iconType], subSuffix).c_str());
                                if (spriteFrame && spriteFrame->getTag() == 105871529) spriteFrame = nullptr;
                                if (spriteFrame) m_frames->setObject(spriteFrame, subSuffix);
                                else m_frames->removeObjectForKey(subSuffix);
                                updateSprites();
                            }
                        );
                    });
                    resetButton->setID(fmt::format("reset-button-{}", j + 1));
                    frameMenu->insertAfter(resetButton, frameButton);
                }
            }

            frameMenu->updateLayout();
            m_frameMenus->addObject(frameMenu);
            m_mainLayer->addChild(frameMenu);
        }

        m_player = SimplePlayer::create(1);
        if (!name.empty()) MoreIconsAPI::updateSimplePlayer(m_player, name, type);
        else m_player->updatePlayerFrame(id, type);
        m_player->setGlowOutline({ 255, 255, 255 });
        m_player->setPosition({ 175.0f, 150.0f + isRobot * 80.0f - suffixes.size() * 30.0f - read * 70.0f });
        m_player->setID("player-icon");
        m_mainLayer->addChild(m_player);
    }
    else if (type == IconType::Special) {
        auto icon = name.empty() ? nullptr : MoreIconsAPI::getIcon(name, type);

        auto stroke = 10.0f;
        auto tint = true;
        auto trailID = icon ? icon->trailID : id;
        if (trailID > 0) switch (trailID) {
            case 2:
            case 7:
                stroke = 14.0f;
                tint = false;
                break;
            case 3:
                stroke = 8.5f;
                break;
            case 4:
                stroke = 10.0f;
                break;
            case 5:
                stroke = 5.0f;
                break;
            case 6:
                stroke = 3.0f;
                break;
        } else {
            stroke = icon ? icon->stroke : 14.0f;
            tint = icon && icon->tint;
        }

        m_streak = CCSprite::create((name.empty() ?
            fmt::format("streak_{:02}_001.png", read ? id : 1) : icon ? icon->textures[0] : "streak_01_001.png").c_str());
        m_streak->setBlendFunc({
            GL_SRC_ALPHA,
            (uint32_t)GL_ONE_MINUS_SRC_ALPHA - (trailID > 0 || (icon && icon->blend)) * (uint32_t)GL_SRC_ALPHA
        });
        m_streak->setPosition({ 175.0f, 120.0f - read * 70.0f });
        m_streak->setRotation(-90.0f);
        m_streak->setScaleX(stroke / m_streak->getContentWidth());
        m_streak->setScaleY(320.0f / m_streak->getContentHeight());
        if (tint) m_streak->setColor(gameManager->colorForIdx(gameManager->m_playerColor2));
        m_streak->setID("streak-preview");
        m_mainLayer->addChild(m_streak);
    }

    if (read) return true;

    m_textInput = TextInput::create(300.0f, "Name");
    m_textInput->setPosition({ 175.0f, 75.0f });
    m_textInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    m_textInput->setMaxCharCount(100);
    m_textInput->setID("text-input");
    m_mainLayer->addChild(m_textInput);

    auto pngButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("PNG"), [this](auto) { pickFile(0, 0, false); });
    pngButton->setPosition({ 130.0f - isIcon * 45.0f, 30.0f });
    pngButton->setID("png-button");
    m_buttonMenu->addChild(pngButton);

    m_saveButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Save"), [this](auto) {
        saveIcon();
    });
    m_saveButton->setPosition({ 215.0f - isIcon * 40.0f, 30.0f });
    m_saveButton->setID("save-button");
    m_buttonMenu->addChild(m_saveButton);

    if (isIcon) {
        auto plistSprite = ButtonSprite::create("Plist");
        plistSprite->m_BGSprite->setOpacity(105);
        plistSprite->m_label->setOpacity(105);
        m_plistButton = CCMenuItemExt::createSpriteExtra(plistSprite, [this](auto) { pickFile(0, 0, true); });
        m_plistButton->setPosition({ 270.0f, 30.0f });
        m_plistButton->setEnabled(false);
        m_plistButton->setID("plist-button");
        m_buttonMenu->addChild(m_plistButton);
    }

    return true;
}

void EditIconPopup::pickFile(int index, int type, bool plist) {
    m_listener.bind([this, index, type, plist](Task<Result<std::filesystem::path>>::Event* e) {
        if (auto res = e->getValue(); res && res->isOk()) {
            m_path = res->unwrap();
            if ((plist && m_path.extension() != ".plist") || (!plist && m_path.extension() != ".png")) return;

            if (plist) {
                if (auto dict = CCDictionary::createWithContentsOfFileThreadSafe(m_path.string().c_str())) {
                    auto metadata = static_cast<CCDictionary*>(dict->objectForKey("metadata"));
                    auto formatStr = metadata ? metadata->valueForKey("format") : nullptr;
                    auto format = formatStr ? numFromString<int>(formatStr->m_sString).unwrapOr(0) : 0;
                    m_frames->removeAllObjects();
                    for (auto [frameName, frame] : CCDictionaryExt<std::string, CCDictionary*>(static_cast<CCDictionary*>(dict->objectForKey("frames")))) {
                        if (auto spriteFrame = MoreIconsAPI::createSpriteFrame(frame, m_texture, format)) {
                            m_frames->setObject(spriteFrame, MoreIconsAPI::getFrameName(frameName, "", m_iconType).substr(sizeof(GEODE_MOD_ID)));
                        }
                    }
                    updateSprites();

                    auto saveSprite = static_cast<ButtonSprite*>(m_saveButton->getNormalImage());
                    saveSprite->m_BGSprite->setOpacity(255);
                    saveSprite->m_label->setOpacity(255);
                    m_saveButton->setEnabled(true);
                    auto plistSprite = static_cast<ButtonSprite*>(m_plistButton->getNormalImage());
                    plistSprite->m_BGSprite->setOpacity(105);
                    plistSprite->m_label->setOpacity(105);
                    m_plistButton->setEnabled(false);
                    for (auto frameMenu : CCArrayExt<CCMenu*>(m_frameMenus)) {
                        frameMenu->setEnabled(true);
                    }

                    CC_SAFE_RELEASE(m_texture);
                    m_texture = nullptr;
                }
            }
            else {
                auto image = new CCImage();
                if (!image->initWithImageFile(m_path.string().c_str())) {
                    image->release();
                    return;
                }
                auto texture = new CCTexture2D();
                if (!texture->initWithImage(image)) {
                    image->release();
                    texture->release();
                    return;
                }
                image->release();
                if (type > 0) {
                    constexpr std::array suffixes = { "_001.png", "_2_001.png", "_3_001.png", "_glow_001.png", "_extra_001.png" };
                    auto suffix = suffixes[type + (m_iconType != IconType::Ufo && type > 2) - 1];
                    auto key = index > 0 ? fmt::format("_{:02}{}", index, suffix) : suffix;
                    auto spriteFrame = CCSpriteFrame::createWithTexture(texture, { { 0.0f, 0.0f }, texture->getContentSize() });
                    m_frames->setObject(spriteFrame, key);
                    updateSprites();
                }
                else {
                    if (m_iconType <= IconType::Jetpack) {
                        CC_SAFE_RELEASE(m_texture);
                        m_texture = texture;

                        auto saveSprite = static_cast<ButtonSprite*>(m_saveButton->getNormalImage());
                        saveSprite->m_BGSprite->setOpacity(105);
                        saveSprite->m_label->setOpacity(105);
                        m_saveButton->setEnabled(false);
                        auto plistSprite = static_cast<ButtonSprite*>(m_plistButton->getNormalImage());
                        plistSprite->m_BGSprite->setOpacity(255);
                        plistSprite->m_label->setOpacity(255);
                        m_plistButton->setEnabled(true);
                        for (auto frameMenu : CCArrayExt<CCMenu*>(m_frameMenus)) {
                            frameMenu->setEnabled(false);
                        }
                    }
                    else {
                        auto blendFunc = m_streak->getBlendFunc();
                        m_streak->setTexture(texture);
                        m_streak->setBlendFunc(blendFunc);
                        texture->release();
                    }
                }
            }
        }
    });

    std::error_code code;
    m_listener.setFilter(file::pick(file::PickMode::OpenFile, {
        m_path.empty() || !std::filesystem::exists(m_path, code) ? Mod::get()->getConfigDir() : m_path,
        {{
            plist ? "Plist files" : "PNG files",
            { plist ? "*.plist" : "*.png" }
        }}
    }));
}

void EditIconPopup::updateSprites() {
    auto crossFrame = CCSpriteFrameCache::get()->spriteFrameByName("GJ_deleteIcon_001.png");
    for (auto [prefix, sprite] : CCDictionaryExt<std::string, CCSprite*>(m_sprites)) {
        auto spriteFrame = static_cast<CCSpriteFrame*>(m_frames->objectForKey(prefix));
        sprite->setDisplayFrame(spriteFrame ? spriteFrame : crossFrame);
        sprite->setPosition(sprite->getParent()->getContentSize() / 2.0f);
    }

    if (m_iconType == IconType::Robot || m_iconType == IconType::Spider) {
        auto sprite = m_iconType == IconType::Spider ? m_player->m_spiderSprite : m_player->m_robotSprite;
        auto spriteParts = sprite->m_paSprite->m_spriteParts;
        for (int i = 0; i < spriteParts->count(); i++) {
            auto spritePart = static_cast<CCSprite*>(spriteParts->objectAtIndex(i));
            auto tag = spritePart->getTag();

            spritePart->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("_{:02}_001.png", tag))));
            if (auto secondSprite = static_cast<CCSprite*>(sprite->m_secondArray->objectAtIndex(i))) {
                secondSprite->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("_{:02}_2_001.png", tag))));
                secondSprite->setPosition(spritePart->getContentSize() / 2);
            }

            static_cast<CCSprite*>(sprite->m_glowSprite->getChildren()->objectAtIndex(i))->setDisplayFrame(
                static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("_{:02}_glow_001.png", tag))));

            if (spritePart == sprite->m_headSprite) {
                auto extraFrame = static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("_{:02}_extra_001.png", tag)));
                if (extraFrame) {
                    if (sprite->m_extraSprite) sprite->m_extraSprite->setDisplayFrame(extraFrame);
                    else {
                        sprite->m_extraSprite = CCSprite::createWithSpriteFrame(extraFrame);
                        sprite->m_headSprite->addChild(sprite->m_extraSprite, 2);
                    }
                    sprite->m_extraSprite->setPosition(spritePart->getContentSize() / 2);
                }
                sprite->m_extraSprite->setVisible(extraFrame != nullptr);
            }
        }
    }
    else {
        m_player->m_firstLayer->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey("_001.png")));
        m_player->m_secondLayer->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey("_2_001.png")));
        auto firstCenter = m_player->m_firstLayer->getContentSize() / 2.0f;
        m_player->m_secondLayer->setPosition(firstCenter);
        if (m_iconType == IconType::Ufo) {
            m_player->m_birdDome->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey("_3_001.png")));
            m_player->m_birdDome->setPosition(firstCenter);
        }
        m_player->m_outlineSprite->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey("_glow_001.png")));
        m_player->m_outlineSprite->setPosition(firstCenter);
        auto extraFrame = static_cast<CCSpriteFrame*>(m_frames->objectForKey("_extra_001.png"));
        m_player->m_detailSprite->setVisible(extraFrame != nullptr);
        if (extraFrame) {
            m_player->m_detailSprite->setDisplayFrame(extraFrame);
            m_player->m_detailSprite->setPosition(firstCenter);
        }
    }
}

std::vector<std::vector<uint8_t>> getRows(const uint8_t* data, int width, int height) {
    std::vector<std::vector<uint8_t>> rows(height);
    for (int y = 0; y < height; y++) {
        rows[y].resize(width * 4);
        for (int x = 0; x < width * 4; x += 4) {
            rows[y][x] = data[y * width + x];
            rows[y][x + 1] = data[y * width + x + 1];
            rows[y][x + 2] = data[y * width + x + 2];
            rows[y][x + 3] = data[y * width + x + 3];
        }
    }
    return rows;
}

std::vector<std::vector<uint8_t>> getRows90(const uint8_t* data, int width, int height) {
    std::vector<std::vector<uint8_t>> rows(width);
    for (int y = 0; y < width; y++) {
        rows[y].resize(height * 4);
        for (int x = 0; x < height * 4; x += 4) {
            rows[y][x] = data[(height - x / 4 - 1) * width + y];
            rows[y][x + 1] = data[(height - x / 4 - 1) * width + y + 1];
            rows[y][x + 2] = data[(height - x / 4 - 1) * width + y + 2];
            rows[y][x + 3] = data[(height - x / 4 - 1) * width + y + 3];
        }
    }
    return rows;
}

void EditIconPopup::saveIcon() {
    auto iconName = m_textInput->getString();
    if (iconName.empty()) return Notification::create("Please enter a name.", NotificationIcon::Info)->show();

    if (m_iconType == IconType::Special) {
        auto path = Mod::get()->getConfigDir() / "trail" / fmt::format("{}.png", iconName);
        std::error_code code;
        if (std::filesystem::exists(path, code)) {
            createQuickPopup(
                "Existing Trail",
                fmt::format("<cy>{}</c> already exists.\nDo you want to <cr>overwrite</c> it?", iconName),
                "No",
                "Yes",
                [this, iconName, path](auto, bool btn2) {
                    if (!btn2) return;
                    std::error_code code;
                    if (std::filesystem::remove(path, code)) saveIcon();
                    else Notification::create(fmt::format("Failed to delete {}.", iconName), NotificationIcon::Error)->show();
                }
            );
            return;
        }

        auto result = MoreIcons::saveTexture(m_streak->getTexture(), path);
        if (result) Notification::create(fmt::format("{} saved!", iconName), NotificationIcon::Success)->show();
        else Notification::create(fmt::format("Failed to save {}.", iconName), NotificationIcon::Error)->show();
    }
    else if (m_iconType <= IconType::Jetpack) {
        constexpr std::array directories = { "icon", "ship", "ball", "ufo", "wave", "robot", "spider", "swing", "jetpack" };
        auto path = (Mod::get()->getConfigDir() / directories[(int)m_iconType] / iconName).string();
        std::error_code code;
        auto plistPath = fmt::format("{}.plist", path);
        auto pngPath = fmt::format("{}.png", path);
        if (std::filesystem::exists(plistPath, code) || std::filesystem::exists(pngPath, code)) {
            createQuickPopup(
                "Existing Icon",
                fmt::format("<cy>{}</c> already exists.\nDo you want to <cr>overwrite</c> it?", iconName),
                "No",
                "Yes",
                [this, iconName, plistPath, pngPath](auto, bool btn2) {
                    if (!btn2) return;
                    std::error_code code;
                    if (std::filesystem::remove(plistPath, code) || std::filesystem::remove(pngPath, code)) saveIcon();
                    else Notification::create(fmt::format("Failed to delete {}.", iconName), NotificationIcon::Error)->show();
                }
            );
            return;
        }
        
        std::vector<std::string> required;
        if (m_iconType == IconType::Robot || m_iconType == IconType::Spider) {
            for (int i = 1; i < 5; i++) {
                required.push_back(fmt::format("_{:02}_001.png", i));
                required.push_back(fmt::format("_{:02}_2_001.png", i));
                required.push_back(fmt::format("_{:02}_glow_001.png", i));
            }
        }
        else {
            required.push_back("_001.png");
            required.push_back("_2_001.png");
            if (m_iconType == IconType::Ufo) required.push_back("_3_001.png");
            required.push_back("_glow_001.png");
        }
        for (auto& suffix : required) {
            if (!m_frames->objectForKey(suffix)) return Notification::create(fmt::format("Missing frame: {}", suffix), NotificationIcon::Info)->show();
        }

        auto keyArray = m_frames->allKeys();
        auto keyStrings = reinterpret_cast<CCString**>(keyArray->data->arr);
        std::sort(keyStrings, keyStrings + keyArray->data->num, [](CCString* a, CCString* b) {
            return a->m_sString < b->m_sString;
        });
        auto keys = ranges::map<std::vector<std::string>>(std::vector<CCString*>(keyStrings, keyStrings + keyArray->data->num), [](CCString* str) {
            return str->m_sString;
        });
        auto scaleFactor = CCDirector::get()->getContentScaleFactor();
        auto rectangles = ranges::map<std::vector<rect_xywhf>>(keys, [this, scaleFactor](const std::string& key) {
            auto size = static_cast<CCSprite*>(m_sprites->objectForKey(key))->getContentSize() * scaleFactor;
            return rect_xywhf { 0, 0, (int)size.width, (int)size.height, false };
        });

        auto packed = find_best_packing<empty_spaces<true>>(rectangles, make_finder_input(
            1000,
            -4,
            [](rect_xywhf&) { return callback_result::CONTINUE_PACKING; },
            [](rect_xywhf&) { return callback_result::ABORT_PACKING; },
            flipping_option::ENABLED
        ));

        if (packed.w <= 0 || packed.h <= 0) return Notification::create("Failed to pack frames.", NotificationIcon::Error)->show();
        else log::info("Packed {} frames into {}x{}", rectangles.size(), packed.w, packed.h);

        for (int i = 0; i < rectangles.size(); i++) {
            log::info("Packing for {}{}: ({}, {}, {}, {}, {})",
                iconName, keys[i], rectangles[i].x, rectangles[i].y, rectangles[i].w, rectangles[i].h, rectangles[i].flipped);
        }
    }

    m_path.clear();
    m_textInput->setString("");
    onClose(nullptr);
}

void EditIconPopup::onClose(cocos2d::CCObject* sender) {
    if (m_path.empty() && (!m_textInput || m_textInput->getString().empty())) return Popup::onClose(sender);

    constexpr std::array lowercase = {
        "", "icon", "", "", "ship", "ball",
        "UFO", "wave", "robot", "spider", "trail",
        "death effect", "", "swing", "jetpack", "ship fire"
    };

    auto type = (int)GameManager::get()->iconTypeToUnlockType(m_iconType);
    createQuickPopup(
        fmt::format("Exit {} Editor", uppercase[type]).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} editor</c>?\n<cr>All unsaved changes will be lost.</c>", lowercase[type]),
        "No",
        "Yes",
        [this, sender](auto, bool btn2) {
            if (btn2) Popup::onClose(sender);
        }
    );
}

EditIconPopup::~EditIconPopup() {
    CC_SAFE_RELEASE(m_texture);
    CC_SAFE_RELEASE(m_frames);
    CC_SAFE_RELEASE(m_sprites);
    CC_SAFE_RELEASE(m_frameMenus);
}

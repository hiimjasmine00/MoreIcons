#include "EditIconPopup.hpp"
#include "MoreIconsPopup.hpp"
#include "../../MoreIcons.hpp"
#include "../../api/MoreIconsAPI.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Notification.hpp>
#include <texpack.hpp>

using namespace geode::prelude;

EditIconPopup* EditIconPopup::create(IconType type, int id, const std::string& name, bool read) {
    auto ret = new EditIconPopup();
    auto isIcon = type <= IconType::Jetpack;
    if (ret->initAnchored(
        350.0f,
        180.0f + isIcon * 50.0f + (type == IconType::Robot || type == IconType::Spider) * 30.0f - read * 60.0f,
        type,
        id,
        name,
        read,
        "geode.loader/GE_square03.png"
    )) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

constexpr std::array lowercase = {
    "", "icon", "", "", "ship", "ball",
    "UFO", "wave", "robot", "spider", "trail",
    "death effect", "", "swing", "jetpack", "ship fire"
};
constexpr std::array uppercase = {
    "", "Icon", "", "", "Ship", "Ball",
    "UFO", "Wave", "Robot", "Spider", "Trail",
    "Death Effect", "", "Swing", "Jetpack", "Ship Fire"
};

bool EditIconPopup::setup(IconType type, int id, const std::string& name, bool read) {
    auto gameManager = GameManager::get();
    auto unlock = (int)gameManager->iconTypeToUnlockType(type);
    auto unlockName = uppercase[unlock];
    auto colonIndex = name.find(':');

    setID("EditIconPopup");
    setTitle(read ? name.empty() ? fmt::format("{} {:02}", unlockName, id) : name.substr(colonIndex + 1) : fmt::format("{} Editor", unlockName));
    m_title->setID("edit-icon-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    if (read && colonIndex != std::string::npos) {
        if (auto icon = MoreIconsAPI::getIcon(name, type)) {
            auto subTitle = CCLabelBMFont::create(icon->packName.c_str(), "goldFont.fnt");
            subTitle->setPosition(m_title->getPosition() - CCPoint { 0.0f, 15.0f });
            subTitle->setScale(0.4f);
            subTitle->setID("edit-icon-sub-title");
            m_mainLayer->addChild(subTitle);
        }
    }

    m_frames = CCDictionary::create();
    m_frames->retain();
    m_sprites = CCDictionary::create();
    m_sprites->retain();
    m_frameMenus = CCArray::create();
    m_frameMenus->retain();
    m_iconType = type;

    auto isIcon = type <= IconType::Jetpack;
    auto iconID = read ? id : 1;
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

        m_player = SimplePlayer::create(1);
        if (!name.empty()) MoreIconsAPI::updateSimplePlayer(m_player, name, type);
        else m_player->updatePlayerFrame(id, type);
        m_player->setGlowOutline({ 255, 255, 255 });
        m_player->setPosition({ 175.0f, 150.0f + isRobot * 80.0f - suffixes.size() * 30.0f - read * 70.0f });
        m_player->setID("player-icon");
        m_mainLayer->addChild(m_player);

        auto iconName = name.empty() ? fmt::format("{}{:02}", MoreIcons::prefixes[unlock], iconID) : Mod::get()->expandSpriteName(name).data();
        auto spriteFrameCache = CCSpriteFrameCache::get();
        for (int i = 0; i < suffixes.size(); i++) {
            auto frameMenu = CCMenu::create();
            frameMenu->setPosition({ 175.0f, 170.0f + isRobot * 40.0f - i * 30.0f + std::max(i - 1, 0) * 10.0f - read * 70.0f });
            frameMenu->setContentSize({ 350.0f, 30.0f });
            frameMenu->setEnabled(!read);
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
                else if (read) continue;

                auto sprite = CCSprite::createWithSpriteFrame(spriteFrame ? spriteFrame : crossFrame);
                m_sprites->setObject(sprite, subSuffix);

                auto button = CCMenuItemExt::createSpriteExtra(sprite, [this, i, isRobot, j](auto) { pickFile(i + isRobot, j + 1, false); });
                button->setID(fmt::format("frame-button-{}", j + 1));
                frameButtons->addObject(button);
                frameMenu->addChild(button);
            }

            auto gap = 0.0f;
            if (read && i == 0) {
                switch (frameMenu->getChildrenCount()) {
                    case 2: gap = 100.0f; break;
                    case 3: gap = 50.0f; break;
                    case 4: gap = 20.0f; break;
                    case 5: gap = 10.0f; break;
                }
            }
            else if (read) gap = 20.0f;
            else if (type == IconType::Ship || type == IconType::Wave || isRobot) gap = 15.0f;
            else if (type == IconType::Ball || type == IconType::Jetpack) gap = 5.0f;
            else if (type != IconType::Ufo) gap = 10.0f;

            frameMenu->setLayout(RowLayout::create()->setGap(gap));
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
        }
        else {
            stroke = icon ? icon->stroke : 14.0f;
            tint = icon && icon->tint;
        }

        m_streak = CCSprite::create((name.empty() ?
            fmt::format("streak_{:02}_001.png", iconID) : icon ? icon->textures[0] : "streak_01_001.png").c_str());
        m_streak->setBlendFunc({
            GL_SRC_ALPHA,
            (uint32_t)GL_ONE_MINUS_SRC_ALPHA - (trailID > 0 || (icon && icon->blend)) * (uint32_t)GL_SRC_ALPHA
        });
        m_streak->setPosition({ 175.0f, 120.0f + (read && icon && icon->packID.empty()) * 20.0f - read * 70.0f });
        m_streak->setRotation(-90.0f);
        m_streak->setScaleX(stroke / m_streak->getContentWidth());
        m_streak->setScaleY(320.0f / m_streak->getContentHeight());
        if (tint) m_streak->setColor(gameManager->colorForIdx(gameManager->m_playerColor2));
        m_streak->setID("streak-preview");
        m_mainLayer->addChild(m_streak);
    }

    if (read) {
        auto icon = id > 0 ? nullptr : MoreIconsAPI::getIcon(name, type);
        if (!icon || !icon->packID.empty()) return true;

        auto trashButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_trashBtn_001.png", 0.8f, [this, icon, unlock, unlockName](auto) {
            createQuickPopup(
                fmt::format("Trash {}", unlockName).c_str(),
                fmt::format("Are you sure you want to <cr>trash</c> this <cg>{}</c>?", lowercase[unlock]),
                "No",
                "Yes",
                [this, icon, unlock](auto, bool btn2) {
                    if (!btn2) return;

                    auto trashDir = Mod::get()->getConfigDir() / "trash";
                    std::error_code code;
                    auto exists = std::filesystem::exists(trashDir, code);
                    if (!exists) exists = std::filesystem::create_directory(trashDir, code);
                    if (!exists) return Notification::create("Failed to create trash directory.", NotificationIcon::Error)->show();
                    else std::filesystem::permissions(trashDir, std::filesystem::perms::all, code);

                    if (icon->sheetName.empty() && !icon->folderName.empty()) {
                        auto filename = std::filesystem::path(icon->folderName).filename();
                        std::filesystem::rename(icon->folderName, trashDir / filename, code);
                        if (code) return Notification::create(
                            fmt::format("Failed to trash {}: {}.", filename, code.message()), NotificationIcon::Error)->show();
                    }
                    else if (icon->folderName.empty() && !icon->sheetName.empty()) {
                        auto sheetName = std::filesystem::path(icon->sheetName).filename();
                        std::filesystem::rename(icon->sheetName, trashDir / sheetName, code);
                        if (code) return Notification::create(
                            fmt::format("Failed to trash {}: {}.", sheetName, code.message()), NotificationIcon::Error)->show();

                        auto textureName = std::filesystem::path(icon->textures[0]).filename();
                        std::filesystem::rename(icon->textures[0], trashDir / textureName, code);
                        if (code) return Notification::create(
                            fmt::format("Failed to trash {}: {}.", textureName, code.message()), NotificationIcon::Error)->show();
                    }
                    else if (!icon->textures.empty()) {
                        auto filename = std::filesystem::path(icon->textures[0]).filename();
                        std::filesystem::rename(icon->textures[0], trashDir / filename, code);
                        if (code) return Notification::create(
                            fmt::format("Failed to trash {}: {}.", filename, code.message()), NotificationIcon::Error)->show();
                    }

                    Notification::create(fmt::format("Trashed {}!", icon->name), NotificationIcon::Success)->show();
                    MoreIcons::showReload = true;
                    if (auto miPopup = CCScene::get()->getChildByType<MoreIconsPopup>(0)) miPopup->m_reloadButton->setVisible(true);
                    onClose(nullptr);
                }
            );
        });
        trashButton->setPosition({ 320.0f, 30.0f });
        trashButton->setID("trash-button");
        m_buttonMenu->addChild(trashButton);
        return true;
    }

    m_textInput = TextInput::create(300.0f, "Name");
    m_textInput->setPosition({ 175.0f, 75.0f });
    m_textInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    m_textInput->setMaxCharCount(100);
    m_textInput->setID("text-input");
    m_mainLayer->addChild(m_textInput);

    auto bottomMenu = CCMenu::create();
    bottomMenu->setPosition({ 175.0f, 30.0f });
    bottomMenu->setContentSize({ 350.0f, 30.0f });
    bottomMenu->setLayout(RowLayout::create()->setGap(25.0f));
    bottomMenu->setID("bottom-menu");

    auto pngButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("PNG", "goldFont.fnt", "GJ_button_05.png"), [this](auto) {
        pickFile(0, 0, false);
    });
    pngButton->setID("png-button");
    bottomMenu->addChild(pngButton);

    m_saveButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png"), [this](auto) {
        auto iconName = m_textInput->getString();
        if (iconName.empty()) return Notification::create("Please enter a name.", NotificationIcon::Info)->show();

        auto mod = Mod::get();
        auto configDir = mod->getConfigDir();
        std::error_code code;
        if (m_iconType == IconType::Special) {
            auto path = configDir / "trail" / fmt::format("{}.png", iconName);
            if (std::filesystem::exists(path, code)) createQuickPopup(
                "Existing Trail",
                fmt::format("<cy>{}</c> already exists.\nDo you want to <cr>overwrite</c> it?", iconName),
                "No",
                "Yes",
                [this, path](auto, bool btn2) {
                    if (btn2) saveTrail(path);
                }
            );
            else saveTrail(path);
        }
        else if (m_iconType <= IconType::Jetpack) {
            constexpr std::array directories = { "icon", "ship", "ball", "ufo", "wave", "robot", "spider", "swing", "jetpack" };
            auto factor = CCDirector::get()->getContentScaleFactor();
            auto path = configDir / directories[(int)m_iconType] / (iconName + (factor >= 4.0f ? "-uhd" : factor >= 2.0f ? "-hd" : ""));
            auto png = std::filesystem::path(path).replace_extension(".png");
            auto plist = std::filesystem::path(path).replace_extension(".plist");
            if (std::filesystem::exists(png, code) || std::filesystem::exists(plist, code))
                createQuickPopup(
                    "Existing Icon",
                    fmt::format("<cy>{}</c> already exists.\nDo you want to <cr>overwrite</c> it?", iconName),
                    "No",
                    "Yes",
                    [this, png, plist](auto, bool btn2) {
                        if (btn2) saveIcon(png, plist);
                    }
                );
            else saveIcon(png, plist);
        }
    });
    m_saveButton->setID("save-button");
    bottomMenu->addChild(m_saveButton);

    if (isIcon) {
        auto plistSprite = ButtonSprite::create("Plist", "goldFont.fnt", "GJ_button_05.png");
        plistSprite->m_BGSprite->setOpacity(105);
        plistSprite->m_label->setOpacity(105);
        m_plistButton = CCMenuItemExt::createSpriteExtra(plistSprite, [this](auto) { pickFile(0, 0, true); });
        m_plistButton->setEnabled(false);
        m_plistButton->setID("plist-button");
        bottomMenu->addChild(m_plistButton);
    }

    bottomMenu->updateLayout();
    m_mainLayer->addChild(bottomMenu);

    return true;
}

void EditIconPopup::pickFile(int index, int type, bool plist) {
    m_listener.bind([this, index, type, plist](Task<Result<std::filesystem::path>>::Event* e) {
        if (auto res = e->getValue(); res && res->isOk()) {
            m_path = res->unwrap();
            if ((plist && m_path.extension() != ".plist") || (!plist && m_path.extension() != ".png"))
                return Notification::create(fmt::format("Please select a {} file.", plist ? "Plist" : "PNG"), NotificationIcon::Info)->show();

            if (plist) {
                auto framesRes = MoreIconsAPI::createFrames(string::pathToString(m_path), m_texture, "", m_iconType);
                if (framesRes.isOk()) {
                    auto [texture, frames] = framesRes.unwrap();
                    if (texture == m_texture) {
                        m_frames->removeAllObjects();
                        for (auto [frameName, frame] : CCDictionaryExt<gd::string, CCSpriteFrame*>(frames)) {
                            m_frames->setObject(frame, frameName);
                        }
                        frames->release();
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
                    }
                    else {
                        texture->release();
                        frames->release();
                    }

                    CC_SAFE_RELEASE(m_texture);
                    m_texture = nullptr;
                }
                else Notification::create(fmt::format("Failed to load frames: {}", framesRes.unwrapErr()), NotificationIcon::Error)->show();
            }
            else {
                auto imageRes = texpack::fromPNG(m_path);
                if (imageRes.isErr())
                    return Notification::create(fmt::format("Failed to load image: {}", imageRes.unwrapErr()), NotificationIcon::Error)->show();

                auto image = imageRes.unwrap();
                auto texture = new CCTexture2D();
                texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
                    (float)image.width,
                    (float)image.height
                });

                if (type > 0) {
                    constexpr std::array suffixes = { "_001.png", "_2_001.png", "_3_001.png", "_glow_001.png", "_extra_001.png" };
                    auto suffix = suffixes[type + (m_iconType != IconType::Ufo && type > 2) - 1];
                    auto key = index > 0 ? fmt::format("_{:02}{}", index, suffix) : suffix;
                    auto spriteFrame = CCSpriteFrame::createWithTexture(texture, { { 0.0f, 0.0f }, texture->getContentSize() });
                    texture->release();
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
                    else if (m_iconType == IconType::Special) {
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
    for (auto [prefix, sprite] : CCDictionaryExt<gd::string, CCSprite*>(m_sprites)) {
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

texpack::Image getImage(cocos2d::CCSprite* sprite) {
    auto director = CCDirector::get();
    auto size = sprite->getContentSize() * director->getContentScaleFactor();
    uint32_t width = size.width;
    uint32_t height = size.height;

    auto texture = 0u;
    glPixelStorei(GL_PACK_ALIGNMENT, 8);
    glGenTextures(1, &texture);
    ccGLBindTexture2D(texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    auto oldFBO = 0;
    auto fbo = 0u;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    auto winSize = director->getWinSizeInPixels();
    glViewport(0, 0, winSize.width, winSize.height);

    auto blendFunc = sprite->getBlendFunc();
    sprite->setBlendFunc({ GL_ONE, GL_ZERO });
    sprite->draw();
    sprite->setBlendFunc(blendFunc);

    std::vector<uint8_t> data(width * height * 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
    director->setViewport();
    ccGLDeleteTexture(texture);
    glDeleteFramebuffers(1, &fbo);

    for (int y = 0; y < height / 2; y++) {
        std::swap_ranges(data.begin() + y * width * 4, data.begin() + (y + 1) * width * 4, data.end() - (y + 1) * width * 4);
    }

    return { data, width, height };
}

void EditIconPopup::saveTrail(const std::filesystem::path& path) {
    auto iconName = m_textInput->getString();

    if (auto res = texpack::toPNG(path, getImage(m_streak)); res.isErr())
        return Notification::create(fmt::format("Failed to save {}: {}", iconName, res.unwrapErr()), NotificationIcon::Error)->show();

    Notification::create(fmt::format("{} saved!", iconName), NotificationIcon::Success)->show();
    finishSave();
}

void EditIconPopup::saveIcon(const std::filesystem::path& png, const std::filesystem::path& plist) {
    auto iconName = m_textInput->getString();

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
        if (!m_frames->objectForKey(suffix))
            return Notification::create(fmt::format("Missing {}{}.", iconName, suffix), NotificationIcon::Info)->show();
    }

    texpack::Packer packer;
    for (auto [frameName, frame] : CCDictionaryExt<std::string, CCSpriteFrame*>(m_frames)) {
        packer.frame(iconName + frameName, getImage(static_cast<CCSprite*>(m_sprites->objectForKey(frameName))));
    }

    if (auto res = packer.pack(); res.isErr())
        return Notification::create(fmt::format("Failed to pack frames: {}", res.unwrapErr()), NotificationIcon::Error)->show();

    if (auto res = packer.png(png); res.isErr())
        return Notification::create(fmt::format("Failed to save image: {}", res.unwrapErr()), NotificationIcon::Error)->show();

    if (auto res = packer.plist(plist, fmt::format("icons/{}", plist.filename()), "    "); res.isErr())
        return Notification::create(fmt::format("Failed to save {}: {}", iconName, res.unwrapErr()), NotificationIcon::Error)->show();

    Notification::create(fmt::format("{} saved!", iconName), NotificationIcon::Success)->show();
    finishSave();
}

void EditIconPopup::finishSave() {
    MoreIcons::showReload = true;
    if (auto miPopup = CCScene::get()->getChildByType<MoreIconsPopup>(0)) miPopup->m_reloadButton->setVisible(true);

    m_path.clear();
    m_textInput->setString("");
    onClose(nullptr);
}

void EditIconPopup::onClose(cocos2d::CCObject* sender) {
    if (m_path.empty() && (!m_textInput || m_textInput->getString().empty())) return Popup::onClose(sender);

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

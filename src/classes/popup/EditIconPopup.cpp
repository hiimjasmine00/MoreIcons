#include "EditIconPopup.hpp"
#include "MoreIconsPopup.hpp"
#include "view/IconViewPopup.hpp"
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
    if (ret->initAnchored(
        350.0f,
        180.0f + (type <= IconType::Jetpack) * 50.0f + (type == IconType::Robot || type == IconType::Spider) * 30.0f - read * 60.0f,
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

template <typename... T>
void notify(NotificationIcon icon, fmt::format_string<T...> message, T&&... args) {
    Notification::create(fmt::format(message, std::forward<T>(args)...), icon)->show();
}

bool EditIconPopup::setup(IconType type, int id, const std::string& name, bool read) {
    auto gameManager = GameManager::get();
    auto unlock = gameManager->iconTypeToUnlockType(type);
    auto unlockName = uppercase[(int)unlock];
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
    m_iconType = type;
    m_readOnly = read;

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

        m_player = SimplePlayer::create(1);
        if (!name.empty()) MoreIconsAPI::updateSimplePlayer(m_player, name, type);
        else m_player->updatePlayerFrame(id, type);
        m_player->setGlowOutline({ 255, 255, 255 });
        m_player->setPosition({ 175.0f, 150.0f + isRobot * 80.0f - suffixes.size() * 30.0f - read * 70.0f });
        m_player->setID("player-icon");
        m_mainLayer->addChild(m_player);

        auto prefix = name.empty() ? MoreIconsAPI::iconName(id, unlock) : fmt::format("{}"_spr, name);
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
                auto& suffix = subSuffixes[j];

                auto spriteFrame = MoreIconsAPI::getFrame("{}{}", prefix, suffix);
                if (spriteFrame) m_frames->setObject(spriteFrame, suffix);
                else if (read) continue;

                auto sprite = CCSprite::createWithSpriteFrame(spriteFrame ? spriteFrame : crossFrame);
                m_sprites->setObject(sprite, suffix);

                auto button = CCMenuItemExt::createSpriteExtra(sprite, [this, i, isRobot, j](auto) {
                    pickFile(i + isRobot, j + 1);
                });
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
                    auto& suffix = subSuffixes[j];
                    auto resetSprite = CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png");
                    resetSprite->setScale(i < 1 && type != IconType::Ufo ? contentHeight / resetSprite->getContentHeight() : 0.4f);
                    auto resetButton = CCMenuItemExt::createSpriteExtra(resetSprite, [this, i, j, suffix](auto) {
                        createQuickPopup(
                            "Reset Frame",
                            "Are you sure you want to <cy>reset</c> this <cg>frame</c>?",
                            "No",
                            "Yes",
                            [this, i, j, suffix](auto, bool btn2) {
                                if (!btn2) return;
                                auto spriteFrame = MoreIconsAPI::getFrame("{}{}", MoreIconsAPI::iconName(1, m_iconType), suffix);
                                if (spriteFrame) m_frames->setObject(spriteFrame, suffix);
                                else m_frames->removeObjectForKey(suffix);
                                updateSprites();
                            }
                        );
                    });
                    resetButton->setID(fmt::format("reset-button-{}", j + 1));
                    frameMenu->insertAfter(resetButton, frameButton);
                }
            }

            frameMenu->updateLayout();
            m_mainLayer->addChild(frameMenu);
        }
    }
    else if (type == IconType::Special) {
        auto icon = MoreIconsAPI::getIcon(name, type);

        auto stroke = 10.0f;
        auto tint = true;
        auto trailID = icon ? icon->trailID : read ? id : 0;
        if (trailID != 0) switch (trailID) {
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

        m_streak = CCSprite::create((icon ? icon->textures[0] : fmt::format("streak_{:02}_001.png", name.empty() ? id : 1)).c_str());
        m_streak->setBlendFunc({
            GL_SRC_ALPHA,
            (uint32_t)GL_ONE_MINUS_SRC_ALPHA - (trailID != 0 || (icon && icon->blend)) * (uint32_t)GL_SRC_ALPHA
        });
        m_streak->setPosition({ 175.0f, 120.0f + (read && icon && !icon->vanilla && !icon->zipped) * 20.0f - read * 70.0f });
        m_streak->setRotation(-90.0f);
        m_streak->setScaleX(stroke / m_streak->getContentWidth());
        m_streak->setScaleY(320.0f / m_streak->getContentHeight());
        if (tint) m_streak->setColor(gameManager->colorForIdx(gameManager->m_playerColor2));
        m_streak->setID("streak-preview");
        m_mainLayer->addChild(m_streak);
    }

    if (read) {
        auto icon = id > 0 ? nullptr : MoreIconsAPI::getIcon(name, type);
        if (!icon || icon->vanilla || icon->zipped) return true;

        auto trashButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_trashBtn_001.png", 0.8f, [this, icon, unlock, unlockName](auto) {
            createQuickPopup(
                fmt::format("Trash {}", unlockName).c_str(),
                fmt::format("Are you sure you want to <cr>trash</c> this <cg>{}</c>?", lowercase[(int)unlock]),
                "No",
                "Yes",
                [this, icon](auto, bool btn2) {
                    if (!btn2) return;

                    auto trashDir = MoreIcons::createTrash();
                    if (!trashDir.empty()) return;

                    std::error_code code;

                    if (!icon->sheetName.empty()) {
                        auto sheetName = std::filesystem::path(icon->sheetName).filename();
                        std::filesystem::rename(icon->sheetName, trashDir / sheetName, code);
                        if (code) return notify(NotificationIcon::Error, "Failed to trash {}: {}.", sheetName, code.message());
                    }

                    auto textureName = std::filesystem::path(icon->textures[0]).filename();
                    std::filesystem::rename(icon->textures[0], trashDir / textureName, code);
                    if (code) return notify(NotificationIcon::Error, "Failed to trash {}: {}.", textureName, code.message());

                    auto name = icon->name;
                    MoreIconsAPI::removeIcon(icon);
                    notify(NotificationIcon::Success, "Trashed {}! Re-open the icon kit to apply changes.", name);
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

    auto pngButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create(isIcon ? "PNG + Plist" : "PNG", "goldFont.fnt", "GJ_button_05.png"),
        [this](auto) {
            pickFile(0, 0);
        });
    pngButton->setID("png-button");
    bottomMenu->addChild(pngButton);

    auto saveButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png"), [this](auto) {
        auto iconName = m_textInput->getString();
        if (iconName.empty()) return notify(NotificationIcon::Info, "Please enter a name.");

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
            auto png = std::filesystem::path(path).concat(".png");
            auto plist = std::filesystem::path(path).concat(".plist");
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
    saveButton->setID("save-button");
    bottomMenu->addChild(saveButton);

    bottomMenu->updateLayout();
    m_mainLayer->addChild(bottomMenu);

    return true;
}

void EditIconPopup::fullClose() {
    m_pickerOpened = false;
    if (m_textInput) m_textInput->setString("");
    onClose(nullptr);
    auto scene = CCScene::get();
    if (m_readOnly) {
        if (auto iconViewPopup = scene->getChildByType<IconViewPopup>(0)) iconViewPopup->close();
    }
    if (auto moreIconsPopup = scene->getChildByType<MoreIconsPopup>(0)) moreIconsPopup->close();
}

void EditIconPopup::pickFile(int index, int type) {
    m_listener.bind([this, index, type](Task<Result<std::vector<std::filesystem::path>>>::Event* e) {
        auto res = e->getValue();
        if (!res || res->isErr()) return;

        auto paths = res->unwrap();
        if (paths.empty()) return;

        m_pickerOpened = true;
        std::string png;
        std::string plist;
        for (auto& path : paths) {
            if (path.extension() == ".png") {
                if (png.empty()) png = string::pathToString(path);
            }
            else if (path.extension() == ".plist") {
                if (plist.empty()) plist = string::pathToString(path);
            }
            if (!png.empty() && !plist.empty()) break;
        }

        auto isIcon = m_iconType <= IconType::Jetpack;
        auto pngEmpty = png.empty();
        auto plistEmpty = type == 0 && isIcon && plist.empty();
        if (pngEmpty && plistEmpty) return notify(NotificationIcon::Info, "Please select a PNG file and a Plist file");
        else if (plistEmpty) return notify(NotificationIcon::Info, "Please select a Plist file");
        else if (pngEmpty) return notify(NotificationIcon::Info, "Please select a PNG file");

        if (type > 0) {
            GEODE_UNWRAP_OR_ELSE(image, err, texpack::fromPNG(png)) return notify(NotificationIcon::Error, "Failed to load image: {}", err);

            auto texture = new CCTexture2D();
            texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
                (float)image.width,
                (float)image.height
            });

            constexpr std::array suffixes = { "_001.png", "_2_001.png", "_3_001.png", "_glow_001.png", "_extra_001.png" };
            auto suffix = suffixes[type + (m_iconType != IconType::Ufo && type > 2) - 1];
            m_frames->setObject(CCSpriteFrame::createWithTexture(texture, { { 0.0f, 0.0f }, texture->getContentSize() }),
                index > 0 ? fmt::format("_{:02}{}", index, suffix) : suffix);
            texture->release();
            return updateSprites();
        }

        GEODE_UNWRAP_OR_ELSE(image, err, MoreIconsAPI::createFrames(png, plist, "", m_iconType))
            return notify(NotificationIcon::Error, "Failed to load frames: {}", err);

        if (isIcon) {
            m_frames->removeAllObjects();
            for (auto [frameName, frame] : CCDictionaryExt<gd::string, CCSpriteFrame*>(image.frames)) {
                m_frames->setObject(frame, frameName);
            }
            image.frames->release();
            image.texture->release();
            updateSprites();
        }
        else if (m_iconType == IconType::Special) {
            m_streak->m_pobTexture->release();
            m_streak->m_pobTexture = image.texture;
        }
    });

    m_listener.setFilter(file::pickMany({ std::nullopt, {{ "PNG/Plist files", { "*.png", "*.plist" } }} }));
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
                secondSprite->setPosition(spritePart->getContentSize() / 2.0f);
            }

            if (auto glowChild = getChild<CCSprite>(sprite->m_glowSprite, i))
                glowChild->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("_{:02}_glow_001.png", tag))));

            if (spritePart == sprite->m_headSprite) {
                auto extraFrame = static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("_{:02}_extra_001.png", tag)));
                if (extraFrame) {
                    if (sprite->m_extraSprite) sprite->m_extraSprite->setDisplayFrame(extraFrame);
                    else {
                        sprite->m_extraSprite = CCSprite::createWithSpriteFrame(extraFrame);
                        spritePart->addChild(sprite->m_extraSprite, 2);
                    }
                    sprite->m_extraSprite->setPosition(spritePart->getContentSize() / 2.0f);
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

void EditIconPopup::addOrUpdateIcon(const std::string& name, const std::filesystem::path& png, const std::filesystem::path& plist) {
    if (auto icon = MoreIconsAPI::getIcon(name, m_iconType)) MoreIconsAPI::updateIcon(icon);
    else {
        MoreIconsAPI::addIcon({
            .name = name,
            .textures = { string::pathToString(png) },
            .frameNames = {},
            .sheetName = string::pathToString(plist),
            .packName = "More Icons",
            .packID = "",
            .type = m_iconType,
            .trailID = 0,
            .blend = false,
            .tint = false,
            .show = false,
            .fade = 0.0f,
            .stroke = 0.0f,
            .shortName = name,
            .vanilla = false,
            .zipped = false
        });
    }
    notify(NotificationIcon::Success, "{} saved! Re-open the icon kit to apply changes.", name);
    fullClose();
}

bool EditIconPopup::checkFrame(std::string_view suffix) {
    auto frame = m_frames->objectForKey(gd::string(suffix.data(), suffix.size()));
    if (!frame) notify(NotificationIcon::Info, "Missing {}{}.", m_textInput->getString(), suffix);
    return frame != nullptr;
}

texpack::Image getImage(CCSprite* sprite) {
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

    if (!sprite->getDontDraw() && sprite->getOpacity() > 0) {
        if (auto shaderProgram = sprite->getShaderProgram()) {
            shaderProgram->use();
            shaderProgram->setUniformsForBuiltins();
        }
        ccGLBlendFunc(GL_ONE, GL_ZERO);
        ccGLBindTexture2D(sprite->getTexture()->getName());
        glVertexAttribPointer(kCCVertexAttrib_Position, 3, GL_FLOAT, GL_FALSE, 24, &sprite->m_sQuad.tl.vertices);
        glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 24, &sprite->m_sQuad.tl.colors);
        glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, 24, &sprite->m_sQuad.tl.texCoords);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

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
    if (GEODE_UNWRAP_IF_ERR(err, texpack::toPNG(path, getImage(m_streak)))) return notify(NotificationIcon::Error, "Failed to save image: {}", err);

    addOrUpdateIcon(m_textInput->getString(), path, "");
}

void EditIconPopup::saveIcon(const std::filesystem::path& png, const std::filesystem::path& plist) {
    if (m_iconType == IconType::Robot || m_iconType == IconType::Spider) {
        if (!checkFrame("_01_001.png") || !checkFrame("_01_2_001.png") || !checkFrame("_01_glow_001.png")) return;
        if (!checkFrame("_02_001.png") || !checkFrame("_02_2_001.png") || !checkFrame("_02_glow_001.png")) return;
        if (!checkFrame("_03_001.png") || !checkFrame("_03_2_001.png") || !checkFrame("_03_glow_001.png")) return;
        if (!checkFrame("_04_001.png") || !checkFrame("_04_2_001.png") || !checkFrame("_04_glow_001.png")) return;
    }
    else {
        if (!checkFrame("_001.png")) return;
        if (!checkFrame("_2_001.png")) return;
        if (m_iconType == IconType::Ufo && !checkFrame("_3_001.png")) return;
        if (!checkFrame("_glow_001.png")) return;
    }

    auto name = m_textInput->getString();

    texpack::Packer packer;
    for (auto [frameName, frame] : CCDictionaryExt<std::string, CCSpriteFrame*>(m_frames)) {
        packer.frame(name + frameName, getImage(static_cast<CCSprite*>(m_sprites->objectForKey(frameName))));
    }

    if (GEODE_UNWRAP_IF_ERR(err, packer.pack())) return notify(NotificationIcon::Error, "Failed to pack frames: {}", err);
    if (GEODE_UNWRAP_IF_ERR(err, packer.png(png))) return notify(NotificationIcon::Error, "Failed to save image: {}", err);
    if (GEODE_UNWRAP_IF_ERR(err, packer.plist(plist, fmt::format("icons/{}", plist.filename()), "    ")))
        return notify(NotificationIcon::Error, "Failed to save plist: {}", err);

    addOrUpdateIcon(name, png, plist);
}

void EditIconPopup::onClose(CCObject* sender) {
    if (!m_pickerOpened && (!m_textInput || m_textInput->getString().empty())) return Popup::onClose(sender);

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
    CC_SAFE_RELEASE(m_frames);
    CC_SAFE_RELEASE(m_sprites);
}

#include "EditIconPopup.hpp"
#include "MoreIconsPopup.hpp"
#include "../../MoreIcons.hpp"
#include "../../api/MoreIconsAPI.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCPartAnimSprite.hpp>
#include <Geode/binding/CCSpritePart.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Notification.hpp>
#include <texpack.hpp>

using namespace geode::prelude;

EditIconPopup* EditIconPopup::create(MoreIconsPopup* popup, IconType type) {
    auto ret = new EditIconPopup();
    if (ret->initAnchored(
        350.0f,
        180.0f + (type <= IconType::Jetpack) * 50.0f + (type == IconType::Robot || type == IconType::Spider) * 30.0f,
        popup,
        type,
        "geode.loader/GE_square03.png"
    )) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

template <typename... T>
void notify(NotificationIcon icon, fmt::format_string<T...> message, T&&... args) {
    Notification::create(fmt::format(message, std::forward<T>(args)...), icon)->show();
}

bool EditIconPopup::setup(MoreIconsPopup* popup, IconType type) {
    auto miType = MoreIconsAPI::convertType(type);

    setID("EditIconPopup");
    setTitle(fmt::format("{} Editor", MoreIconsAPI::uppercase[miType]));
    m_title->setID("edit-icon-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_parentPopup = popup;
    m_frames = CCDictionary::create();
    m_sprites = CCDictionary::create();
    m_iconType = type;

    auto isIcon = type <= IconType::Jetpack;
    if (isIcon) {
        auto isRobot = type == IconType::Robot || type == IconType::Spider;
        std::vector<std::vector<std::string>> suffixes;
        suffixes.reserve(isRobot * 3 + 1);
        if (isRobot) {
            suffixes.push_back({ "_01_001.png", "_01_2_001.png", "_01_glow_001.png", "_01_extra_001.png" });
            suffixes.push_back({ "_02_001.png", "_02_2_001.png", "_02_glow_001.png" });
            suffixes.push_back({ "_03_001.png", "_03_2_001.png", "_03_glow_001.png" });
            suffixes.push_back({ "_04_001.png", "_04_2_001.png", "_04_glow_001.png" });
        }
        else {
            if (type == IconType::Ufo) suffixes.push_back({ "_001.png", "_2_001.png", "_3_001.png", "_glow_001.png", "_extra_001.png" });
            else suffixes.push_back({ "_001.png", "_2_001.png", "_glow_001.png", "_extra_001.png" });
        }

        m_player = SimplePlayer::create(1);
        m_player->updatePlayerFrame(1, type);
        m_player->setGlowOutline({ 255, 255, 255 });
        m_player->setPosition({ 175.0f, 150.0f + isRobot * 80.0f - suffixes.size() * 30.0f });
        m_player->setID("player-icon");
        m_mainLayer->addChild(m_player);

        auto prefix = MoreIconsAPI::prefixes[miType];
        auto spriteFrameCache = MoreIconsAPI::get<CCSpriteFrameCache>();
        auto crossFrame = spriteFrameCache->spriteFrameByName("GJ_deleteIcon_001.png");
        for (int i = 0; i < suffixes.size(); i++) {
            auto frameMenu = CCMenu::create();
            frameMenu->setPosition({ 175.0f, 170.0f + isRobot * 40.0f - i * 30.0f + std::max(i - 1, 0) * 10.0f });
            frameMenu->setContentSize({ 350.0f, 30.0f });
            frameMenu->setID(fmt::format("frame-menu-{}", i + 1));

            auto& subSuffixes = suffixes[i];
            auto frameButtons = CCArray::create();
            for (int j = 0; j < subSuffixes.size(); j++) {
                auto& suffix = subSuffixes[j];

                auto spriteFrame = MoreIconsAPI::getFrame(fmt::format("{}01{}", prefix, suffix));
                if (spriteFrame) m_frames->setObject(spriteFrame, suffix);

                auto sprite = CCSprite::createWithSpriteFrame(spriteFrame ? spriteFrame : crossFrame);
                m_sprites->setObject(sprite, suffix);

                auto button = CCMenuItemExt::createSpriteExtra(sprite, [this, i, isRobot, suffix](auto) {
                    pickFile(i + isRobot, suffix);
                });
                button->setID(fmt::format("frame-button-{}", j + 1));
                frameButtons->addObject(button);
                frameMenu->addChild(button);
            }

            auto gap = 0.0f;
            if (i == 0) {
                switch (frameMenu->getChildrenCount()) {
                    case 2: gap = 100.0f; break;
                    case 3: gap = 50.0f; break;
                    case 4: gap = 20.0f; break;
                    case 5: gap = 10.0f; break;
                }
            }
            else gap = 20.0f;

            frameMenu->setLayout(RowLayout::create()->setGap(gap));
            frameMenu->updateLayout();

            for (auto button : CCArrayExt<CCMenuItemSprite*>(frameButtons)) {
                auto height = frameMenu->getContentHeight() * button->getScale();
                button->setContentSize({ height, height });
                button->getNormalImage()->setPosition({ height / 2.0f, height / 2.0f });
            }

            frameMenu->updateLayout();
            m_mainLayer->addChild(frameMenu);
        }
    }
    else if (type == IconType::Special) {
        auto stroke = 10.0f;
        auto tint = true;

        m_streak = CCSprite::create("streak_01_001.png");
        m_streak->setBlendFunc({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA });
        m_streak->setPosition({ 175.0f, 120.0f });
        m_streak->setRotation(-90.0f);
        auto& size = m_streak->getContentSize();
        m_streak->setScaleX(14.0f / size.width);
        m_streak->setScaleY(320.0f / size.height);
        m_streak->setID("streak-preview");
        m_mainLayer->addChild(m_streak);
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
            pickFile(0, "");
        });
    pngButton->setID("png-button");
    bottomMenu->addChild(pngButton);

    auto saveButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png"), [this, miType](auto) {
        auto iconName = m_textInput->getString();
        if (iconName.empty()) return notify(NotificationIcon::Info, "Please enter a name.");

        auto configDir = Mod::get()->getConfigDir();
        auto folder = MoreIcons::folders[miType];
        if (m_iconType == IconType::Special) {
            auto path = configDir / folder / fmt::format("{}.png", iconName);
            if (MoreIcons::doesExist(path)) createQuickPopup(
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
            auto parent = configDir / folder;
            auto factor = MoreIconsAPI::get<CCDirector>()->getContentScaleFactor();
            auto filename = iconName + (factor >= 4.0f ? "-uhd" : factor >= 2.0f ? "-hd" : "");
            auto png = parent / (filename + ".png");
            auto plist = parent / (filename + ".plist");
            if (MoreIcons::doesExist(png) || MoreIcons::doesExist(plist)) {
                createQuickPopup(
                    "Existing Icon",
                    fmt::format("<cy>{}</c> already exists.\nDo you want to <cr>overwrite</c> it?", iconName),
                    "No",
                    "Yes",
                    [this, png, plist](auto, bool btn2) {
                        if (btn2) saveIcon(png, plist);
                    }
                );
            }
            else saveIcon(png, plist);
        }
    });
    saveButton->setID("save-button");
    bottomMenu->addChild(saveButton);

    bottomMenu->updateLayout();
    m_mainLayer->addChild(bottomMenu);

    handleTouchPriority(this);

    return true;
}

void EditIconPopup::pickFile(int index, std::string_view suffix) {
    m_listener.bind([this, index, suffix](Task<Result<std::vector<std::filesystem::path>>>::Event* e) {
        auto res = e->getValue();
        if (!res || res->isErr()) return;

        auto paths = res->unwrap();
        if (paths.empty()) return;

        m_pickerOpened = true;

        auto isIcon = m_iconType <= IconType::Jetpack;
        auto isTrail = m_iconType == IconType::Special;
        std::string name;
        std::filesystem::path png;
        std::filesystem::path plist;
        for (auto& path : paths) {
            if (path.extension() == ".png") {
                if (png.empty()) {
                    if (isTrail) name = string::pathToString(path.stem());
                    png = path;
                }
            }
            else if (path.extension() == ".plist") {
                if (plist.empty()) {
                    if (isIcon) {
                        name = string::pathToString(path.stem());
                        if (name.ends_with("-uhd")) name = name.substr(0, name.size() - 4);
                        else if (name.ends_with("-hd")) name = name.substr(0, name.size() - 3);
                    }
                    plist = path;
                }
            }
            if (!png.empty() && !plist.empty()) break;
        }

        auto pngEmpty = png.empty();
        auto plistEmpty = suffix.empty() && isIcon && plist.empty();
        if (pngEmpty && plistEmpty) return notify(NotificationIcon::Info, "Please select a PNG file and a Plist file");
        else if (plistEmpty) return notify(NotificationIcon::Info, "Please select a Plist file");
        else if (pngEmpty) return notify(NotificationIcon::Info, "Please select a PNG file");

        GEODE_UNWRAP_OR_ELSE(image, err, texpack::fromPNG(png))
            return notify(NotificationIcon::Error, "Failed to load image: {}", err);

        Autorelease texture = new CCTexture2D();
        texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
            (float)image.width,
            (float)image.height
        });

        if (!suffix.empty()) {
            m_frames->setObject(CCSpriteFrame::createWithTexture(texture, { { 0.0f, 0.0f }, texture->getContentSize() }),
                index > 0 ? fmt::format("_{:02}{}", index, suffix) : std::string(suffix));
            return updateSprites();
        }

        GEODE_UNWRAP_OR_ELSE(frames, err, MoreIconsAPI::createFrames(string::pathToString(plist), texture, "", m_iconType))
            return notify(NotificationIcon::Error, "Failed to load frames: {}", err);

        if (m_textInput->getString().empty() && !name.empty()) m_textInput->setString(name);

        if (isIcon) {
            m_frames->removeAllObjects();
            for (auto [frameName, frame] : CCDictionaryExt<gd::string, CCSpriteFrame*>(frames)) {
                m_frames->setObject(frame, frameName);
            }
            updateSprites();
        }
        else if (isTrail) m_streak->setTexture(texture);
    });

    m_listener.setFilter(file::pickMany({ std::nullopt, {{ "PNG/Plist files", { "*.png", "*.plist" } }} }));
}

void EditIconPopup::updateSprites() {
    auto crossFrame = MoreIconsAPI::get<CCSpriteFrameCache>()->spriteFrameByName("GJ_deleteIcon_001.png");
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

            if (auto glowChild = sprite->m_glowSprite->getChildByIndex<CCSprite>(i)) {
                glowChild->setDisplayFrame(static_cast<CCSpriteFrame*>(m_frames->objectForKey(fmt::format("_{:02}_glow_001.png", tag))));
            }

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
    m_parentPopup->close();
    Popup::onClose(nullptr);

    if (auto icon = MoreIconsAPI::getIcon(name, m_iconType)) MoreIconsAPI::updateIcon(icon);
    else {
        icon = MoreIconsAPI::addIcon(name, name, m_iconType,
            string::pathToString(png), string::pathToString(plist), "", "More Icons", 0, {}, false, false);
        if (MoreIconsAPI::preloadIcons) {
            MoreIconsAPI::createFrames(icon->textures[0], icon->sheetName, icon->name, icon->type).inspect([icon](const ImageResult& image) {
                MoreIconsAPI::addFrames(image, icon->frameNames);
            }).inspectErr([icon](const std::string& err) {
                log::error("{}: {}", icon->name, err);
            });
        }
    }

    notify(NotificationIcon::Success, "{} saved!", name);
    MoreIcons::updateGarage();
}

bool EditIconPopup::checkFrame(std::string_view suffix) {
    auto frame = m_frames->objectForKey({ suffix.data(), suffix.size() });
    if (!frame) notify(NotificationIcon::Info, "Missing {}{}.", m_textInput->getString(), suffix);
    return frame != nullptr;
}

texpack::Image getImage(CCSprite* sprite) {
    auto director = MoreIconsAPI::get<CCDirector>();
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
    if (GEODE_UNWRAP_IF_ERR(err, texpack::toPNG(path, getImage(m_streak)))) {
        return notify(NotificationIcon::Error, "Failed to save image: {}", err);
    }
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

    if (GEODE_UNWRAP_IF_ERR(err, packer.pack())) {
        return notify(NotificationIcon::Error, "Failed to pack frames: {}", err);
    }
    if (GEODE_UNWRAP_IF_ERR(err, packer.png(png))) {
        return notify(NotificationIcon::Error, "Failed to save image: {}", err);
    }
    if (GEODE_UNWRAP_IF_ERR(err, packer.plist(plist, "icons/" + string::pathToString(png.filename()), "    "))) {
        return notify(NotificationIcon::Error, "Failed to save plist: {}", err);
    }

    addOrUpdateIcon(name, png, plist);
}

void EditIconPopup::onClose(CCObject* sender) {
    if (!m_pickerOpened && (!m_textInput || m_textInput->getString().empty())) return Popup::onClose(sender);

    auto type = MoreIconsAPI::convertType(m_iconType);
    createQuickPopup(
        fmt::format("Exit {} Editor", MoreIconsAPI::uppercase[type]).c_str(),
        fmt::format("Are you sure you want to <cy>exit</c> the <cg>{} editor</c>?\n<cr>All unsaved changes will be lost!</c>",
            MoreIconsAPI::lowercase[type]),
        "No",
        "Yes",
        [this, sender](auto, bool btn2) {
            if (btn2) Popup::onClose(sender);
        }
    );
}

#include "EditTrailPopup.hpp"
#include "IconPresetPopup.hpp"
#include "../MoreIconsPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Notification.hpp>
#include <texpack.hpp>

using namespace geode::prelude;

EditTrailPopup* EditTrailPopup::create(MoreIconsPopup* popup) {
    auto ret = new EditTrailPopup();
    if (ret->initAnchored(350.0f, 180.0f, popup, "geode.loader/GE_square03.png")) {
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

bool EditTrailPopup::setup(MoreIconsPopup* popup) {
    setID("EditTrailPopup");
    setTitle("Trail Editor");
    m_title->setID("edit-trail-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_parentPopup = popup;

    m_streak = CCSprite::create("streak_01_001.png");
    m_streak->setBlendFunc({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA });
    m_streak->setPosition({ 175.0f, 120.0f });
    m_streak->setRotation(-90.0f);
    auto& size = m_streak->getContentSize();
    m_streak->setScaleX(14.0f / size.width);
    m_streak->setScaleY(320.0f / size.height);
    m_streak->setID("streak-preview");
    m_mainLayer->addChild(m_streak);

    m_nameInput = TextInput::create(300.0f, "Name");
    m_nameInput->setPosition({ 175.0f, 75.0f });
    m_nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-. ");
    m_nameInput->setMaxCharCount(100);
    m_nameInput->setID("text-input");
    m_mainLayer->addChild(m_nameInput);

    auto bottomMenu = CCMenu::create();
    bottomMenu->setPosition({ 175.0f, 30.0f });
    bottomMenu->setContentSize({ 350.0f, 30.0f });
    bottomMenu->setLayout(RowLayout::create()->setGap(25.0f));
    bottomMenu->setID("bottom-menu");

    auto pngButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("PNG", "goldFont.fnt", "GJ_button_05.png"), [this](auto) {
        m_listener.bind([this](Task<Result<std::filesystem::path>>::Event* event) {
            auto res = event->getValue();
            if (res && res->isErr()) return notify(NotificationIcon::Error, "Failed to import PNG file: {}", res->unwrapErr());
            if (!res || !res->isOk()) return;

            m_hasChanged = true;

            auto imageRes = texpack::fromPNG(res->unwrap());
            if (imageRes.isErr()) return notify(NotificationIcon::Error, "Failed to load image: {}", imageRes.unwrapErr());

            auto image = std::move(imageRes).unwrap();

            Autorelease texture = new CCTexture2D();
            texture->initWithData(image.data.data(), kCCTexture2DPixelFormat_RGBA8888, image.width, image.height, {
                (float)image.width,
                (float)image.height
            });

            m_streak->setTexture(texture);
        });

        m_listener.setFilter(file::pick(file::PickMode::OpenFile, {
            .filters = {{
                .description = "PNG files",
                .files = { "*.png" }
            }}
        }));
    });
    pngButton->setID("png-button");
    bottomMenu->addChild(pngButton);

    auto presetButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Preset", "goldFont.fnt", "GJ_button_05.png"), [this](auto) {
        IconPresetPopup::create(IconType::Special, {}, [this](int id, IconInfo* info) {
            m_streak->setTexture(MoreIconsAPI::getTextureCache()->addImage(
                info ? info->textures[0].c_str() : fmt::format("streak_{:02}_001.png", id).c_str(), false));
            m_hasChanged = true;
        })->show();
    });
    presetButton->setID("preset-button");
    bottomMenu->addChild(presetButton);

    auto saveButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png"), [this](auto) {
        auto iconName = m_nameInput->getString();
        if (iconName.empty()) return notify(NotificationIcon::Info, "Please enter a name.");

        std::filesystem::path path = (Mod::get()->getConfigDir() / MI_PATH("trail") / MoreIconsAPI::strPath(iconName)).native() + MI_PATH(".png");
        if (MoreIcons::doesExist(path)) createQuickPopup(
            "Existing Trail",
            fmt::format("<cy>{}</c> already exists.\nDo you want to <cr>overwrite</c> it?", iconName),
            "No",
            "Yes",
            [this, path = std::move(path)](auto, bool btn2) {
                if (btn2) saveTrail(path);
            }
        );
        else saveTrail(path);
    });
    saveButton->setID("save-button");
    bottomMenu->addChild(saveButton);

    bottomMenu->updateLayout();
    m_mainLayer->addChild(bottomMenu);

    handleTouchPriority(this);

    return true;
}

void EditTrailPopup::addOrUpdateIcon(const std::string& name, const std::filesystem::path& path) {
    if (auto icon = MoreIconsAPI::getIcon(name, IconType::Special)) MoreIconsAPI::updateIcon(icon);
    else {
        icon = MoreIconsAPI::addIcon(name, name, IconType::Special, string::pathToString(path), {}, {}, "More Icons", 0, {}, false, false);
        if (MoreIconsAPI::preloadIcons) {
            if (auto res = MoreIconsAPI::createFrames(MoreIconsAPI::strPath(icon->textures[0]), std::filesystem::path(), icon->name, icon->type)) {
                MoreIconsAPI::addFrames(res.unwrap(), icon->frameNames);
            }
            else {
                log::error("{}: {}", icon->name, res.unwrapErr());
            }
        }
    }

    m_parentPopup->close();
    Popup::onClose(nullptr);

    notify(NotificationIcon::Success, "{} saved!", name);
    MoreIcons::updateGarage();
}

texpack::Image getImage(CCSprite* sprite) {
    auto director = MoreIconsAPI::getDirector();
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

void EditTrailPopup::saveTrail(const std::filesystem::path& path) {
    if (auto res = texpack::toPNG(path, getImage(m_streak)); res.isErr()) {
        return notify(NotificationIcon::Error, "Failed to save image: {}", res.unwrapErr());
    }
    addOrUpdateIcon(m_nameInput->getString(), path);
}

void EditTrailPopup::onClose(CCObject* sender) {
    if (!m_hasChanged && (!m_nameInput || m_nameInput->getString().empty())) return Popup::onClose(sender);
    createQuickPopup(
        "Exit Trail Editor",
        "Are you sure you want to <cy>exit</c> the <cg>trail editor</c>?\n<cr>All unsaved changes will be lost!</c>",
        "No",
        "Yes",
        [this, sender](auto, bool btn2) {
            if (btn2) Popup::onClose(sender);
        }
    );
}

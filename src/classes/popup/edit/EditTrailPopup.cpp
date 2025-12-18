#include "EditTrailPopup.hpp"
#include "IconPresetPopup.hpp"
#include "ImageRenderer.hpp"
#include "../MoreIconsPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Defaults.hpp"
#include "../../../utils/Get.hpp"
#include "../../../utils/Load.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <MoreIcons.hpp>

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
            if (res && res->isErr()) return MoreIcons::notifyFailure("Failed to import PNG file: {}", res->unwrapErr());
            if (!res || !res->isOk()) return;

            updateWithPath(res->unwrap());
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
            updateWithPath(info ? info->getTexture() : MoreIcons::strPath(MoreIcons::getTrailTexture(id)));
        })->show();
    });
    presetButton->setID("preset-button");
    bottomMenu->addChild(presetButton);

    auto saveButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_05.png"), [this](auto) {
        auto iconName = m_nameInput->getString();
        if (iconName.empty()) return MoreIcons::notifyInfo("Please enter a name.");

        auto path = MoreIcons::getIconStem(fmt::format("{}.png", iconName), IconType::Special);
        if (MoreIcons::doesExist(path)) createQuickPopup(
            "Existing Trail",
            fmt::format("<cy>{}</c> already exists.\nDo you want to <cr>overwrite</c> it?", iconName),
            "No",
            "Yes",
            [this, path = std::move(path)](auto, bool btn2) {
                if (btn2) saveTrail(std::move(path));
            }
        );
        else saveTrail(std::move(path));
    });
    saveButton->setID("save-button");
    bottomMenu->addChild(saveButton);

    bottomMenu->updateLayout();
    m_mainLayer->addChild(bottomMenu);

    handleTouchPriority(this);

    return true;
}

void EditTrailPopup::updateWithPath(const std::filesystem::path& path) {
    if (auto textureRes = Load::createTexture(path); textureRes.isOk()) {
        m_streak->setTexture(textureRes.unwrap());
        m_hasChanged = true;
    }
    else if (textureRes.isErr()) MoreIcons::notifyFailure(textureRes.unwrapErr());
}

void EditTrailPopup::saveTrail(std::filesystem::path path) {
    auto sprite = CCSprite::createWithTexture(m_streak->getTexture());
    sprite->setAnchorPoint({ 0.0f, 0.0f });
    sprite->setBlendFunc({ GL_ONE, GL_ZERO });
    auto image = ImageRenderer::getImage(sprite);
    sprite->release();
    if (auto res = texpack::toPNG(path, image); res.isErr()) {
        return MoreIcons::notifyFailure("Failed to save image: {}", res.unwrapErr());
    }

    auto name = m_nameInput->getString();

    if (auto icon = more_icons::getIcon(name, IconType::Special)) {
        more_icons::updateIcon(icon);
    }
    else {
        auto jsonPath = std::filesystem::path(path).replace_extension(L(".json"));
        (void)file::writeString(jsonPath, "{}");
        icon = more_icons::addTrail(
            name, name, std::move(path), std::move(jsonPath), {},
            {}, "More Icons", 0, Defaults::getTrailInfo(0), false, false
        );
        if (MoreIcons::preloadIcons) MoreIcons::createAndAddFrames(icon);
    }

    m_parentPopup->close();
    Popup::onClose(nullptr);

    MoreIcons::notifySuccess("{} saved!", name);
    MoreIcons::updateGarage();
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

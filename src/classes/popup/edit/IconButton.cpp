#include "IconButton.hpp"
#include "ImageRenderer.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Filesystem.hpp"
#include "../../../utils/Get.hpp"
#include "../../../utils/Load.hpp"
#include "../../../utils/Notify.hpp"
#include <Geode/utils/file.hpp>

using namespace geode::prelude;

IconButton* IconButton::create() {
    auto ret = new IconButton();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool IconButton::init() {
    if (!CCMenuItemSpriteExtra::init(CCSprite::create(), nullptr, nullptr, nullptr)) return false;

    auto normalImage = getNormalImage();
    setContentSize({ 30.0f, 30.0f });
    normalImage->setPosition({ 15.0f, 15.0f });

    m_unselectedSprite = CCSprite::createWithSpriteFrameName("player_special_01_001.png");
    m_unselectedSprite->setColor({ 150, 150, 150 });
    normalImage->addChild(m_unselectedSprite);

    m_plusLabel = CCLabelBMFont::create("+", "bigFont.fnt");
    m_plusLabel->setPosition({ 0.75f, 2.25f });
    m_plusLabel->setScale(0.75f);
    normalImage->addChild(m_plusLabel);

    return true;
}

void IconButton::activate() {
    CCMenuItemSpriteExtra::activate();

    m_listener.spawn(file::pick(file::PickMode::OpenFile, {
        .filters = {{
            .description = "PNG files",
            .files = { "*.png" }
        }}
    }), [this](Result<std::optional<std::filesystem::path>> res) {
        if (res.isErr()) return Notify::error("Failed to import PNG file: {}", res.unwrapErr());

        auto path = std::move(res).unwrap();
        if (!path.has_value()) return;

        auto textureRes = Load::createTexture(path.value());
        if (textureRes.isErr()) {
            return Notify::error("Failed to load texture: {}", textureRes.unwrapErr());
        }

        if (m_unselectedSprite) {
            m_unselectedSprite->removeFromParent();
            m_unselectedSprite = nullptr;
        }
        if (m_plusLabel) {
            m_plusLabel->removeFromParent();
            m_plusLabel = nullptr;
        }
        MoreIcons::setTexture(static_cast<CCSprite*>(getNormalImage()), textureRes.unwrap());
        updateSprite();
    });
}

std::filesystem::path IconButton::saveIcon(const std::filesystem::path& path) {
    std::filesystem::path iconPath;
    if (!m_unselectedSprite && !m_plusLabel) {
        auto texture = static_cast<CCSprite*>(getNormalImage())->getTexture();
        auto scaleFactor = Get::director->getContentScaleFactor();
        std::array scales = { 4.0f / scaleFactor, 2.0f / scaleFactor, 1.0f / scaleFactor };
        std::array names = { L("icon-uhd.png"), L("icon-hd.png"), L("icon.png") };
        for (int i = 0; i < 3; i++) {
            auto scale = scales[i];
            auto sprite = CCSprite::createWithTexture(texture);
            sprite->setScale(scale);
            sprite->setAnchorPoint({ 0.0f, 0.0f });
            sprite->setBlendFunc({ GL_ONE, GL_ZERO });
            auto iconImage = ImageRenderer::getImage(sprite);
            sprite->release();

            auto iconImageRes = texpack::toPNG(iconImage);
            if (iconImageRes.isErr()) {
                Notify::error("Failed to encode icon image: {}", iconImageRes.unwrapErr());
            }

            auto savePath = path / names[i];
            if (auto res = file::writeBinary(savePath, iconImageRes.unwrap()); res.isErr()) {
                Notify::error("Failed to save icon image: {}", res.unwrapErr());
            }

            if (scale == 1.0f) iconPath = std::move(savePath);
        }
    }
    return iconPath;
}

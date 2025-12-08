#include "LoadEditorPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../scroll/BiggerScrollLayer.hpp"
#include <algorithm>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/ui/Scrollbar.hpp>
#include <Geode/utils/string.hpp>

using namespace geode::prelude;

LoadEditorPopup* LoadEditorPopup::create(IconType type, std23::move_only_function<void(const std::filesystem::path&)> callback) {
    auto ret = new LoadEditorPopup();
    if (ret->initAnchored(230.0f, 250.0f, type, std::move(callback), "geode.loader/GE_square03.png")) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LoadEditorPopup::setup(IconType type, std23::move_only_function<void(const std::filesystem::path&)> callback) {
    setID("LoadEditorPopup");
    setTitle(fmt::format("Load {} Editor", MoreIcons::uppercase[(int)type]));
    m_title->setID("load-editor-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    m_callback = std::move(callback);

    auto scrollLayer = BiggerScrollLayer::create(200.0f, 190.0f, 5.0f, 5.0f);
    scrollLayer->setPosition({ 110.0f, 115.0f });
    scrollLayer->m_contentLayer->setLayout(ColumnLayout::create()->setAxisAlignment(AxisAlignment::End)->setAutoGrowAxis(200.0f));
    scrollLayer->setID("scroll-layer");
    m_mainLayer->addChild(scrollLayer);

    std::vector<std::filesystem::path> entries;

    MoreIcons::iterate(MoreIcons::getEditorDir(type), std::filesystem::file_type::directory, [&entries](const std::filesystem::path& path) {
        entries.insert(std::ranges::find_if(entries, [filename = string::pathToString(path.filename())](const std::filesystem::path& p) {
            return string::caseInsensitiveCompare(filename, string::pathToString(p.filename())) != std::strong_ordering::less;
        }), path);
    });

    for (auto& path : entries) {
        auto filename = string::pathToString(path.filename());

        auto entryMenu = CCMenu::create();
        entryMenu->setContentSize({ 200.0f, 30.0f });
        entryMenu->ignoreAnchorPointForPosition(false);
        entryMenu->setID(fmt::format("{}-menu", filename));
        scrollLayer->m_contentLayer->addChild(entryMenu);

        auto entryButton = CCMenuItemExt::createSpriteExtra(
            ButtonSprite::create(filename.c_str(), 174, 0, 1.0f, false, "goldFont.fnt", "GJ_button_05.png", 0.0f),
            [this, path = std::move(path)](auto) {
                m_callback(path);
                onClose(nullptr);
            }
        );
        entryButton->setPosition({ 100.0f, 15.0f });
        entryButton->setID(std::move(filename));
        entryMenu->addChild(entryButton);
    }

    scrollLayer->m_contentLayer->updateLayout();
    scrollLayer->scrollToTop();

    auto scrollbar = Scrollbar::create(scrollLayer);
    scrollbar->setPosition({ 217.0f, 115.0f });
    scrollbar->setID("scrollbar");
    m_mainLayer->addChild(scrollbar);

    handleTouchPriority(this);

    return true;
}

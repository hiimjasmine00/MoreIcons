#include "LoadEditorPopup.hpp"
#include "../../../MoreIcons.hpp"
#include <algorithm>
#ifdef GEODE_IS_WINDOWS
#include <cwctype>
#endif
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/ui/Scrollbar.hpp>
#include <Geode/ui/ScrollLayer.hpp>
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

    auto scrollBackground = CCLayerColor::create({ 0, 0, 0, 105 }, 200.0f, 200.0f);
    scrollBackground->setPosition({ 110.0f, 115.0f });
    scrollBackground->ignoreAnchorPointForPosition(false);
    scrollBackground->setID("scroll-background");
    m_mainLayer->addChild(scrollBackground);

    auto scrollLayer = ScrollLayer::create({ 200.0f, 200.0f });
    auto contentLayer = scrollLayer->m_contentLayer;
    scrollLayer->setPosition({ 110.0f, 115.0f });
    scrollLayer->ignoreAnchorPointForPosition(false);
    contentLayer->setLayout(ColumnLayout::create()->setAxisReverse(true)
        ->setAxisAlignment(AxisAlignment::End)->setAutoGrowAxis(0.0f));
    scrollLayer->setID("scroll-layer");
    m_mainLayer->addChild(scrollLayer);

    std::vector<std::filesystem::path> entries;

    MoreIcons::iterate(MoreIcons::getEditorDir(type), std::filesystem::file_type::directory, [&entries](const std::filesystem::path& path) {
        auto a = MoreIcons::getPathFilename(path);
        entries.insert(std::ranges::find_if(entries, [a](const std::filesystem::path& path) {
            auto b = MoreIcons::getPathFilename(path);
            if (a == b) return false;
            for (size_t i = 0; i < a.size() && i < b.size(); i++) {
                #ifdef GEODE_IS_WINDOWS
                auto charA = std::towlower(a[i]);
                auto charB = std::towlower(b[i]);
                #else
                auto charA = std::tolower(a[i]);
                auto charB = std::tolower(b[i]);
                #endif
                if (charA != charB) return charA < charB;
            }
            return a.size() <= b.size();
        }), path);
    });

    for (auto& path : entries) {
        auto filename = string::pathToString(path.filename());

        auto entryMenu = CCMenu::create();
        entryMenu->setContentSize({ 200.0f, 30.0f });
        entryMenu->ignoreAnchorPointForPosition(false);
        entryMenu->setID(fmt::format("{}-menu", filename));
        contentLayer->addChild(entryMenu);

        auto entryButton = CCMenuItemExt::createSpriteExtra(
            ButtonSprite::create(filename.c_str(), 174, 0, 1.0f, false, "goldFont.fnt", "GJ_button_05.png", 0.0f),
            [this, path = std::move(path)](auto) {
                m_callback(path);
                onClose(nullptr);
            }
        );
        entryButton->setPosition({ 100.0f, 20.0f });
        entryButton->setID(std::move(filename));
        entryMenu->addChild(entryButton);
    }

    contentLayer->updateLayout();

    contentLayer->setContentSize(contentLayer->getContentSize() + CCSize { 0.0f, 10.0f });
    for (auto child : contentLayer->getChildrenExt()) {
        child->setPosition(child->getPosition() + CCPoint { 0.0f, 5.0f });
        child->setContentSize(child->getContentSize() + CCSize { 0.0f, 10.0f });
    }

    scrollLayer->scrollToTop();

    auto scrollable = contentLayer->getContentHeight() > scrollLayer->getContentHeight();
    scrollLayer->enableScrollWheel(scrollable);

    auto scrollbar = Scrollbar::create(scrollLayer);
    scrollbar->setPosition({ 217.0f, 115.0f });
    scrollbar->setTouchEnabled(scrollable);
    scrollbar->setID("scrollbar");
    m_mainLayer->addChild(scrollbar);

    handleTouchPriority(this);

    return true;
}

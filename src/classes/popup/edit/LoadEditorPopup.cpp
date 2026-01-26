#include "LoadEditorPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Filesystem.hpp"
#include <algorithm>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/ui/Scrollbar.hpp>
#include <Geode/ui/ScrollLayer.hpp>

using namespace geode::prelude;

LoadEditorPopup* LoadEditorPopup::create(IconType type, Function<void(const std::filesystem::path&, std::string_view)> callback) {
    auto ret = new LoadEditorPopup();
    if (ret->init(type, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LoadEditorPopup::init(IconType type, Function<void(const std::filesystem::path&, std::string_view)> callback) {
    if (!BasePopup::init(230.0f, 250.0f, "geode.loader/GE_square03.png")) return false;

    setID("LoadEditorPopup");
    setTitle(fmt::format("Load {} Editor", Constants::getSingularUppercase(type)));
    m_title->setID("load-editor-title");

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
    contentLayer->ignoreAnchorPointForPosition(false);
    scrollLayer->setID("scroll-layer");
    m_mainLayer->addChild(scrollLayer);

    std::vector<std::filesystem::path> entries;

    Filesystem::iterate(MoreIcons::getEditorDir(type), std::filesystem::file_type::directory, [&entries](const std::filesystem::path& path) {
        auto a = Filesystem::filenameView(path);
        entries.insert(std::ranges::find_if(entries, [a](const std::filesystem::path& path) {
            auto b = Filesystem::filenameView(path);
            if (a == b) return false;
            for (size_t i = 0; i < a.size() && i < b.size(); i++) {
                #ifdef GEODE_IS_WINDOWS
                auto charA = towlower(a[i]);
                auto charB = towlower(b[i]);
                #else
                auto charA = tolower(a[i]);
                auto charB = tolower(b[i]);
                #endif
                if (charA != charB) return charA < charB;
            }
            return a.size() <= b.size();
        }), path);
    });

    for (auto& path : entries) {
        auto filename = std::string(Filesystem::strNarrow(Filesystem::filenameView(path)));

        auto entryMenu = CCMenu::create();
        entryMenu->setContentSize({ 200.0f, 30.0f });
        entryMenu->ignoreAnchorPointForPosition(false);
        entryMenu->setID(fmt::format("{}-menu", filename));
        contentLayer->addChild(entryMenu);

        auto entryButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(filename.c_str(), 174, 0, 1.0f, false, "goldFont.fnt", "GJ_button_05.png", 0.0f),
            this, menu_selector(LoadEditorPopup::onEntry)
        );
        entryButton->setUserObject("entry-path", ObjWrapper<std::filesystem::path>::create(std::move(path)));
        entryButton->setPosition({ 100.0f, 20.0f });
        entryButton->setID(std::move(filename));
        entryMenu->addChild(entryButton);
    }

    contentLayer->setLayout(ColumnLayout::create()->setAxisReverse(true)->setAxisAlignment(AxisAlignment::End)->setAutoGrowAxis(0.0f));

    contentLayer->setContentSize(contentLayer->getContentSize() + CCSize { 0.0f, 10.0f });
    for (auto child : contentLayer->getChildrenExt()) {
        child->setPosition(child->getPosition() + CCPoint { 0.0f, 5.0f });
        child->setContentSize(child->getContentSize() + CCSize { 0.0f, 10.0f });
    }

    scrollLayer->scrollToTop();

    auto scrollable = contentLayer->getContentHeight() > scrollLayer->getContentHeight();
    scrollLayer->enableScrollWheel(scrollable);
    scrollLayer->setTouchEnabled(scrollable);

    auto scrollbar = Scrollbar::create(scrollLayer);
    scrollbar->setPosition({ 217.0f, 115.0f });
    scrollbar->setTouchEnabled(scrollable);
    scrollbar->setID("scrollbar");
    m_mainLayer->addChild(scrollbar);

    handleTouchPriority(this);

    return true;
}

void LoadEditorPopup::onEntry(CCObject* sender) {
    auto node = static_cast<CCNode*>(sender);
    m_callback(static_cast<ObjWrapper<std::filesystem::path>*>(node->getUserObject("entry-path"))->getValue(), node->getID());
    close();
}

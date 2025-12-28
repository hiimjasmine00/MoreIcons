#include "IconPresetPopup.hpp"
#include "../../misc/LazyIcon.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Get.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/ui/Scrollbar.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

IconPresetPopup* IconPresetPopup::create(IconType type, std::string_view suffix, std23::move_only_function<void(int, IconInfo*)> callback) {
    auto ret = new IconPresetPopup();
    if (ret->init(type, suffix, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool IconPresetPopup::init(IconType type, std::string_view suffix, std23::move_only_function<void(int, IconInfo*)> callback) {
    if (!BasePopup::init(440.0f, 290.0f, "geode.loader/GE_square03.png")) return false;

    setID("IconPresetPopup");
    setTitle(fmt::format("{} Presets", Constants::getSingularUppercase(type)));
    m_title->setID("icon-preset-title");

    m_callback = std::move(callback);

    auto scrollBackground = CCLayerColor::create({ 0, 0, 0, 105 }, 400.0f, 240.0f);
    scrollBackground->setPosition({ 215.0f, 135.0f });
    scrollBackground->ignoreAnchorPointForPosition(false);
    scrollBackground->setID("scroll-background");
    m_mainLayer->addChild(scrollBackground);

    auto gameManager = Get::GameManager();
    auto scrollLayer = ScrollLayer::create({ 400.0f, 240.0f });
    auto contentLayer = scrollLayer->m_contentLayer;
    scrollLayer->setPosition({ 215.0f, 135.0f });
    scrollLayer->ignoreAnchorPointForPosition(false);
    contentLayer->setLayout(RowLayout::create()->setGap(Constants::getIconGap(type))->setGrowCrossAxis(true));
    scrollLayer->setID("scroll-layer");
    m_mainLayer->addChild(scrollLayer);

    auto count = gameManager->countForType(type);
    for (int i = 1; i <= count; i++) {
        auto iconMenu = CCMenu::create();
        auto lazyIcon = LazyIcon::create(type, i, nullptr, suffix, [this, i] {
            if (m_callback) m_callback(i, nullptr);
            onClose(nullptr);
        });
        lazyIcon->setPosition({ 15.0f, 30.0f });
        iconMenu->setContentSize({ 30.0f, 30.0f });
        iconMenu->ignoreAnchorPointForPosition(false);
        iconMenu->setID(fmt::format("{}-menu", lazyIcon->getID()));
        iconMenu->addChild(lazyIcon);
        contentLayer->addChild(iconMenu);
    }

    if (auto icons = more_icons::getIcons(type)) {
        auto end = icons->data() + icons->size();
        for (auto info = icons->data(); info != end; info++) {
            auto iconMenu = CCMenu::create();
            auto lazyIcon = LazyIcon::create(type, 0, info, suffix, [this, info] {
                if (m_callback) m_callback(0, info);
                onClose(nullptr);
            });
            lazyIcon->setPosition({ 15.0f, 30.0f });
            iconMenu->setContentSize({ 30.0f, 30.0f });
            iconMenu->ignoreAnchorPointForPosition(false);
            iconMenu->setID(fmt::format("{}-menu", lazyIcon->getID()));
            iconMenu->addChild(lazyIcon);
            contentLayer->addChild(iconMenu);
        }
    }

    contentLayer->updateLayout();

    contentLayer->setContentSize(contentLayer->getContentSize() + CCSize { 0.0f, 10.0f });
    for (auto child : contentLayer->getChildrenExt()) {
        child->setPosition(child->getPosition() + CCPoint { 0.0f, 5.0f });
        child->setContentSize(child->getContentSize() + CCSize { 0.0f, 30.0f });
    }

    scrollLayer->scrollToTop();

    auto scrollable = contentLayer->getContentHeight() > scrollLayer->getContentHeight();
    scrollLayer->enableScrollWheel(scrollable);
    scrollLayer->setTouchEnabled(scrollable);

    auto scrollbar = Scrollbar::create(scrollLayer);
    scrollbar->setPosition({ 425.0f, 135.0f });
    scrollbar->setTouchEnabled(scrollable);
    scrollbar->setID("scrollbar");
    m_mainLayer->addChild(scrollbar);

    handleTouchPriority(this);

    return true;
}

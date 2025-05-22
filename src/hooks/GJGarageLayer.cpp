#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include "../classes/misc/ButtonHooker.hpp"
#include "../classes/popup/MoreIconsPopup.hpp"
#include "../classes/popup/info/MoreInfoPopup.hpp"
#include <Geode/binding/BoomScrollLayer.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/ListButtonBar.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/GJGarageLayer.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

using namespace geode::prelude;

class $modify(MIGarageLayer, GJGarageLayer) {
    struct Fields {
        ListButtonBar* m_pageBar;
        CCMenu* m_navMenu;
        std::map<IconType, int> m_pages;
        std::string m_selectedIcon;
        bool m_initialized;
    };

    static void onModify(ModifyBase<ModifyDerive<MIGarageLayer, GJGarageLayer>>& self) {
        (void)self.setHookPriorityAfterPost("GJGarageLayer::init", "weebify.separate_dual_icons");
        (void)self.setHookPriorityBeforePre("GJGarageLayer::onSelect", "weebify.separate_dual_icons");
        (void)self.setHookPriorityBeforePost("GJGarageLayer::setupPage", "weebify.separate_dual_icons");
    }

    bool init() {
        if (!GJGarageLayer::init()) return false;

        auto f = m_fields.self();
        f->m_initialized = true;
        f->m_pages[IconType::Cube] = m_iconPages[IconType::Cube];

        MoreIconsAPI::updateSimplePlayer(m_playerObject, GameManager::get()->m_playerIconType, false);
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        if (sdi) MoreIconsAPI::updateSimplePlayer(
            static_cast<SimplePlayer*>(getChildByID("player2-icon")), (IconType)sdi->getSavedValue("lastmode", 0), true);

        auto customIcon = MoreIconsAPI::activeIcon(IconType::Cube, false);
        if (!customIcon.empty() && MoreIconsAPI::hasIcon(customIcon, IconType::Cube)) setupCustomPage(findIconPage(IconType::Cube));
        else createNavMenu();

        auto shardsMenu = getChildByID("shards-menu");
        if (sdi) {
            if (auto playerButtonsMenu = getChildByID("player-buttons-menu")) {
                ButtonHooker::create(
                    static_cast<CCMenuItem*>(playerButtonsMenu->getChildByID("player1-button")),
                    this, menu_selector(MIGarageLayer::newOn2PToggle)
                );
                ButtonHooker::create(
                    static_cast<CCMenuItem*>(playerButtonsMenu->getChildByID("player2-button")),
                    this, menu_selector(MIGarageLayer::newOn2PToggle)
                );
            }

            if (shardsMenu) ButtonHooker::create(
                static_cast<CCMenuItem*>(shardsMenu->getChildByID("swap-2p-button")),
                this, menu_selector(MIGarageLayer::newSwap2PKit)
            );
        }

        auto moreIconsSprite = CircleButtonSprite::createWithSprite("MI_moreIcons_001.png"_spr, 1.0f, CircleBaseColor::Gray, CircleBaseSize::Small);
        if (MoreIcons::severity > Severity::Debug) {
            auto severitySprite = CCSprite::createWithSpriteFrameName(MoreIcons::severityFrames[MoreIcons::severity]);
            severitySprite->setPosition(moreIconsSprite->getContentSize() - CCPoint { 6.0f, 6.0f });
            severitySprite->setScale(0.6f);
            moreIconsSprite->addChild(severitySprite, 1);
        }
        auto moreIconsButton = CCMenuItemSpriteExtra::create(moreIconsSprite, this, menu_selector(MIGarageLayer::onMoreIcons));
        moreIconsButton->setID("more-icons-button"_spr);
        if (shardsMenu) {
            shardsMenu->addChild(moreIconsButton);
            shardsMenu->updateLayout();
        }

        return true;
    }

    void onMoreIcons(CCObject* sender) {
        MoreIconsPopup::create()->show();
    }

    static int findIconPage(IconType type) {
        auto gameManager = GameManager::get();
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto active = MoreIconsAPI::activeIcon(type, sdi && sdi->getSavedValue("2pselected", false));
        auto it = std::ranges::find_if(MoreIconsAPI::icons, [&active, type](const IconInfo& info) {
            return info.name == active && info.type == type;
        });
        return it != MoreIconsAPI::icons.end() && MoreIconsAPI::iconIndices.contains(type) ?
            (gameManager->countForType(type) + 35) / 36 + (it - MoreIconsAPI::iconIndices[type].first - MoreIconsAPI::icons.begin()) / 36 :
            (gameManager->activeIconForType(type) - 1) / 36;
    }

    void onSelect(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        if (btn->getUserObject("name"_spr)) {
            m_cursor1->setOpacity(255);
            return m_iconType == IconType::Special ? onCustomSpecialSelect(btn) : onCustomSelect(btn);
        }

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        if (MoreIconsAPI::hasIcon(MoreIconsAPI::activeIcon(m_iconType, dual), m_iconType)) m_iconID = 0;

        GJGarageLayer::onSelect(sender);

        if (!GameManager::get()->isIconUnlocked(sender->getTag(), btn->m_iconType)) return;

        if (btn->m_iconType != IconType::ShipFire) {
            m_cursor1->setOpacity(255);
            m_fields->m_selectedIcon = "";
            MoreIconsAPI::setIcon("", dual ? (IconType)Loader::get()->getLoadedMod(
                "weebify.separate_dual_icons")->getSavedValue("lasttype", 0) : m_selectedIconType, dual);
        }
    }

    void newOn2PToggle(CCObject* sender) {
        CALL_BUTTON_ORIGINAL(sender);

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto active = MoreIconsAPI::activeIcon(m_iconType, sdi && sdi->getSavedValue("2pselected", false));
        if (MoreIconsAPI::hasIcon(active, m_iconType)) {
            if (m_cursor1->isVisible()) m_cursor1->setOpacity(127);
            else m_cursor1->setOpacity(255);
        }
        else m_cursor1->setOpacity(255);

        setupCustomPage(m_fields->m_pages[m_iconType]);
    }

    static void swapDual(IconType type) {
        MoreIconsAPI::setIcon(MoreIconsAPI::setIcon(MoreIconsAPI::activeIcon(type, true), type, false), type, true);
    }

    void newSwap2PKit(CCObject* sender) {
        CALL_BUTTON_ORIGINAL(sender);

        swapDual(IconType::Cube);
        swapDual(IconType::Ship);
        swapDual(IconType::Ball);
        swapDual(IconType::Ufo);
        swapDual(IconType::Wave);
        swapDual(IconType::Robot);
        swapDual(IconType::Spider);
        swapDual(IconType::Swing);
        swapDual(IconType::Jetpack);
        swapDual(IconType::Special);
        MoreIconsAPI::updateSimplePlayer(m_playerObject, GameManager::get()->m_playerIconType, false);
        MoreIconsAPI::updateSimplePlayer(static_cast<SimplePlayer*>(getChildByID("player2-icon")),
            (IconType)Loader::get()->getLoadedMod("weebify.separate_dual_icons")->getSavedValue("lastmode", 0), true);
        setupCustomPage(m_fields->m_pages[m_iconType]);
    }

    void updatePlayerColors() {
        GJGarageLayer::updatePlayerColors();

        if (m_iconSelection && m_fields->m_pageBar && MoreIconsAPI::getCount(m_iconType) > 0) m_iconSelection->setVisible(false);
    }

    void createNavMenu() {
        auto f = m_fields.self();
        auto winSize = CCDirector::get()->getWinSize();
        if (!f->m_navMenu) {
            f->m_navMenu = CCMenu::create();
            f->m_navMenu->setPosition({ winSize.width / 2, 15.0f });
            f->m_navMenu->setContentSize({ winSize.width - 60.0f, 20.0f });
            f->m_navMenu->setLayout(RowLayout::create()->setGap(6.0f)->setAxisAlignment(AxisAlignment::Center));
            f->m_navMenu->setID("navdot-menu"_spr);
            addChild(f->m_navMenu, 1);
        }

        auto iconCount = MoreIconsAPI::getCount(m_iconType);
        m_navDotMenu->setPositionY(iconCount > 0 ? 35.0f : 25.0f);
        auto count = (GameManager::get()->countForType(m_iconType) + 35) / 36;
        if (count < 2) {
            m_navDotMenu->setVisible(true);
            m_navDotMenu->setEnabled(true);
            m_navDotMenu->removeAllChildren();
            auto firstDot = static_cast<CCMenuItemSprite*>(m_pageButtons->objectAtIndex(0));
            static_cast<CCSprite*>(firstDot->getNormalImage())->setDisplayFrame(CCSpriteFrameCache::get()->spriteFrameByName(
                f->m_pageBar && iconCount > 0 ? "gj_navDotBtn_off_001.png" : "gj_navDotBtn_on_001.png"));
            m_navDotMenu->addChild(firstDot);
            m_navDotMenu->updateLayout();
            m_leftArrow->setVisible(true);
            m_leftArrow->setEnabled(true);
            m_rightArrow->setVisible(true);
            m_rightArrow->setEnabled(true);
        }
        f->m_navMenu->setVisible(iconCount > 0);
        if (iconCount <= 0) return;

        f->m_navMenu->removeAllChildren();
        auto navDotAmount = (iconCount + 35) / 36;
        for (int i = count; i < navDotAmount + count; i++) {
            auto pages = f->m_pages;
            auto dotSprite = CCSprite::createWithSpriteFrameName(
                pages.contains(m_iconType) && i == pages[m_iconType] ? "gj_navDotBtn_on_001.png" : "gj_navDotBtn_off_001.png");
            dotSprite->setScale(0.9f);
            auto dot = CCMenuItemSpriteExtra::create(dotSprite, this, menu_selector(MIGarageLayer::onCustomNavigate));
            dot->setTag(i);
            dot->setSizeMult(1.1f);
            f->m_navMenu->addChild(dot);
        }

        f->m_navMenu->updateLayout();
    }

    void setupPage(int page, IconType type) {
        GJGarageLayer::setupPage(page, type);

        auto f = m_fields.self();
        if (f->m_pageBar) {
            f->m_pageBar->removeFromParent();
            f->m_pageBar = nullptr;
        }

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto active = MoreIconsAPI::activeIcon(type, sdi && sdi->getSavedValue("2pselected", false));
        if (MoreIconsAPI::hasIcon(active, type)) {
            if (m_cursor1->isVisible()) m_cursor1->setOpacity(127);
            else m_cursor1->setOpacity(255);
        }
        else m_cursor1->setOpacity(255);
    }

    void onNavigate(CCObject* sender) {
        GJGarageLayer::onNavigate(sender);

        m_fields->m_pages[m_iconType] = sender->getTag();

        createNavMenu();
    }

    void onCustomNavigate(CCObject* sender) {
        setupCustomPage(sender->getTag());
    }

    void wrapPage(int page) {
        auto pages = (GameManager::get()->countForType(m_iconType) + 35) / 36 + (MoreIconsAPI::getCount(m_iconType) + 35) / 36;
        m_fields->m_pages[m_iconType] = pages > 0 ? page < 0 ? pages - 1 : page >= pages ? 0 : page : 0;
    }

    void onArrow(CCObject* sender) {
        GJGarageLayer::onArrow(sender);

        auto typeCount = GameManager::get()->countForType(m_iconType);
        auto maxPage = std::max((typeCount - 1) / 36, 0);
        auto tag = sender->getTag();
        auto& vanillaPage = m_iconPages[m_iconType];
        vanillaPage -= tag;
        vanillaPage = vanillaPage < 0 ? maxPage : vanillaPage > maxPage ? 0 : vanillaPage;

        auto f = m_fields.self();
        wrapPage(f->m_pages[m_iconType] + tag);
        auto page = f->m_pages[m_iconType];

        if (page * 36 < typeCount) {
            setupPage(vanillaPage = tag == -1 && vanillaPage < 0 ? maxPage : tag == 1 && vanillaPage > maxPage ? 0 : page, m_iconType);
            createNavMenu();
        }
        else setupCustomPage(page);
    }

    void selectTab(IconType type) {
        GJGarageLayer::selectTab(type);

        auto f = m_fields.self();
        if (!f->m_initialized) return;

        setupCustomPage(f->m_pages[type] = f->m_pages.contains(type) ? f->m_pages[type] : findIconPage(type));
    }

    std::vector<IconInfo> getPage() {
        auto customPage = m_fields->m_pages[m_iconType] - (GameManager::get()->countForType(m_iconType) + 35) / 36;
        if (MoreIconsAPI::getCount(m_iconType) <= customPage * 36 || customPage < 0 || !MoreIconsAPI::iconIndices.contains(m_iconType)) return {};

        auto& [start, end] = MoreIconsAPI::iconIndices[m_iconType];
        return {
            MoreIconsAPI::icons.begin() + start + customPage * 36,
            MoreIconsAPI::icons.begin() + std::min(end, start + (customPage + 1) * 36)
        };
    }

    void setupCustomPage(int page) {
        if (m_iconType == IconType::Special) return setupCustomSpecialPage(page);

        auto f = m_fields.self();
        if (f->m_pageBar) {
            f->m_pageBar->removeFromParent();
            f->m_pageBar = nullptr;
        }

        auto gameManager = GameManager::get();
        if (MoreIconsAPI::getCount(m_iconType) <= 0 || page * 36 < gameManager->countForType(m_iconType)) return createNavMenu();

        m_cursor1->setOpacity(255);
        m_iconSelection->setVisible(false);

        wrapPage(page);
        createNavMenu();

        auto sfc = CCSpriteFrameCache::get();
        auto offFrame = sfc->spriteFrameByName("gj_navDotBtn_off_001.png");
        for (auto navDot : CCArrayExt<CCMenuItemSpriteExtra*>(m_navDotMenu->getChildren())) {
            static_cast<CCSprite*>(navDot->getNormalImage())->setDisplayFrame(offFrame);
        }

        auto unlockType = gameManager->iconTypeToUnlockType(m_iconType);
        auto playerSquare = sfc->spriteFrameByName("playerSquare_001.png")->getOriginalSize();
        auto objs = CCArray::create();
        CCMenuItemSpriteExtra* current = nullptr;
        int i = 1;
        auto hasAnimProf = Loader::get()->isModLoaded("thesillydoggo.animatedprofiles");
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        auto active = MoreIconsAPI::activeIcon(m_iconType, dual);
        for (auto& info : getPage()) {
            auto itemIcon = GJItemIcon::createBrowserItem(unlockType, 1);
            itemIcon->setScale(GJItemIcon::scaleForType(unlockType));
            auto simplePlayer = static_cast<SimplePlayer*>(itemIcon->m_player);
            MoreIconsAPI::updateSimplePlayer(simplePlayer, info.name, m_iconType);
            if (hasAnimProf) {
                if (auto robotSprite = simplePlayer->m_robotSprite) robotSprite->runAnimation("idle01");
                if (auto spiderSprite = simplePlayer->m_spiderSprite) spiderSprite->runAnimation("idle01");
            }
            auto iconButton = CCMenuItemSpriteExtra::create(itemIcon, this, menu_selector(GJGarageLayer::onSelect));
            iconButton->setUserObject("name"_spr, CCString::create(info.name));
            iconButton->setContentSize(playerSquare);
            itemIcon->setPosition(playerSquare / 2);
            iconButton->setTag(i++);
            iconButton->m_iconType = m_iconType;
            objs->addObject(iconButton);
            if (info.name == active) current = iconButton;
        }

        f->m_pageBar = ListButtonBar::create(objs, CCDirector::get()->getWinSize() / 2 - CCPoint { 0.0f, 65.0f },
            12, 3, 5.0f, 5.0f, 25.0f, 220.0f, 1);
        f->m_pageBar->m_scrollLayer->togglePageIndicators(false);
        f->m_pageBar->setID("icon-selection-bar"_spr);
        addChild(f->m_pageBar, 101);

        m_cursor1->setVisible(current != nullptr);
        if (current) m_cursor1->setPosition(current->getParent()->convertToWorldSpace(current->getPosition()));
    }

    void onCustomSelect(CCNode* sender) {
        auto f = m_fields.self();
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        std::string name = static_cast<CCString*>(sender->getUserObject("name"_spr))->m_sString;

        m_cursor1->setPosition(sender->getParent()->convertToWorldSpace(sender->getPosition()));
        m_cursor1->setVisible(true);
        auto player = dual ? static_cast<SimplePlayer*>(getChildByID("player2-icon")) : m_playerObject;
        player->updateColors();
        MoreIconsAPI::updateSimplePlayer(player, name, m_iconType);
        player->setScale(m_iconType == IconType::Jetpack ? 1.5f : 1.6f);
        auto selectedIconType = dual ? (IconType)sdi->getSavedValue("lasttype", 0) : m_selectedIconType;
        if (f->m_selectedIcon == name && selectedIconType == m_iconType) {
            if (auto info = MoreIconsAPI::getIcon(name, m_iconType)) MoreInfoPopup::create(info)->show();
        }

        if (dual) {
            sdi->setSavedValue("lastmode", (int)m_iconType);
            sdi->setSavedValue("lasttype", (int)m_iconType);
        }
        else {
            GameManager::get()->m_playerIconType = m_iconType;
            m_selectedIconType = m_iconType;
        }

        f->m_selectedIcon = name;
        MoreIconsAPI::setIcon(name, m_iconType, dual);
    }

    void setupCustomSpecialPage(int page) {
        auto f = m_fields.self();
        if (f->m_pageBar) {
            f->m_pageBar->removeFromParent();
            f->m_pageBar = nullptr;
        }

        if (MoreIconsAPI::getCount(m_iconType) <= 0 || page * 36 < GameManager::get()->countForType(m_iconType)) return createNavMenu();

        m_cursor1->setOpacity(255);
        m_iconSelection->setVisible(false);

        wrapPage(page);
        createNavMenu();

        auto offFrame = CCSpriteFrameCache::get()->spriteFrameByName("gj_navDotBtn_off_001.png");
        for (auto navDot : CCArrayExt<CCMenuItemSpriteExtra*>(m_navDotMenu->getChildren())) {
            static_cast<CCSprite*>(navDot->getNormalImage())->setDisplayFrame(offFrame);
        }

        auto objs = CCArray::create();
        CCMenuItemSpriteExtra* current = nullptr;
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        int i = 1;
        auto active = MoreIconsAPI::activeIcon(m_iconType, dual);
        for (auto& info : getPage()) {
            auto square = CCSprite::createWithSpriteFrameName("playerSquare_001.png");
            square->setColor({ 150, 150, 150 });
            auto streak = CCSprite::create(info.textures[0].c_str());
            limitNodeHeight(streak, 27.0f, 999.0f, 0.001f);
            streak->setRotation(-90.0f);
            square->addChild(streak);
            streak->setPosition(square->getContentSize() / 2);
            square->setScale(0.8f);
            auto iconButton = CCMenuItemSpriteExtra::create(square, this, menu_selector(GJGarageLayer::onSelect));
            iconButton->setUserObject("name"_spr, CCString::create(info.name));
            iconButton->setTag(i++);
            iconButton->m_iconType = m_iconType;
            objs->addObject(iconButton);
            if (info.name == active) current = iconButton;
        }

        f->m_pageBar = ListButtonBar::create(objs, CCDirector::get()->getWinSize() / 2 - CCPoint { 0.0f, 65.0f },
            12, 3, 5.0f, 5.0f, 25.0f, 220.0f, 1);
        f->m_pageBar->m_scrollLayer->togglePageIndicators(false);
        f->m_pageBar->setID("icon-selection-bar"_spr);
        addChild(f->m_pageBar, 101);

        m_cursor1->setVisible(current != nullptr);
        if (current) m_cursor1->setPosition(current->getParent()->convertToWorldSpace(current->getPosition()));
        m_cursor2->setVisible(false);
    }

    void onCustomSpecialSelect(CCNode* sender) {
        auto f = m_fields.self();
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        std::string name = static_cast<CCString*>(sender->getUserObject("name"_spr))->m_sString;

        m_cursor1->setPosition(sender->getParent()->convertToWorldSpace(sender->getPosition()));
        m_cursor1->setVisible(true);
        auto selectedIconType = dual ? (IconType)sdi->getSavedValue("lasttype", 0) : m_selectedIconType;
        if (f->m_selectedIcon == name && selectedIconType == m_iconType) {
            if (auto info = MoreIconsAPI::getIcon(name, m_iconType)) MoreInfoPopup::create(info)->show();
        }

        if (dual) sdi->setSavedValue("lasttype", (int)m_iconType);
        else m_selectedIconType = m_iconType;

        f->m_selectedIcon = name;
        MoreIconsAPI::setIcon(name, m_iconType, dual);
    }
};

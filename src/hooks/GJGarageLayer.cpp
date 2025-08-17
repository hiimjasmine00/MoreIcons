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
        ListButtonBar* m_pageBar = nullptr;
        CCMenu* m_navMenu = nullptr;
        std::string m_selectedIcon;
        bool m_initialized = false;
    };

    static void onModify(ModifyBase<ModifyDerive<MIGarageLayer, GJGarageLayer>>& self) {
        (void)self.setHookPriorityAfterPost("GJGarageLayer::init", "weebify.separate_dual_icons");
        (void)self.setHookPriorityBeforePre("GJGarageLayer::onSelect", "weebify.separate_dual_icons");
        (void)self.setHookPriorityBeforePre("GJGarageLayer::setupPage", "weebify.separate_dual_icons");
        (void)self.setHookPriority("GJGarageLayer::onArrow", INT_MAX-2);
    }

    bool init() {
        if (!GJGarageLayer::init()) return false;

        auto f = m_fields.self();
        f->m_initialized = true;

        MoreIcons::updateGarage(this);

        if (MoreIconsAPI::hasIcon(IconType::Cube, false)) setupCustomPage(findIconPage(IconType::Cube), IconType::Cube);
        else createNavMenu(IconType::Cube);

        auto sdi = Loader::get()->isModLoaded("weebify.separate_dual_icons");
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
        }

        if (auto shardsMenu = getChildByID("shards-menu")) {
            auto miSprite = CircleButtonSprite::createWithSprite("MI_moreIcons_001.png"_spr, 1.0f, CircleBaseColor::Gray, CircleBaseSize::Small);
            if (MoreIcons::severity > Severity::Debug) {
                auto severitySprite = CCSprite::createWithSpriteFrameName(MoreIcons::severityFrames[MoreIcons::severity]);
                severitySprite->setPosition(miSprite->getContentSize() - CCPoint { 6.0f, 6.0f });
                severitySprite->setScale(0.6f);
                miSprite->addChild(severitySprite, 1);
            }
            auto miButton = CCMenuItemSpriteExtra::create(miSprite, this, menu_selector(MIGarageLayer::onMoreIcons));
            miButton->setID("more-icons-button"_spr);
            shardsMenu->addChild(miButton);
            shardsMenu->updateLayout();

            if (sdi) ButtonHooker::create(
                static_cast<CCMenuItem*>(shardsMenu->getChildByID("swap-2p-button")),
                this, menu_selector(MIGarageLayer::newSwap2PKit)
            );
        }

        return true;
    }

    void onMoreIcons(CCObject* sender) {
        MoreIconsPopup::create()->show();
    }

    static int separateDualIconsActiveIconForType(IconType type) {
        Mod* separateDualIcons = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        if (!separateDualIcons) return 0;
#define SDI_GET(id) separateDualIcons->getSavedValue<int>(id, 1)
        switch(type) {
            case    IconType::Cube: return SDI_GET("cube");
            case    IconType::Ship: return SDI_GET("ship");
            case    IconType::Ball: return SDI_GET("roll");
            case     IconType::Ufo: return SDI_GET("bird");
            case    IconType::Wave: return SDI_GET("dart");
            case   IconType::Robot: return SDI_GET("robot");
            case  IconType::Spider: return SDI_GET("spider");
            case   IconType::Swing: return SDI_GET("swing");
            case IconType::Jetpack: return SDI_GET("jetpack");
            case IconType::Special: return SDI_GET("trail");
        }
#undef SDI_GET
        return 0;
    }

    static int findIconPage(IconType type) {
        auto gameManager = GameManager::get();
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        bool secondPlayerSelected = sdi && sdi->getSavedValue("2pselected", false);
        auto info = MoreIconsAPI::getIcon(type, secondPlayerSelected);
        return (
            (info && MoreIconsAPI::iconSpans.contains(type)) ? (gameManager->countForType(type) + 35) / 36 + (info - MoreIconsAPI::iconSpans[type].data()) / 36 :
            secondPlayerSelected ? (separateDualIconsActiveIconForType(type) - 1) / 36 :
            (gameManager->activeIconForType(type) - 1) / 36
        );
    }

    void onSelect(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        if (btn->getUserObject("name"_spr)) {
            m_cursor1->setOpacity(255);
            return m_iconType == IconType::Special ? onCustomSpecialSelect(btn) : onCustomSelect(btn);
        }

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        if (MoreIconsAPI::hasIcon(m_iconType, dual)) m_iconID = 0;

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
        if (MoreIconsAPI::hasIcon(m_iconType, sdi && sdi->getSavedValue("2pselected", false))) {
            if (m_cursor1->isVisible()) m_cursor1->setOpacity(127);
            else m_cursor1->setOpacity(255);
        }
        else m_cursor1->setOpacity(255);

        selectTab(m_iconType);
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

        selectTab(m_iconType);
    }

    void updatePlayerColors() {
        GJGarageLayer::updatePlayerColors();

        if (m_iconSelection && m_fields->m_pageBar && MoreIconsAPI::getCount(m_iconType) > 0) m_iconSelection->setVisible(false);
    }

    void createNavMenu(IconType type) {
        if (!m_fields->m_initialized) return;
        auto f = m_fields.self();
        auto winSize = CCDirector::get()->getWinSize();
        if (!f->m_navMenu) {
            f->m_navMenu = CCMenu::create();
            f->m_navMenu->setPosition({ winSize.width / 2.0f, 15.0f });
            f->m_navMenu->setContentSize({ winSize.width - 60.0f, 20.0f });
            f->m_navMenu->setLayout(RowLayout::create()->setGap(6.0f)->setAxisAlignment(AxisAlignment::Center));
            f->m_navMenu->setID("navdot-menu"_spr);
            addChild(f->m_navMenu, 1);
        }

        auto iconCount = MoreIconsAPI::getCount(type);
        m_navDotMenu->setPositionY(iconCount > 0 ? 35.0f : 25.0f);
        auto count = (GameManager::get()->countForType(type) + 35) / 36;
        if (count == 1) {
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
            auto pages = m_iconPages;
            auto dotSprite = CCSprite::createWithSpriteFrameName(
                pages.contains(type) && i == pages[type] ? "gj_navDotBtn_on_001.png" : "gj_navDotBtn_off_001.png");
            dotSprite->setScale(0.9f);
            auto dot = CCMenuItemSpriteExtra::create(dotSprite, this, menu_selector(GJGarageLayer::onNavigate));
            dot->setTag(i);
            dot->setSizeMult(1.1f);
            f->m_navMenu->addChild(dot);
        }

        f->m_navMenu->updateLayout();
    }

    void setupPage(int page, IconType type) {

        int maxVanillaPage = (GameManager::get()->countForType(type) + 35) / 36;

        // put it before the custom page stuff, so that it gets called at least once; useful while the garage isn't initialized yet
        // you could put it behind checks so it doesn't get unnecessarily called if the garage is already initialized, but it didn't
        // seem to break anything if i just kept it like this
        GJGarageLayer::setupPage(page, type);
        createNavMenu(type);

        if (m_fields->m_initialized) {
            if (page >= maxVanillaPage) {
                setupCustomPage(page, type);
                createNavMenu(type);
                return;
            } else if (page == -1) {
                int actualPage = findIconPage(type);
                if (actualPage >= maxVanillaPage) {
                    setupCustomPage(actualPage, type);
                    createNavMenu(type);
                    return;
                }
            }
        }

        auto f = m_fields.self();
        if (f->m_pageBar) {
            f->m_pageBar->removeFromParent();
            f->m_pageBar = nullptr;
        }

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        if (MoreIconsAPI::hasIcon(type, sdi && sdi->getSavedValue("2pselected", false))) {
            if (m_cursor1->isVisible()) m_cursor1->setOpacity(127);
            else m_cursor1->setOpacity(255);
        }
        else m_cursor1->setOpacity(255);
    }

    void onArrow(CCObject* sender) {
        auto maxPage = (GameManager::get()->countForType(m_iconType) - 1) / 36;
        auto tag = sender->getTag();
        int page = m_iconPages[m_iconType] + tag;
        auto pages = (GameManager::get()->countForType(m_iconType) + 35) / 36 + (MoreIconsAPI::getCount(m_iconType) + 35) / 36;
        m_iconPages[m_iconType] = pages > 0 ? page < 0 ? pages - 1 : page >= pages ? 0 : page : 0;
        setupPage(m_iconPages[m_iconType], m_iconType);
    }

    std::span<IconInfo> getPage() {
        auto customPage = m_iconPages[m_iconType] - (GameManager::get()->countForType(m_iconType) + 35) / 36;
        if (customPage < 0 || !MoreIconsAPI::iconSpans.contains(m_iconType)) return {};

        auto index = customPage * 36;
        auto& iconSpan = MoreIconsAPI::iconSpans[m_iconType];
        if (iconSpan.size() <= index) return {};

        return iconSpan.subspan(index, std::min<size_t>(36, iconSpan.size() - index));
    }

    void setupCustomPage(int page, IconType type) {
        if (type == IconType::Special) return setupCustomSpecialPage(page);

        auto f = m_fields.self();
        if (f->m_pageBar) {
            f->m_pageBar->removeFromParent();
            f->m_pageBar = nullptr;
        }

        auto gameManager = GameManager::get();
        if (MoreIconsAPI::getCount(type) <= 0 || page * 36 < gameManager->countForType(type)) return createNavMenu(type);

        m_cursor1->setOpacity(255);
        m_iconSelection->setVisible(false);

        m_iconType = type;
        m_iconPages[type] = page;
        createNavMenu(type);

        auto sfc = CCSpriteFrameCache::get();
        auto offFrame = sfc->spriteFrameByName("gj_navDotBtn_off_001.png");
        for (auto navDot : CCArrayExt<CCMenuItemSpriteExtra*>(m_navDotMenu->getChildren())) {
            static_cast<CCSprite*>(navDot->getNormalImage())->setDisplayFrame(offFrame);
        }

        auto unlockType = gameManager->iconTypeToUnlockType(type);
        auto playerSquare = sfc->spriteFrameByName("playerSquare_001.png")->getOriginalSize();
        auto objs = CCArray::create();
        CCMenuItemSpriteExtra* current = nullptr;
        int i = 1;
        auto hasAnimProf = Loader::get()->isModLoaded("thesillydoggo.animatedprofiles");
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        auto active = MoreIconsAPI::activeIcon(type, dual);
        for (auto& info : getPage()) {
            auto itemIcon = GJItemIcon::createBrowserItem(unlockType, 1);
            itemIcon->setScale(GJItemIcon::scaleForType(unlockType));
            auto simplePlayer = static_cast<SimplePlayer*>(itemIcon->m_player);
            MoreIconsAPI::updateSimplePlayer(simplePlayer, info.name, type);
            if (hasAnimProf) {
                if (auto robotSprite = simplePlayer->m_robotSprite) robotSprite->runAnimation("idle01");
                if (auto spiderSprite = simplePlayer->m_spiderSprite) spiderSprite->runAnimation("idle01");
            }
            auto iconButton = CCMenuItemSpriteExtra::create(itemIcon, this, menu_selector(GJGarageLayer::onSelect));
            iconButton->setUserObject("name"_spr, CCString::create(info.name));
            iconButton->setContentSize(playerSquare);
            itemIcon->setPosition(playerSquare / 2.0f);
            iconButton->setTag(i++);
            iconButton->m_iconType = type;
            objs->addObject(iconButton);
            if (info.name == active) current = iconButton;
        }

        f->m_pageBar = ListButtonBar::create(objs, CCDirector::get()->getWinSize() / 2.0f - CCPoint { 0.0f, 65.0f },
            12, 3, 5.0f, 5.0f, 25.0f, 220.0f, 1);
        f->m_pageBar->m_scrollLayer->togglePageIndicators(false);
        f->m_pageBar->setID("icon-selection-bar"_spr);
        addChild(f->m_pageBar, 101);

        m_cursor1->setVisible(current != nullptr);
        log::info("{}", current);
        if (current) m_cursor1->setPosition(current->getParent()->convertToWorldSpace(current->getPosition()));
    }

    void onCustomSelect(CCNode* sender) {
        auto f = m_fields.self();
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        auto name = MoreIconsAPI::getIconName(sender);

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

        if (MoreIconsAPI::getCount(IconType::Special) <= 0 || page * 36 < GameManager::get()->countForType(IconType::Special)) return createNavMenu(IconType::Special);


        m_cursor1->setOpacity(255);
        m_iconSelection->setVisible(false);

        m_iconType = IconType::Special;
        m_iconPages[IconType::Special] = page;
        createNavMenu(IconType::Special);

        auto offFrame = CCSpriteFrameCache::get()->spriteFrameByName("gj_navDotBtn_off_001.png");
        for (auto navDot : CCArrayExt<CCMenuItemSpriteExtra*>(m_navDotMenu->getChildren())) {
            static_cast<CCSprite*>(navDot->getNormalImage())->setDisplayFrame(offFrame);
        }

        auto objs = CCArray::create();
        CCMenuItemSpriteExtra* current = nullptr;
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        int i = 1;
        auto active = MoreIconsAPI::activeIcon(IconType::Special, dual);
        for (auto& info : getPage()) {
            auto square = MoreIconsAPI::customTrail(info.textures[0]);
            square->setScale(0.8f);
            auto iconButton = CCMenuItemSpriteExtra::create(square, this, menu_selector(GJGarageLayer::onSelect));
            iconButton->setUserObject("name"_spr, CCString::create(info.name));
            iconButton->setTag(i++);
            iconButton->m_iconType = IconType::Special;
            objs->addObject(iconButton);
            if (info.name == active) current = iconButton;
        }

        f->m_pageBar = ListButtonBar::create(objs, CCDirector::get()->getWinSize() / 2.0f - CCPoint { 0.0f, 65.0f },
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
        auto name = MoreIconsAPI::getIconName(sender);

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

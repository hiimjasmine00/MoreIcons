#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include "../classes/popup/MoreIconsPopup.hpp"
#include "../classes/popup/info/MoreInfoPopup.hpp"
#include <Geode/binding/BoomScrollLayer.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/ListButtonBar.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <Geode/modify/GJGarageLayer.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <jasmine/button.hpp>

using namespace geode::prelude;
using namespace jasmine::button;

$execute {
    new EventListener(+[](FLAlertLayer** layer, const std::string& name, IconType type) {
        if (auto info = MoreIconsAPI::getIcon(name, type)) *layer = MoreInfoPopup::create(info);
        return ListenerResult::Propagate;
    }, DispatchFilter<FLAlertLayer**, std::string, IconType>("info-popup"_spr));
}

class $modify(MIGarageLayer, GJGarageLayer) {
    struct Fields {
        ListButtonBar* m_pageBar = nullptr;
        CCMenu* m_navMenu = nullptr;
        std::string m_selectedIcon;
        bool m_initialized = false;
    };

    static void onModify(ModifyBase<ModifyDerive<MIGarageLayer, GJGarageLayer>>& self) {
        (void)self.setHookPriorityAfterPost("GJGarageLayer::init", "weebify.separate_dual_icons");
        (void)self.setHookPriority("GJGarageLayer::onArrow", Priority::Replace);
        (void)self.setHookPriorityBeforePre("GJGarageLayer::onSelect", "weebify.separate_dual_icons");
        (void)self.setHookPriorityAfterPost("GJGarageLayer::setupPage", "weebify.separate_dual_icons");
    }

    bool init() {
        if (!GJGarageLayer::init()) return false;

        m_fields->m_initialized = true;

        MoreIcons::updateGarage(this);

        if (MoreIconsAPI::hasIcon(IconType::Cube, false)) setupCustomPage(findIconPage(IconType::Cube, false), IconType::Cube);
        else createNavMenu(m_iconPages[IconType::Cube], IconType::Cube);

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

    int findIconPage(IconType type, bool dual) {
        auto info = MoreIconsAPI::getIcon(type, dual);
        return info
            ? (MoreIconsAPI::getGameManager()->countForType(type) + 35) / 36 + (info - MoreIconsAPI::icons[type].data()) / 36
            : m_iconPages[type];
    }

    void onSelect(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        if (btn->getUserObject("name"_spr)) return onCustomSelect(btn);

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        if (MoreIconsAPI::hasIcon(m_iconType, dual)) m_iconID = 0;

        GJGarageLayer::onSelect(sender);

        if (!MoreIconsAPI::getGameManager()->isIconUnlocked(sender->getTag(), btn->m_iconType)) return;

        if (btn->m_iconType != IconType::ShipFire) {
            m_cursor1->setOpacity(255);
            m_fields->m_selectedIcon = "";
            MoreIconsAPI::setIcon("", dual ? (IconType)sdi->getSavedValue("lasttype", 0) : m_selectedIconType, dual);
        }
    }

    void newOn2PToggle(CCObject* sender) {
        ButtonHooker::call(sender);

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        m_cursor1->setOpacity(255 - (
            MoreIconsAPI::hasIcon(m_iconType, sdi && sdi->getSavedValue("2pselected", false)) && m_cursor1->isVisible()) * 128);

        selectTab(m_iconType);
    }

    void newSwap2PKit(CCObject* sender) {
        ButtonHooker::call(sender);

        for (int i = 0; i < 11; i++) {
            if (i == 9) continue;
            auto type = MoreIconsAPI::convertType(i);
            MoreIconsAPI::setIcon(MoreIconsAPI::setIcon(MoreIconsAPI::activeIcon(type, true), type, false), type, true);
        }

        MoreIconsAPI::updateSimplePlayer(m_playerObject, MoreIconsAPI::getGameManager()->m_playerIconType, false);
        MoreIconsAPI::updateSimplePlayer(static_cast<SimplePlayer*>(getChildByID("player2-icon")),
            (IconType)Loader::get()->getLoadedMod("weebify.separate_dual_icons")->getSavedValue("lastmode", 0), true);
        selectTab(m_iconType);
    }

    void updatePlayerColors() {
        GJGarageLayer::updatePlayerColors();

        if (m_iconSelection && m_fields->m_pageBar && MoreIconsAPI::icons[m_iconType].size() > 0) m_iconSelection->setVisible(false);
    }

    void createNavMenu(int page, IconType type) {
        auto f = m_fields.self();
        if (!f->m_navMenu) {
            auto winSize = MoreIconsAPI::getDirector()->getWinSize();
            f->m_navMenu = CCMenu::create();
            f->m_navMenu->setPosition({ winSize.width / 2.0f, 15.0f });
            f->m_navMenu->setContentSize({ winSize.width - 60.0f, 20.0f });
            f->m_navMenu->setLayout(RowLayout::create()->setGap(6.0f)->setAxisAlignment(AxisAlignment::Center));
            f->m_navMenu->setID("navdot-menu"_spr);
            addChild(f->m_navMenu, 1);
        }

        auto iconCount = MoreIconsAPI::icons[type].size();
        m_navDotMenu->setPositionY(iconCount > 0 ? 35.0f : 25.0f);
        auto count = (MoreIconsAPI::getGameManager()->countForType(type) + 35) / 36;
        if (count < 2) {
            m_navDotMenu->setVisible(true);
            m_navDotMenu->setEnabled(true);
            m_navDotMenu->removeAllChildren();
            auto firstDot = static_cast<CCMenuItemSprite*>(m_pageButtons->objectAtIndex(0));
            static_cast<CCSprite*>(firstDot->getNormalImage())->setDisplayFrame(
                MoreIconsAPI::getSpriteFrameCache()->spriteFrameByName("gj_navDotBtn_on_001.png"));
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
        auto navDotAmount = count + (iconCount + 35) / 36;
        for (int i = count; i < navDotAmount; i++) {
            auto dotSprite = CCSprite::createWithSpriteFrameName(i == page ? "gj_navDotBtn_on_001.png" : "gj_navDotBtn_off_001.png");
            dotSprite->setScale(0.9f);
            auto dot = CCMenuItemSpriteExtra::create(dotSprite, this, menu_selector(GJGarageLayer::onNavigate));
            dot->setTag(i);
            dot->setSizeMult(1.1f);
            f->m_navMenu->addChild(dot);
        }

        f->m_navMenu->updateLayout();
    }

    void setupPage(int page, IconType type) {
        auto f = m_fields.self();
        if (f->m_pageBar) {
            f->m_pageBar->removeFromParent();
            f->m_pageBar = nullptr;
        }

        GJGarageLayer::setupPage(page, type);

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        m_cursor1->setOpacity(255 - (MoreIconsAPI::hasIcon(type, dual) && m_cursor1->isVisible()) * 128);

        if (f->m_initialized) setupCustomPage(page == -1 ? findIconPage(type, dual) : page, type);
    }

    void onArrow(CCObject* sender) {
        auto page = m_iconPages[m_iconType] + sender->getTag();
        auto pages = (MoreIconsAPI::getGameManager()->countForType(m_iconType) + 35) / 36 + (MoreIconsAPI::icons[m_iconType].size() + 35) / 36;
        GJGarageLayer::setupPage(pages > 0 ? page < 0 ? pages + page : page >= pages ? page - pages : page : 0, m_iconType);
    }

    void setupCustomPage(int page, IconType type) {
        m_iconPages[type] = page;
        createNavMenu(page, type);

        auto found = MoreIconsAPI::icons.find(type);
        if (found == MoreIconsAPI::icons.end()) return;

        auto gameManager = MoreIconsAPI::getGameManager();
        auto customPage = page - (gameManager->countForType(type) + 35) / 36;
        if (customPage < 0) return;

        auto& icons = found->second;
        auto index = customPage * 36;
        auto size = icons.size();
        if (size < index) return;

        m_cursor1->setOpacity(255);
        m_iconSelection->setVisible(false);

        auto sfc = MoreIconsAPI::getSpriteFrameCache();
        auto offFrame = sfc->spriteFrameByName("gj_navDotBtn_off_001.png");
        for (auto navDot : CCArrayExt<CCMenuItemSpriteExtra*>(m_navDotMenu->getChildren())) {
            static_cast<CCSprite*>(navDot->getNormalImage())->setDisplayFrame(offFrame);
        }

        auto objs = CCArray::create();
        CCMenuItemSpriteExtra* current = nullptr;
        auto i = 1;
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto active = MoreIconsAPI::activeIcon(type, sdi && sdi->getSavedValue("2pselected", false));
        std::span infoPage(icons.data() + index, std::min<size_t>(36, size - index));

        if (type <= IconType::Jetpack) {
            auto unlockType = gameManager->iconTypeToUnlockType(type);
            auto playerSquare = sfc->spriteFrameByName("playerSquare_001.png")->getOriginalSize();
            auto hasAnimProf = Loader::get()->isModLoaded("thesillydoggo.animatedprofiles");
            for (auto& info : infoPage) {
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
        }
        else if (type == IconType::Special) {
            for (auto& info : infoPage) {
                auto square = MoreIconsAPI::customTrail(info.textures[0]);
                square->setScale(0.8f);
                auto iconButton = CCMenuItemSpriteExtra::create(square, this, menu_selector(GJGarageLayer::onSelect));
                iconButton->setUserObject("name"_spr, CCString::create(info.name));
                iconButton->setTag(i++);
                iconButton->m_iconType = type;
                objs->addObject(iconButton);
                if (info.name == active) current = iconButton;
            }
        }

        auto f = m_fields.self();
        if (f->m_pageBar) {
            f->m_pageBar->removeFromParent();
            f->m_pageBar = nullptr;
        }

        f->m_pageBar = ListButtonBar::create(objs, MoreIconsAPI::getDirector()->getWinSize() / 2.0f - CCPoint { 0.0f, 65.0f },
            12, 3, 5.0f, 5.0f, 25.0f, 220.0f, 1);
        f->m_pageBar->m_scrollLayer->togglePageIndicators(false);
        f->m_pageBar->setID("icon-selection-bar"_spr);
        addChild(f->m_pageBar, 101);

        m_cursor1->setVisible(current != nullptr);
        if (current) m_cursor1->setPosition(current->getParent()->convertToWorldSpace(current->getPosition()));
        if (type == IconType::Special) m_cursor2->setVisible(false);
    }

    void onCustomSelect(CCNode* sender) {
        auto& selectedIcon = m_fields->m_selectedIcon;
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        auto name = MoreIconsAPI::getIconName(sender);
        auto isIcon = m_iconType <= IconType::Jetpack;

        m_cursor1->setPosition(sender->getParent()->convertToWorldSpace(sender->getPosition()));
        m_cursor1->setVisible(true);
        m_cursor1->setOpacity(255);
        if (isIcon) {
            auto player = dual ? static_cast<SimplePlayer*>(getChildByID("player2-icon")) : m_playerObject;
            player->updateColors();
            MoreIconsAPI::updateSimplePlayer(player, name, m_iconType);
            player->setScale(m_iconType == IconType::Jetpack ? 1.5f : 1.6f);
        }

        if (name == selectedIcon && m_iconType == (dual ? (IconType)sdi->getSavedValue("lasttype", 0) : m_selectedIconType)) {
            if (auto info = MoreIconsAPI::getIcon(name, m_iconType)) MoreInfoPopup::create(info)->show();
        }

        if (dual) {
            if (isIcon) sdi->setSavedValue("lastmode", (int)m_iconType);
            sdi->setSavedValue("lasttype", (int)m_iconType);
        }
        else {
            if (isIcon) MoreIconsAPI::getGameManager()->m_playerIconType = m_iconType;
            m_selectedIconType = m_iconType;
        }

        selectedIcon = name;
        MoreIconsAPI::setIcon(name, m_iconType, dual);
    }
};

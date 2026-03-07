#include "../MoreIcons.hpp"
#include "../classes/popup/MoreIconsPopup.hpp"
#include "../utils/Constants.hpp"
#include "../utils/Get.hpp"
#include "../utils/Icons.hpp"
#include "../utils/Log.hpp"
#include <Geode/binding/BoomScrollLayer.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJItemIcon.hpp>
#include <Geode/binding/GJSpiderSprite.hpp>
#include <Geode/binding/ListButtonBar.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/GJGarageLayer.hpp>
#include <jasmine/button.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;
using namespace jasmine::button;

class $modify(MIGarageLayer, GJGarageLayer) {
    struct Fields {
        CCSprite* m_cursor3 = nullptr;
        CCSprite* m_cursor4 = nullptr;
        SimplePlayer* m_playerObject2 = nullptr;
        ListButtonBar* m_pageBar = nullptr;
        CCMenu* m_navMenu = nullptr;
        IconInfo* m_selectedIcon = nullptr;
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

        auto f = m_fields.self();
        if (MoreIcons::separateDualIcons) {
            m_cursor1->setCascadeOpacityEnabled(true);
            m_cursor2->setCascadeOpacityEnabled(true);
            f->m_cursor3 = static_cast<CCSprite*>(getChildByID("cursor-3"));
            f->m_cursor3->setCascadeOpacityEnabled(true);
            f->m_cursor4 = static_cast<CCSprite*>(getChildByID("cursor-4"));
            f->m_cursor4->setCascadeOpacityEnabled(true);
            f->m_playerObject2 = static_cast<SimplePlayer*>(getChildByID("player2-icon"));
        }

        m_fields->m_initialized = true;

        MoreIcons::updateGarage(this);

        if (f->m_cursor3) {
            f->m_cursor3->setOpacity(more_icons::hasIcon(IconType::Cube, true) && f->m_cursor3->isVisible() ? 127 : 255);
        }

        if (more_icons::hasIcon(IconType::Cube, false)) setupCustomPage(findIconPage(IconType::Cube, false), IconType::Cube);
        else createNavMenu(m_iconPages[IconType::Cube], IconType::Cube);

        if (MoreIcons::separateDualIcons) {
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
            if (!Log::logs.empty()) {
                Severity severity = Severity::Debug;
                for (auto& [type, logs] : Log::logs) {
                    if (logs[0].severity > severity) severity = logs[0].severity;
                }
                auto severitySprite = CCSprite::createWithSpriteFrameName(Constants::getSeverityFrame(severity));
                severitySprite->setPosition(miSprite->getContentSize() - CCPoint { 6.0f, 6.0f });
                severitySprite->setScale(0.6f);
                miSprite->addChild(severitySprite, 1);
            }
            auto miButton = CCMenuItemSpriteExtra::create(miSprite, this, menu_selector(MIGarageLayer::onMoreIcons));
            miButton->setID("more-icons-button"_spr);
            shardsMenu->addChild(miButton);
            shardsMenu->updateLayout();

            if (MoreIcons::separateDualIcons) {
                ButtonHooker::create(
                    static_cast<CCMenuItem*>(shardsMenu->getChildByID("swap-2p-button")),
                    this, menu_selector(MIGarageLayer::newSwap2PKit)
                );
            }
        }

        return true;
    }

    void onMoreIcons(CCObject* sender) {
        MoreIconsPopup::create()->show();
    }

    int findIconPage(IconType type, bool dual) {
        auto icons = more_icons::getIcons(type);
        if (!icons) return m_iconPages[type];

        auto info = more_icons::activeIcon(type, dual);
        return info ? (Get::GameManager()->countForType(type) + 35) / 36 + (info - icons->data()) / 36 : m_iconPages[type];
    }

    CCSprite* getCursor(bool alt, bool dual) {
        return dual ? (alt ? m_fields->m_cursor4 : m_fields->m_cursor3) : (alt ? m_cursor2 : m_cursor1);
    }

    IconType getSelectedType(bool dual) {
        return dual ? (IconType)MoreIcons::separateDualIcons->getSavedValue("lasttype", 0) : m_selectedIconType;
    }

    void onSelect(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        if (btn->getUserObject("info"_spr)) return onCustomSelect(btn);

        auto dual = MoreIcons::dualSelected();
        if (more_icons::hasIcon(m_iconType, dual)) m_iconID = 0;

        GJGarageLayer::onSelect(sender);

        if (!Get::GameManager()->isIconUnlocked(sender->getTag(), btn->m_iconType)) return;

        getCursor(btn->m_iconType == IconType::ShipFire, dual)->setOpacity(255);

        m_fields->m_selectedIcon = nullptr;
        more_icons::setIcon(nullptr, getSelectedType(dual), dual);
    }

    void newOn2PToggle(CCObject* sender) {
        ButtonHooker::call(sender);

        selectTab(m_iconType);
    }

    static void swapDual(IconType type) {
        more_icons::setIcon(more_icons::setIcon(more_icons::activeIcon(type, true), type, false), type, true);
    }

    void newSwap2PKit(CCObject* sender) {
        ButtonHooker::call(sender);

        swapDual(IconType::Cube);
        swapDual(IconType::Ship);
        swapDual(IconType::Ball);
        swapDual(IconType::Ufo);
        swapDual(IconType::Wave);
        swapDual(IconType::Robot);
        swapDual(IconType::Spider);
        swapDual(IconType::Swing);
        swapDual(IconType::Jetpack);
        swapDual(IconType::DeathEffect);
        swapDual(IconType::Special);
        swapDual(IconType::ShipFire);

        more_icons::updateSimplePlayer(m_playerObject, Get::GameManager()->m_playerIconType, false);
        more_icons::updateSimplePlayer(m_fields->m_playerObject2, (IconType)MoreIcons::separateDualIcons->getSavedValue("lastmode", 0), true);
        selectTab(m_iconType);
    }

    void updatePlayerColors() {
        GJGarageLayer::updatePlayerColors();

        if (m_iconSelection && m_fields->m_pageBar && more_icons::getIconCount(m_iconType) > 0) m_iconSelection->setVisible(false);
    }

    void createNavMenu(int page, IconType type) {
        auto f = m_fields.self();
        if (!f->m_navMenu) {
            auto winSize = Get::Director()->getWinSize();
            f->m_navMenu = CCMenu::create();
            f->m_navMenu->setPosition({ winSize.width / 2.0f, 15.0f });
            f->m_navMenu->setContentSize({ winSize.width - 60.0f, 20.0f });
            f->m_navMenu->ignoreAnchorPointForPosition(false);
            f->m_navMenu->setLayout(RowLayout::create()->setGap(6.0f)->setAxisAlignment(AxisAlignment::Center), false);
            f->m_navMenu->setID("navdot-menu"_spr);
            addChild(f->m_navMenu, 1);
        }

        auto iconCount = more_icons::getIconCount(type);
        m_navDotMenu->setPositionY(iconCount > 0 ? 35.0f : 25.0f);
        auto count = (Get::GameManager()->countForType(type) + 35) / 36;
        if (count < 2) {
            m_navDotMenu->setVisible(true);
            m_navDotMenu->setEnabled(true);
            m_navDotMenu->removeAllChildren();
            auto firstDot = static_cast<CCMenuItemSprite*>(m_pageButtons->objectAtIndex(0));
            static_cast<CCSprite*>(firstDot->getNormalImage())->setDisplayFrame(
                Get::SpriteFrameCache()->spriteFrameByName("gj_navDotBtn_on_001.png"));
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

        m_cursor1->setOpacity(more_icons::hasIcon(type, false) && m_cursor1->isVisible() ? 127 : 255);
        if (type == IconType::Special) {
            m_cursor2->setOpacity(more_icons::hasIcon(IconType::ShipFire, false) && m_cursor2->isVisible() ? 127 : 255);
        }

        /*if (f->m_initialized) {
            f->m_cursor3->setOpacity(more_icons::hasIcon(type, true) && f->m_cursor3->isVisible() ? 127 : 255);
            if (type == IconType::Special) {
                f->m_cursor4->setOpacity(more_icons::hasIcon(IconType::ShipFire, true) && f->m_cursor4->isVisible() ? 127 : 255);
            }
            setupCustomPage(page == -1 ? findIconPage(type, MoreIcons::dualSelected()) : page, type);
        }*/

        if (f->m_initialized && f->m_cursor3 && f->m_cursor4) {
            f->m_cursor3->setOpacity(more_icons::hasIcon(type, true) && f->m_cursor3->isVisible() ? 127 : 255);
            if (type == IconType::Special) {
                f->m_cursor4->setOpacity(more_icons::hasIcon(IconType::ShipFire, true) && f->m_cursor4->isVisible() ? 127 : 255);
            }
        }

        setupCustomPage(page == -1 ? findIconPage(type, MoreIcons::dualSelected()) : page, type);
    }

    void onArrow(CCObject* sender) {
        auto page = m_iconPages[m_iconType] + sender->getTag();
        auto pages = (Get::GameManager()->countForType(m_iconType) + 35) / 36 + (more_icons::getIconCount(m_iconType) + 35) / 36;
        GJGarageLayer::setupPage(pages > 0 ? page < 0 ? pages + page : page >= pages ? page - pages : page : 0, m_iconType);
    }

    void setupCustomPage(int page, IconType type) {
        m_iconPages[type] = page;
        createNavMenu(page, type);

        auto icons = more_icons::getIcons(type);
        if (!icons) return;

        auto customPage = page - (Get::GameManager()->countForType(type) + 35) / 36;
        if (customPage < 0) return;

        std::vector<IconInfo*> infoView;
        infoView.reserve(icons->size());
        for (auto& info : *icons) {
            infoView.push_back(&info);
        }
        if (type == IconType::Special) {
            if (auto shipFires = more_icons::getIcons(IconType::ShipFire)) {
                infoView.reserve(infoView.size() + shipFires->size());
                for (auto& info : *shipFires) {
                    infoView.push_back(&info);
                }
            }
        }

        auto index = customPage * 36;
        auto size = infoView.size();
        if (size < index) return;

        auto f = m_fields.self();

        m_cursor1->setOpacity(255);
        if (f->m_cursor3) {
            f->m_cursor3->setOpacity(255);
        }
        if (type == IconType::Special) {
            m_cursor2->setOpacity(255);
            if (f->m_cursor4) {
                f->m_cursor4->setOpacity(255);
            }
        }
        m_iconSelection->setVisible(false);

        if (auto navDots = m_navDotMenu->getChildren()) {
            auto offFrame = Get::SpriteFrameCache()->spriteFrameByName("gj_navDotBtn_off_001.png");
            for (auto navDot : CCArrayExt<CCMenuItemSprite, false>(navDots)) {
                static_cast<CCSprite*>(navDot->getNormalImage())->setDisplayFrame(offFrame);
            }
        }

        auto objs = CCArray::create();
        CCMenuItemSpriteExtra* current = nullptr;
        CCMenuItemSpriteExtra* current2 = nullptr;
        CCMenuItemSpriteExtra* current3 = nullptr;
        CCMenuItemSpriteExtra* current4 = nullptr;
        auto active = more_icons::activeIcon(type, false);
        auto active2 = type == IconType::Special ? more_icons::activeIcon(IconType::ShipFire, false) : nullptr;
        auto active3 = more_icons::activeIcon(type, true);
        auto active4 = type == IconType::Special ? more_icons::activeIcon(IconType::ShipFire, true) : nullptr;

        std::span<IconInfo*> infoPage(infoView.data() + index, std::min<size_t>(36, size - index));

        if (type <= IconType::Jetpack) {
            auto unlockType = Constants::getUnlockType(type);
            auto hasAnimProf = Loader::get()->isModLoaded("thesillydoggo.animatedprofiles");
            for (size_t i = 0; i < infoPage.size(); i++) {
                auto info = infoPage[i];
                auto itemIcon = GJItemIcon::createBrowserItem(unlockType, 1);
                itemIcon->setScale(GJItemIcon::scaleForType(unlockType));
                auto simplePlayer = static_cast<SimplePlayer*>(itemIcon->m_player);
                more_icons::updateSimplePlayer(simplePlayer, info);
                if (hasAnimProf) {
                    if (auto robotSprite = simplePlayer->m_robotSprite) robotSprite->runAnimation("idle01");
                    if (auto spiderSprite = simplePlayer->m_spiderSprite) spiderSprite->runAnimation("idle01");
                }
                auto iconButton = CCMenuItemSpriteExtra::create(itemIcon, this, menu_selector(GJGarageLayer::onSelect));
                Icons::setIcon(iconButton, info);
                iconButton->setContentSize({ 30.0f, 30.0f });
                itemIcon->setPosition({ 15.0f, 15.0f });
                iconButton->setTag(i + 1);
                iconButton->m_iconType = type;
                objs->addObject(iconButton);
                if (info == active) current = iconButton;
                if (info == active3) current3 = iconButton;
            }
        }
        else if (type >= IconType::DeathEffect) {
            for (size_t i = 0; i < infoPage.size(); i++) {
                auto info = infoPage[i];
                auto infoType = info->getType();
                auto sprite = MoreIcons::customIcon(info);
                sprite->setScale(0.8f);
                auto iconButton = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(GJGarageLayer::onSelect));
                Icons::setIcon(iconButton, info);
                iconButton->setTag(i + 1);
                iconButton->m_iconType = infoType;
                objs->addObject(iconButton);
                if (info == active) current = iconButton;
                if (info == active3) current3 = iconButton;
                if (infoType == IconType::ShipFire) {
                    if (info == active2) current2 = iconButton;
                    if (info == active4) current4 = iconButton;
                }
            }
        }

        if (f->m_pageBar) {
            f->m_pageBar->removeFromParent();
            f->m_pageBar = nullptr;
        }

        f->m_pageBar = ListButtonBar::create(objs, Get::Director()->getWinSize() / 2.0f - CCPoint { 0.0f, 65.0f },
            12, 3, 5.0f, 5.0f, 25.0f, 220.0f, 1);
        f->m_pageBar->m_scrollLayer->togglePageIndicators(false);
        f->m_pageBar->setID("icon-selection-bar"_spr);
        addChild(f->m_pageBar, 100);

        m_cursor1->setVisible(current != nullptr);
        if (current) m_cursor1->setPosition(current->getParent()->convertToWorldSpace(current->getPosition()));
        if (f->m_cursor3) {
            f->m_cursor3->setVisible(current3 != nullptr);
            if (current3) f->m_cursor3->setPosition(current3->getParent()->convertToWorldSpace(current3->getPosition()));
        }

        if (type == IconType::Special) {
            m_cursor2->setVisible(current2 != nullptr);
            if (current2) m_cursor2->setPosition(current2->getParent()->convertToWorldSpace(current2->getPosition()));
            if (f->m_cursor4) {
                f->m_cursor4->setVisible(current4 != nullptr);
                if (current4) f->m_cursor4->setPosition(current4->getParent()->convertToWorldSpace(current4->getPosition()));
            }
        }
    }

    void onCustomSelect(CCMenuItemSpriteExtra* sender) {
        auto f = m_fields.self();
        auto dual = MoreIcons::dualSelected();
        auto info = more_icons::getNodeInfo(sender);
        auto type = sender->m_iconType;
        auto isIcon = type <= IconType::Jetpack;

        if (type == IconType::ShipFire) {
            auto cursor2 = getCursor(true, dual);
            cursor2->setPosition(sender->getParent()->convertToWorldSpace(sender->getPosition()));
            cursor2->setVisible(true);
            cursor2->setOpacity(255);
        }
        else {
            auto cursor1 = getCursor(false, dual);
            cursor1->setPosition(sender->getParent()->convertToWorldSpace(sender->getPosition()));
            cursor1->setVisible(true);
            cursor1->setOpacity(255);
        }
        if (isIcon) {
            auto player = dual ? f->m_playerObject2 : m_playerObject;
            player->updateColors();
            more_icons::updateSimplePlayer(player, info);
            player->setScale(type == IconType::Jetpack ? 1.5f : 1.6f);
        }

        if (info == f->m_selectedIcon) {
            if (type == getSelectedType(dual)) {
                if (auto popup = more_icons::createInfoPopup(info)) popup->show();
            }
        }

        if (dual) {
            if (isIcon) MoreIcons::separateDualIcons->setSavedValue("lastmode", (int)type);
            MoreIcons::separateDualIcons->setSavedValue("lasttype", (int)type);
        }
        else {
            if (isIcon) Get::GameManager()->m_playerIconType = type;
            m_selectedIconType = type;
        }

        f->m_selectedIcon = info;
        more_icons::setIcon(info, type, dual);
    }
};

#include "../classes/ButtonHooker.hpp"
#include "../classes/LogLayer.hpp"

using namespace geode::prelude;

#include <Geode/modify/GJGarageLayer.hpp>
class $modify(MIGarageLayer, GJGarageLayer) {
    struct Fields {
        ListButtonBar* m_pageBar;
        CCMenu* m_navMenu;
        std::map<IconType, int> m_pages;
        bool m_initialized;
    };

    static void onModify(auto& self) {
        (void)self.setHookPriority("GJGarageLayer::init", -1);
        (void)self.setHookPriority("GJGarageLayer::setupPage", 1);
    }

    bool init() {
        if (!GJGarageLayer::init()) return false;

        auto f = m_fields.self();
        f->m_initialized = true;
        f->m_pages[IconType::Cube] = m_iconPages[IconType::Cube];

        auto iconType = GameManager::get()->m_playerIconType;
        MoreIconsAPI::updateSimplePlayer(m_playerObject, MoreIconsAPI::activeForType(iconType, false), iconType);
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        if (sdi) {
            auto lastmode = (IconType)sdi->getSavedValue("lastmode", 0);
            MoreIconsAPI::updateSimplePlayer(static_cast<SimplePlayer*>(getChildByID("player2-icon")), MoreIconsAPI::activeForType(lastmode, true), lastmode);
        }

        auto customIcon = MoreIconsAPI::activeForType(IconType::Cube, false);
        if (!customIcon.empty() && MoreIconsAPI::hasIcon(customIcon, IconType::Cube)) setupCustomPage(findIconPage(IconType::Cube));
        else createNavMenu();

        auto shardsMenu = getChildByID("shards-menu");
        if (sdi) {
            if (auto playerButtonsMenu = getChildByID("player-buttons-menu")) {
                auto p1Button = static_cast<CCMenuItemSpriteExtra*>(playerButtonsMenu->getChildByID("player1-button"));
                auto p2Button = static_cast<CCMenuItemSpriteExtra*>(playerButtonsMenu->getChildByID("player2-button"));
                if (p1Button && p2Button) {
                    ButtonHooker::create(p1Button, this, menu_selector(MIGarageLayer::newOn2PToggle));
                    ButtonHooker::create(p2Button, this, menu_selector(MIGarageLayer::newOn2PToggle));
                }
            }

            if (shardsMenu) {
                if (auto swap2PButton = static_cast<CCMenuItemSpriteExtra*>(shardsMenu->getChildByID("swap-2p-button")))
                    ButtonHooker::create(swap2PButton, this, menu_selector(MIGarageLayer::newSwap2PKit));
            }
        }

        auto moreIconsSprite = CircleButtonSprite::createWithSprite("MI_moreIcons_001.png"_spr, 1.0f, CircleBaseColor::Gray, CircleBaseSize::Small);
        if (!MoreIcons::LOGS.empty()) {
            auto severityFrame = "";
            switch (MoreIcons::HIGHEST_SEVERITY) {
                case LogType::Info: severityFrame = "GJ_infoIcon_001.png"; break;
                case LogType::Warn: severityFrame = "geode.loader/info-warning.png"; break;
                case LogType::Error: severityFrame = "geode.loader/info-alert.png"; break;
            }
            auto severitySprite = CCSprite::createWithSpriteFrameName(severityFrame);
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
        if (!MoreIcons::LOGS.empty()) LogLayer::create()->show();
        else MoreIcons::showInfoPopup(true);
    }

    int findIconPage(IconType type) {
        auto gameManager = GameManager::get();
        auto active = (gameManager->activeIconForType(type) - 1) / 36;
        auto& vec = MoreIconsAPI::vectorForType(type);
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto it = std::find(vec.begin(), vec.end(), MoreIconsAPI::activeForType(type, sdi && sdi->getSavedValue("2pselected", false)));
        return it == vec.end() ? active : ((gameManager->countForType(type) + 35) / 36) + (it - vec.begin()) / 36;
    }

    void onSelect(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        if (btn->getUserObject("name"_spr)) {
            if (m_iconType == IconType::Special) onCustomSpecialSelect(btn);
            else onCustomSelect(btn);
            return;
        }

        GJGarageLayer::onSelect(sender);

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        auto savedType = MoreIconsAPI::savedForType(dual ? (IconType)sdi->getSavedValue("lasttype", 0) : m_selectedIconType, dual);
        if (!savedType.empty()) Mod::get()->setSavedValue<std::string>(savedType, "");
    }

    void newOn2PToggle(CCObject* sender) {
        CALL_BUTTON_ORIGINAL(sender);

        setupCustomPage(m_fields->m_pages[m_iconType]);
    }

    void swapDual(std::string_view name) {
        auto dualName = fmt::format("{}-dual", name);
        auto normalIcon = Mod::get()->getSavedValue<std::string>(name, "");
        auto dualIcon = Mod::get()->getSavedValue<std::string>(dualName, "");
        Mod::get()->setSavedValue(name, dualIcon);
        Mod::get()->setSavedValue(dualName, normalIcon);
    }

    void newSwap2PKit(CCObject* sender) {
        CALL_BUTTON_ORIGINAL(sender);

        swapDual("icon");
        swapDual("ship");
        swapDual("ball");
        swapDual("ufo");
        swapDual("wave");
        swapDual("robot");
        swapDual("spider");
        swapDual("swing");
        swapDual("jetpack");
        swapDual("trail");
        auto iconType = GameManager::get()->m_playerIconType;
        MoreIconsAPI::updateSimplePlayer(m_playerObject, MoreIconsAPI::activeForType(iconType, false), iconType);
        auto lastmode = (IconType)Loader::get()->getLoadedMod("weebify.separate_dual_icons")->getSavedValue("lastmode", 0);
        MoreIconsAPI::updateSimplePlayer(static_cast<SimplePlayer*>(getChildByID("player2-icon")), MoreIconsAPI::activeForType(lastmode, true), lastmode);
        setupCustomPage(m_fields->m_pages[m_iconType]);
    }

    void updatePlayerColors() {
        GJGarageLayer::updatePlayerColors();

        if (m_iconSelection && m_fields->m_pageBar && !MoreIconsAPI::vectorForType(m_iconType).empty()) m_iconSelection->setVisible(false);
    }

    void createNavMenu() {
        auto f = m_fields.self();
        auto winSize = CCDirector::get()->getWinSize();
        if (!f->m_navMenu) {
            f->m_navMenu = CCMenu::create();
            f->m_navMenu->setPosition({ winSize.width / 2, 15.0f });
            f->m_navMenu->setLayout(RowLayout::create()->setGap(6.0f)->setAxisAlignment(AxisAlignment::Center));
            f->m_navMenu->setContentSize({ winSize.width - 60.0f, 20.0f });
            f->m_navMenu->setID("navdot-menu"_spr);
            addChild(f->m_navMenu, 1);
        }

        auto& vec = MoreIconsAPI::vectorForType(m_iconType);
        m_navDotMenu->setPositionY(vec.empty() ? 25.0f : 35.0f);
        auto count = (GameManager::get()->countForType(m_iconType) + 35) / 36;
        if (count < 2) {
            m_navDotMenu->setVisible(true);
            m_navDotMenu->setEnabled(true);
            m_navDotMenu->removeAllChildren();
            auto firstDot = static_cast<CCMenuItemSpriteExtra*>(m_pageButtons->objectAtIndex(0));
            static_cast<CCSprite*>(firstDot->getNormalImage())->setDisplayFrame(CCSpriteFrameCache::get()->spriteFrameByName(
                f->m_pageBar && !vec.empty() ? "gj_navDotBtn_off_001.png" : "gj_navDotBtn_on_001.png"
            ));
            m_navDotMenu->addChild(firstDot);
            m_navDotMenu->updateLayout();
            m_leftArrow->setVisible(true);
            m_leftArrow->setEnabled(true);
            m_rightArrow->setVisible(true);
            m_rightArrow->setEnabled(true);
        }
        f->m_navMenu->setVisible(!vec.empty());
        if (vec.empty()) return;

        f->m_navMenu->removeAllChildren();
        auto navDotAmount = (vec.size() + 35) / 36;
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
    }

    void onNavigate(CCObject* sender) {
        GJGarageLayer::onNavigate(sender);

        m_fields->m_pages[m_iconType] = sender->getTag();

        createNavMenu();
    }

    void onCustomNavigate(CCObject* sender) {
        setupCustomPage(sender->getTag());
    }

    int wrapPage(IconType type, int page) {
        auto pages = (GameManager::get()->countForType(type) + 35) / 36 + (MoreIconsAPI::vectorForType(type).size() + 35) / 36;
        return pages > 0 ? page < 0 ? pages - 1 : page >= pages ? 0 : page : 0;
    }

    void onArrow(CCObject* sender) {
        GJGarageLayer::onArrow(sender);

        auto typeCount = GameManager::get()->countForType(m_iconType);
        auto maxPage = std::max((typeCount - 1) / 36, 0);
        auto tag = sender->getTag();
        m_iconPages[m_iconType] -= tag;
        auto vanillaPage = m_iconPages[m_iconType];
        if (vanillaPage < 0) m_iconPages[m_iconType] = maxPage;
        else if (vanillaPage > maxPage) m_iconPages[m_iconType] = 0;
        vanillaPage = m_iconPages[m_iconType];

        auto f = m_fields.self();
        f->m_pages[m_iconType] = wrapPage(m_iconType, f->m_pages[m_iconType] + tag);
        auto page = f->m_pages[m_iconType];

        auto& vec = MoreIconsAPI::vectorForType(m_iconType);
        if (page * 36 < typeCount) {
            m_iconPages[m_iconType] = page;
            if (tag == -1 && vanillaPage < 0) m_iconPages[m_iconType] = maxPage;
            else if (tag == 1 && vanillaPage > maxPage) m_iconPages[m_iconType] = 0;
            setupPage(m_iconPages[m_iconType], m_iconType);
            createNavMenu();
        }
        else setupCustomPage(page);
    }

    void selectTab(IconType type) {
        GJGarageLayer::selectTab(type);

        auto f = m_fields.self();
        if (!f->m_initialized) return;

        auto page = f->m_pages.contains(type) ? f->m_pages[type] : findIconPage(type);
        f->m_pages[type] = page;
        setupCustomPage(page);
    }

    std::vector<std::string> getPage() {
        auto& vec = MoreIconsAPI::vectorForType(m_iconType);
        auto customPage = m_fields->m_pages[m_iconType] - (GameManager::get()->countForType(m_iconType) + 35) / 36;
        if (vec.size() <= customPage * 36) return {};
        if (customPage < 0) return {};
        return std::vector<std::string>(vec.begin() + customPage * 36, vec.begin() + std::min((int)vec.size(), (customPage + 1) * 36));
    }

    void setupCustomPage(int page) {
        if (m_iconType == IconType::Special) {
            setupCustomSpecialPage(page);
            return;
        }

        auto& vec = MoreIconsAPI::vectorForType(m_iconType);
        auto f = m_fields.self();
        if (f->m_pageBar) {
            f->m_pageBar->removeFromParent();
            f->m_pageBar = nullptr;
        }
        auto gameManager = GameManager::get();
        if (vec.empty() || page * 36 < gameManager->countForType(m_iconType)) {
            createNavMenu();
            return;
        }

        m_iconSelection->setVisible(false);

        f->m_pages[m_iconType] = wrapPage(m_iconType, page);
        createNavMenu();

        auto spriteFrameCache = CCSpriteFrameCache::get();
        for (auto navDot : CCArrayExt<CCMenuItemSpriteExtra*>(m_navDotMenu->getChildren())) {
            static_cast<CCSprite*>(navDot->getNormalImage())->setDisplayFrame(spriteFrameCache->spriteFrameByName("gj_navDotBtn_off_001.png"));
        }

        auto unlockType = gameManager->iconTypeToUnlockType(m_iconType);
        auto playerSquare = CCSprite::createWithSpriteFrameName("playerSquare_001.png");
        auto objs = CCArray::create();
        CCMenuItemSpriteExtra* current = nullptr;
        int i = 1;
        auto hasAnimProf = Loader::get()->isModLoaded("thesillydoggo.animatedprofiles");
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        for (auto name : getPage()) {
            auto itemIcon = GJItemIcon::createBrowserItem(unlockType, 1);
            itemIcon->setScale(GJItemIcon::scaleForType(unlockType));
            MoreIconsAPI::updateSimplePlayer(itemIcon->m_player, name, m_iconType);
            if (hasAnimProf) {
                if (auto robotSprite = itemIcon->m_player->m_robotSprite) robotSprite->runAnimation("idle01");
                if (auto spiderSprite = itemIcon->m_player->m_spiderSprite) spiderSprite->runAnimation("idle01");
            }
            auto iconButton = CCMenuItemSpriteExtra::create(itemIcon, this, menu_selector(GJGarageLayer::onSelect));
            iconButton->setUserObject("name"_spr, CCString::create(name));
            iconButton->setContentSize(playerSquare->getContentSize());
            itemIcon->setPosition(iconButton->getContentSize() / 2);
            iconButton->setTag(i++);
            objs->addObject(iconButton);
            if (name == MoreIconsAPI::activeForType(m_iconType, dual)) current = iconButton;
        }

        f->m_pageBar = ListButtonBar::create(objs, CCDirector::get()->getWinSize() / 2 - CCPoint { 0.0f, 65.0f }, 12, 3, 5.0f, 5.0f, 25.0f, 220.0f, 1);
        f->m_pageBar->m_scrollLayer->togglePageIndicators(false);
        f->m_pageBar->setID("icon-selection-bar"_spr);
        addChild(f->m_pageBar, 101);

        m_cursor1->setVisible(current != nullptr);
        if (current) m_cursor1->setPosition(current->getParent()->convertToWorldSpace(current->getPosition()));
    }

    void onCustomSelect(CCMenuItemSpriteExtra* sender) {
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        std::string name = static_cast<CCString*>(sender->getUserObject("name"_spr))->getCString();

        m_cursor1->setPosition(sender->getParent()->convertToWorldSpace(sender->getPosition()));
        m_cursor1->setVisible(true);
        auto player = dual ? static_cast<SimplePlayer*>(getChildByID("player2-icon")) : m_playerObject;
        player->updatePlayerFrame(1, m_iconType);
        player->updateColors();
        MoreIconsAPI::updateSimplePlayer(player, name, m_iconType);
        if (!dual) GameManager::get()->m_playerIconType = m_iconType;
        player->setScale(m_iconType == IconType::Jetpack ? 1.5f : 1.6f);
        auto selectedIconType = dual ? (IconType)sdi->getSavedValue("lasttype", 0) : m_selectedIconType;
        if (MoreIconsAPI::activeForType(m_iconType, dual) == name && selectedIconType == m_iconType) {
            auto& iconInfo = MoreIcons::infoForType(m_iconType)[name];
            auto iconID = 1;
            if (!iconInfo.id.empty()) switch (m_iconType) {
                case IconType::Cube: iconID = 128; break;
                case IconType::Ship: iconID = 44; break;
                case IconType::Ball: iconID = 113; break;
                case IconType::Ufo: iconID = 95; break;
                case IconType::Wave: iconID = 75; break;
                case IconType::Robot: iconID = 51; break;
                case IconType::Spider: iconID = 18; break;
                case IconType::Swing: iconID = 7; break;
                case IconType::Jetpack: iconID = 5; break;
                default: break;
            }
            auto unlockType = GameManager::get()->iconTypeToUnlockType(m_iconType);
            auto popup = ItemInfoPopup::create(iconID, unlockType);
            if (auto nameLabel = static_cast<CCLabelBMFont*>(popup->m_mainLayer->getChildByID("name-label")))
                nameLabel->setString(name.substr(name.find_first_of(':') + 1).c_str());
            if (auto achLabel = static_cast<CCLabelBMFont*>(popup->m_mainLayer->getChildByID("achievement-label"))) achLabel->setString("Custom");
            if (auto popupIcon = static_cast<GJItemIcon*>(popup->m_mainLayer->getChildByIDRecursive("item-icon")))
                MoreIconsAPI::updateSimplePlayer(popupIcon->m_player, name, m_iconType);
            if (auto descText = static_cast<TextArea*>(popup->m_mainLayer->getChildByID("description-area"))) descText->setString(
                fmt::format("This <cg>{}</c> is added by the <cl>More Icons</c> mod.", std::string(ItemInfoPopup::nameForUnlockType(1, unlockType))));
            if (auto completionMenu = popup->m_mainLayer->getChildByID("completionMenu")) completionMenu->setVisible(false);
            if (auto infoButton = popup->m_buttonMenu->getChildByID("infoButton")) infoButton->setVisible(false);
            if (!iconInfo.id.empty()) {
                if (auto creditButton = static_cast<CCMenuItemSpriteExtra*>(popup->m_buttonMenu->getChildByID("author-button"))) {
                    auto creditText = static_cast<CCLabelBMFont*>(creditButton->getNormalImage());
                    creditText->setString(iconInfo.name.c_str());
                    creditText->limitLabelWidth(100.0f, 0.5f, 0.0f);
                    creditButton->setEnabled(false);
                    creditButton->updateSprite();
                }
            }
            popup->show();
        }
        if (dual) {
            sdi->setSavedValue("lastmode", (int)m_iconType);
            sdi->setSavedValue("lasttype", (int)m_iconType);
        }
        else m_selectedIconType = m_iconType;
    }

    void setupCustomSpecialPage(int page) {
        auto& vec = MoreIconsAPI::vectorForType(m_iconType);
        auto f = m_fields.self();
        if (f->m_pageBar) {
            f->m_pageBar->removeFromParent();
            f->m_pageBar = nullptr;
        }
        if (vec.empty() || page * 36 < GameManager::get()->countForType(m_iconType)) {
            createNavMenu();
            return;
        }

        m_iconSelection->setVisible(false);

        f->m_pages[m_iconType] = wrapPage(m_iconType, page);
        createNavMenu();

        m_leftArrow->setVisible(vec.size() > 36);
        m_rightArrow->setVisible(vec.size() > 36);

        auto spriteFrameCache = CCSpriteFrameCache::get();
        for (auto navDot : CCArrayExt<CCMenuItemSpriteExtra*>(m_navDotMenu->getChildren())) {
            static_cast<CCSprite*>(navDot->getNormalImage())->setDisplayFrame(spriteFrameCache->spriteFrameByName("gj_navDotBtn_off_001.png"));
        }

        auto objs = CCArray::create();
        CCMenuItemSpriteExtra* current = nullptr;
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        int i = 1;
        for (auto name : getPage()) {
            auto square = CCSprite::createWithSpriteFrameName("playerSquare_001.png");
            square->setColor({ 150, 150, 150 });
            auto texture = CCTextureCache::get()->textureForKey(MoreIcons::TRAIL_INFO[name].texture.c_str());
            auto streak = CCSprite::createWithTexture(texture);
            limitNodeHeight(streak, 27.0f, 999.0f, 0.001f);
            streak->setRotation(-90.0f);
            square->addChild(streak);
            streak->setPosition(square->getContentSize() / 2);
            square->setScale(0.8f);
            auto iconButton = CCMenuItemSpriteExtra::create(square, this, menu_selector(GJGarageLayer::onSelect));
            iconButton->setUserObject("name"_spr, CCString::create(name));
            iconButton->setTag(i++);
            objs->addObject(iconButton);
            if (name == MoreIconsAPI::activeForType(m_iconType, dual)) current = iconButton;
        }

        f->m_pageBar = ListButtonBar::create(objs, CCDirector::get()->getWinSize() / 2 - CCPoint { 0.0f, 65.0f }, 12, 3, 5.0f, 5.0f, 25.0f, 220.0f, 1);
        f->m_pageBar->m_scrollLayer->togglePageIndicators(false);
        f->m_pageBar->setID("icon-selection-bar"_spr);
        addChild(f->m_pageBar, 101);

        m_cursor1->setVisible(current != nullptr);
        if (current) m_cursor1->setPosition(current->getParent()->convertToWorldSpace(current->getPosition()));
        m_cursor2->setVisible(false);
    }

    void onCustomSpecialSelect(CCMenuItemSpriteExtra* sender) {
        using namespace std::string_view_literals;

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        std::string name = static_cast<CCString*>(sender->getUserObject("name"_spr))->getCString();

        m_cursor1->setPosition(sender->getParent()->convertToWorldSpace(sender->getPosition()));
        m_cursor1->setVisible(true);
        auto selectedIconType = dual ? (IconType)sdi->getSavedValue("lasttype", 0) : m_selectedIconType;
        if (MoreIconsAPI::activeForType(m_iconType, dual) == name && selectedIconType == m_iconType) {
            auto trailInfo = MoreIcons::TRAIL_INFO[name];
            auto popup = ItemInfoPopup::create(!trailInfo.pack.id.empty() ? 128 : 1, UnlockType::Cube);
            if (auto nameLabel = static_cast<CCLabelBMFont*>(popup->m_mainLayer->getChildByID("name-label")))
                nameLabel->setString(name.substr(name.find_first_of(':') + 1).c_str());
            if (auto achLabel = static_cast<CCLabelBMFont*>(popup->m_mainLayer->getChildByID("achievement-label"))) achLabel->setString("Custom");
            if (auto popupIcon = static_cast<GJItemIcon*>(popup->m_mainLayer->getChildByIDRecursive("item-icon"))) {
                popupIcon->setVisible(false);
                auto square = CCSprite::createWithSpriteFrameName("playerSquare_001.png");
                square->setColor({ 150, 150, 150 });
                auto streak = CCSprite::createWithTexture(CCTextureCache::get()->textureForKey(MoreIcons::TRAIL_INFO[name].texture.c_str()));
                limitNodeHeight(streak, 27.0f, 999.0f, 0.001f);
                streak->setRotation(-90.0f);
                square->addChild(streak);
                streak->setPosition(square->getContentSize() / 2);
                square->setPosition(popupIcon->getPosition());
                square->setScale(popupIcon->getScale());
                square->setID("trail-square"_spr);
                popup->m_mainLayer->addChild(square);
            }
            if (auto descText = static_cast<TextArea*>(popup->m_mainLayer->getChildByID("description-area"))) descText->setString(
                fmt::format("This <cg>{}</c> is added by the <cl>More Icons</c> mod.", std::string(ItemInfoPopup::nameForUnlockType(1, UnlockType::Streak))));
            if (auto completionMenu = popup->m_mainLayer->getChildByID("completionMenu")) completionMenu->setVisible(false);
            if (auto infoButton = popup->m_buttonMenu->getChildByID("infoButton")) infoButton->setVisible(false);
            if (!trailInfo.pack.id.empty()) {
                if (auto creditButton = static_cast<CCMenuItemSpriteExtra*>(popup->m_buttonMenu->getChildByID("author-button"))) {
                    auto creditText = static_cast<CCLabelBMFont*>(creditButton->getNormalImage());
                    creditText->setString(trailInfo.pack.name.c_str());
                    creditText->limitLabelWidth(100.0f, 0.5f, 0.0f);
                    creditButton->setEnabled(false);
                    creditButton->updateSprite();
                }
            }
            for (auto child : CCArrayExt<CCNode*>(popup->m_buttonMenu->getChildren())) {
                if (auto button = typeinfo_cast<CCMenuItemSpriteExtra*>(child)) {
                    if (auto normalImage = typeinfo_cast<CCSprite*>(button->getNormalImage())) {
                        if (auto imageLabel = normalImage->getChildByType<CCLabelBMFont>(0)) {
                            if (imageLabel->getString() == "P1"sv || imageLabel->getString() == "P2"sv || imageLabel->getString() == "G"sv)
                                button->setVisible(false);
                        }
                    }
                }
            }
            auto winSize = CCDirector::get()->getWinSize();
            auto blendToggler = CCMenuItemExt::createTogglerWithStandardSprites(0.5f, [this, name](CCMenuItemToggler* sender) {
                MoreIcons::TRAIL_INFO[name].blend = !sender->isToggled();
            });
            blendToggler->setPosition(popup->m_buttonMenu->convertToNodeSpace(winSize / 2 - CCPoint { 123.0f, 78.0f }));
            blendToggler->toggle(trailInfo.blend);
            blendToggler->setID("blend-toggler"_spr);
            popup->m_buttonMenu->addChild(blendToggler);
            auto blendLabel = CCLabelBMFont::create("Blend", "bigFont.fnt");
            blendLabel->setPosition(winSize / 2 - CCPoint { 112.0f, 78.0f });
            blendLabel->setAnchorPoint({ 0.0f, 0.5f });
            blendLabel->setScale(0.3f);
            blendLabel->setID("blend-label"_spr);
            popup->m_mainLayer->addChild(blendLabel);
            auto tintToggler = CCMenuItemExt::createTogglerWithStandardSprites(0.5f, [this, name](CCMenuItemToggler* sender) {
                MoreIcons::TRAIL_INFO[name].tint = !sender->isToggled();
            });
            tintToggler->setPosition(popup->m_buttonMenu->convertToNodeSpace(winSize / 2 - CCPoint { 123.0f, 98.0f }));
            tintToggler->toggle(trailInfo.tint);
            tintToggler->setID("tint-toggler"_spr);
            popup->m_buttonMenu->addChild(tintToggler);
            auto tintLabel = CCLabelBMFont::create("Tint", "bigFont.fnt");
            tintLabel->setPosition(winSize / 2 - CCPoint { 112.0f, 98.0f });
            tintLabel->setAnchorPoint({ 0.0f, 0.5f });
            tintLabel->setScale(0.3f);
            tintLabel->setID("tint-label"_spr);
            popup->m_mainLayer->addChild(tintLabel);
            popup->show();
        }
        if (dual) sdi->setSavedValue("lasttype", (int)m_iconType);
        else m_selectedIconType = m_iconType;
    }
};

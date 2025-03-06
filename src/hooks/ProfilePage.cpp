#include "../api/MoreIconsAPI.hpp"
#include "../classes/ButtonHooker.hpp"
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/ProfilePage.hpp>

using namespace geode::prelude;

class $modify(MIProfilePage, ProfilePage) {
    static void onModify(ModifyBase<ModifyDerive<MIProfilePage, ProfilePage>>& self) {
        (void)self.setHookPriorityAfterPost("ProfilePage::loadPageFromUserInfo", "weebify.separate_dual_icons");
    }

    SimplePlayer* findPlayer(CCNode* node) {
        if (!node) return nullptr;
        return findFirstChildRecursive<SimplePlayer>(node, [](auto) { return true; });
    }

    void changePlayers() {
        auto playerMenu = m_mainLayer->getChildByID("player-menu");
        if (!playerMenu) return;

        auto playerShip = playerMenu->getChildByID("player-ship");
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        MoreIconsAPI::updateSimplePlayer(findPlayer(playerMenu->getChildByID("player-icon")), IconType::Cube, dual);
        MoreIconsAPI::updateSimplePlayer(findPlayer(playerShip), playerShip ? (IconType)playerShip->getTag() : IconType::Ship, dual);
        MoreIconsAPI::updateSimplePlayer(findPlayer(playerMenu->getChildByID("player-ball")), IconType::Ball, dual);
        MoreIconsAPI::updateSimplePlayer(findPlayer(playerMenu->getChildByID("player-ufo")), IconType::Ufo, dual);
        MoreIconsAPI::updateSimplePlayer(findPlayer(playerMenu->getChildByID("player-wave")), IconType::Wave, dual);
        MoreIconsAPI::updateSimplePlayer(findPlayer(playerMenu->getChildByID("player-robot")), IconType::Robot, dual);
        MoreIconsAPI::updateSimplePlayer(findPlayer(playerMenu->getChildByID("player-spider")), IconType::Spider, dual);
        MoreIconsAPI::updateSimplePlayer(findPlayer(playerMenu->getChildByID("player-swing")), IconType::Swing, dual);
        MoreIconsAPI::updateSimplePlayer(findPlayer(playerMenu->getChildByID("player-jetpack")), IconType::Jetpack, dual);
    }

    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);

        if (!m_ownProfile) return;

        changePlayers();

        if (Loader::get()->isModLoaded("weebify.separate_dual_icons")) {
            if (auto leftMenu = m_mainLayer->getChildByID("left-menu")) {
                if (auto twoPToggler = static_cast<CCMenuItemSpriteExtra*>(leftMenu->getChildByID("2p-toggler")))
                    ButtonHooker::create(twoPToggler, this, menu_selector(MIProfilePage::newOn2PToggle));
            }
        }
    }

    void newOn2PToggle(CCObject* sender) {
        CALL_BUTTON_ORIGINAL(sender);

        changePlayers();
    }

    void toggleShip(CCObject* sender) {
        ProfilePage::toggleShip(sender);

        if (!m_ownProfile) return;

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        MoreIconsAPI::updateSimplePlayer(findPlayer(static_cast<CCNode*>(sender)),
            (IconType)sender->getTag(), sdi && sdi->getSavedValue("2pselected", false));
    }
};

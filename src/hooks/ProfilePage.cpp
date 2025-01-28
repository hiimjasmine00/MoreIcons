#include "../MoreIcons.hpp"
#include "../classes/ButtonHooker.hpp"
#include <Geode/modify/ProfilePage.hpp>

using namespace geode::prelude;

class $modify(MIProfilePage, ProfilePage) {
    static void onModify(auto& self) {
        (void)self.setHookPriority("ProfilePage::loadPageFromUserInfo", -1);
    }

    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);

        if (!m_ownProfile) return;

        auto playerMenu = m_mainLayer->getChildByID("player-menu");
        if (!playerMenu) return;

        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-icon")), IconType::Cube);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-ship")), IconType::Ship);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-ball")), IconType::Ball);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-ufo")), IconType::Ufo);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-wave")), IconType::Wave);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-robot")), IconType::Robot);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-spider")), IconType::Spider);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-swing")), IconType::Swing);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-jetpack")), IconType::Jetpack);

        if (Loader::get()->isModLoaded("weebify.separate_dual_icons")) {
            if (auto leftMenu = m_mainLayer->getChildByID("left-menu")) {
                if (auto twoPToggler = static_cast<CCMenuItemSpriteExtra*>(leftMenu->getChildByID("2p-toggler")))
                    ButtonHooker::create(twoPToggler, this, menu_selector(MIProfilePage::newOn2PToggle));
            }
        }
    }

    void newOn2PToggle(CCObject* sender) {
        CALL_BUTTON_ORIGINAL(sender);

        auto playerMenu = m_mainLayer->getChildByID("player-menu");
        if (!playerMenu) return;

        auto playerShip = playerMenu->getChildByID("player-ship");
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-icon")), IconType::Cube);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerShip), playerShip ? (IconType)playerShip->getTag() : IconType::Ship);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-ball")), IconType::Ball);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-ufo")), IconType::Ufo);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-wave")), IconType::Wave);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-robot")), IconType::Robot);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-spider")), IconType::Spider);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-swing")), IconType::Swing);
        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(playerMenu->getChildByID("player-jetpack")), IconType::Jetpack);
    }

    void toggleShip(CCObject* sender) {
        ProfilePage::toggleShip(sender);

        if (!m_ownProfile) return;

        MoreIcons::changeSimplePlayer(MoreIcons::findPlayer(static_cast<CCNode*>(sender)), (IconType)sender->getTag());
    }
};

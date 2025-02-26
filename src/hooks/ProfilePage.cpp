#include "../MoreIcons.hpp"
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
        MoreIcons::changeSimplePlayer(findPlayer(playerMenu->getChildByID("player-icon")), IconType::Cube);
        MoreIcons::changeSimplePlayer(findPlayer(playerShip), playerShip ? (IconType)playerShip->getTag() : IconType::Ship);
        MoreIcons::changeSimplePlayer(findPlayer(playerMenu->getChildByID("player-ball")), IconType::Ball);
        MoreIcons::changeSimplePlayer(findPlayer(playerMenu->getChildByID("player-ufo")), IconType::Ufo);
        MoreIcons::changeSimplePlayer(findPlayer(playerMenu->getChildByID("player-wave")), IconType::Wave);
        MoreIcons::changeSimplePlayer(findPlayer(playerMenu->getChildByID("player-robot")), IconType::Robot);
        MoreIcons::changeSimplePlayer(findPlayer(playerMenu->getChildByID("player-spider")), IconType::Spider);
        MoreIcons::changeSimplePlayer(findPlayer(playerMenu->getChildByID("player-swing")), IconType::Swing);
        MoreIcons::changeSimplePlayer(findPlayer(playerMenu->getChildByID("player-jetpack")), IconType::Jetpack);
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

        MoreIcons::changeSimplePlayer(findPlayer(static_cast<CCNode*>(sender)), (IconType)sender->getTag());
    }
};

#include "../api/MoreIconsAPI.hpp"
#include "../classes/misc/ButtonHooker.hpp"
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/ProfilePage.hpp>

using namespace geode::prelude;

class $modify(MIProfilePage, ProfilePage) {
    static void onModify(ModifyBase<ModifyDerive<MIProfilePage, ProfilePage>>& self) {
        (void)self.setHookPriorityAfterPost("ProfilePage::loadPageFromUserInfo", "weebify.separate_dual_icons");
    }

    static void updatePlayer(CCNode* node, IconType type, bool dual) {
        if (auto player = findFirstChildRecursive<SimplePlayer>(node, [](auto) { return true; })) MoreIconsAPI::updateSimplePlayer(player, type, dual);
    }

    void changePlayers() {
        auto playerMenu = m_mainLayer->getChildByID("player-menu");
        if (!playerMenu) return;

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);

        constexpr std::array icons = {
            "player-icon", "player-ship", "player-ball",
            "player-ufo", "player-wave", "player-robot",
            "player-spider", "player-swing", "player-jetpack"
        };

        for (int i = 0; i < icons.size(); i++) {
            if (auto player = playerMenu->getChildByID(icons[i]))
                updatePlayer(player, i == 1 && player->getTag() != -1 ? (IconType)player->getTag() : (IconType)i, dual);
        }
    }

    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);

        if (!m_ownProfile) return;

        changePlayers();

        if (Loader::get()->isModLoaded("weebify.separate_dual_icons")) {
            if (auto leftMenu = m_mainLayer->getChildByID("left-menu"))
                ButtonHooker::create(static_cast<CCMenuItem*>(leftMenu->getChildByID("2p-toggler")), this, menu_selector(MIProfilePage::newOn2PToggle));
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
        MoreIconsAPI::updateSimplePlayer(static_cast<SimplePlayer*>(static_cast<CCMenuItemSprite*>(sender)->getNormalImage()),
            (IconType)sender->getTag(), sdi && sdi->getSavedValue("2pselected", false));
    }
};

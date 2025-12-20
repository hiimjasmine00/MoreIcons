#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <jasmine/button.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;
using namespace jasmine::button;

class $modify(MIProfilePage, ProfilePage) {
    static void onModify(ModifyBase<ModifyDerive<MIProfilePage, ProfilePage>>& self) {
        (void)self.setHookPriorityAfterPost("ProfilePage::loadPageFromUserInfo", "weebify.separate_dual_icons");
    }

    static bool updatePlayer(CCNode* node, IconType type, bool dual) {
        if (auto player = typeinfo_cast<SimplePlayer*>(node)) {
            more_icons::updateSimplePlayer(player, type, dual);
            return true;
        }
        for (auto child : node->getChildrenExt()) {
            if (updatePlayer(child, type, dual)) return true;
        }
        return false;
    }

    static void updatePlayer(CCNode* menu, std::string_view id, IconType type, bool dual) {
        if (auto player = menu->getChildByID(id)) {
            auto tag = type == IconType::Ship ? player->getTag() : -1;
            updatePlayer(player, tag != -1 ? (IconType)tag : type, dual);
        }
    }

    void changePlayers() {
        auto playerMenu = m_mainLayer->getChildByID("player-menu");
        if (!playerMenu) return;

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);

        updatePlayer(playerMenu, "player-icon", IconType::Cube, dual);
        updatePlayer(playerMenu, "player-ship", IconType::Ship, dual);
        updatePlayer(playerMenu, "player-ball", IconType::Ball, dual);
        updatePlayer(playerMenu, "player-ufo", IconType::Ufo, dual);
        updatePlayer(playerMenu, "player-wave", IconType::Wave, dual);
        updatePlayer(playerMenu, "player-robot", IconType::Robot, dual);
        updatePlayer(playerMenu, "player-spider", IconType::Spider, dual);
        updatePlayer(playerMenu, "player-swing", IconType::Swing, dual);
        updatePlayer(playerMenu, "player-jetpack", IconType::Jetpack, dual);
    }

    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);

        if (!m_ownProfile) return;

        changePlayers();

        if (Loader::get()->isModLoaded("weebify.separate_dual_icons")) {
            if (auto leftMenu = m_mainLayer->getChildByID("left-menu")) {
                ButtonHooker::create(
                    static_cast<CCMenuItem*>(leftMenu->getChildByID("2p-toggler")),
                    this, menu_selector(MIProfilePage::newOn2PToggle)
                );
            }
        }
    }

    void newOn2PToggle(CCObject* sender) {
        ButtonHooker::call(sender);

        changePlayers();
    }

    void toggleShip(CCObject* sender) {
        ProfilePage::toggleShip(sender);

        if (!m_ownProfile) return;

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        more_icons::updateSimplePlayer(static_cast<SimplePlayer*>(static_cast<CCMenuItemSprite*>(sender)->getNormalImage()),
            (IconType)sender->getTag(), sdi && sdi->getSavedValue("2pselected", false));
    }
};

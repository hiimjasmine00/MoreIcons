#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <jasmine/button.hpp>

using namespace geode::prelude;
using namespace jasmine::button;

class $modify(MIProfilePage, ProfilePage) {
    static void onModify(ModifyBase<ModifyDerive<MIProfilePage, ProfilePage>>& self) {
        (void)self.setHookPriorityAfterPost("ProfilePage::loadPageFromUserInfo", "weebify.separate_dual_icons");
    }

    static void updatePlayer(CCNode* node, IconType type, bool dual) {
        if (auto player = findFirstChildRecursive<SimplePlayer>(node, [](auto) { return true; })) {
            MoreIconsAPI::updateSimplePlayer(player, type, dual);
        }
    }

    void changePlayers() {
        auto playerMenu = m_mainLayer->getChildByID("player-menu");
        if (!playerMenu) return;

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);

        for (int i = 0; i < 9; i++) {
            if (auto player = playerMenu->getChildByID(fmt::format("player-{}", MoreIcons::folders[i]))) {
                auto tag = i == 1 ? player->getTag() : -1;
                updatePlayer(player, (IconType)(tag != -1 ? tag : i), dual);
            }
        }
    }

    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);

        if (!m_ownProfile) return;

        changePlayers();

        if (Loader::get()->isModLoaded("weebify.separate_dual_icons")) {
            if (auto leftMenu = m_mainLayer->getChildByID("left-menu")) {
                ButtonHooker::create(static_cast<CCMenuItem*>(leftMenu->getChildByID("2p-toggler")),
                    this, menu_selector(MIProfilePage::newOn2PToggle));
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
        MoreIconsAPI::updateSimplePlayer(static_cast<SimplePlayer*>(static_cast<CCMenuItemSprite*>(sender)->getNormalImage()),
            (IconType)sender->getTag(), sdi && sdi->getSavedValue("2pselected", false));
    }
};

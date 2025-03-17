#include "../api/MoreIconsAPI.hpp"
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/CharacterColorPage.hpp>

using namespace geode::prelude;

class $modify(MICharacterColorPage, CharacterColorPage) {
    static void onModify(ModifyBase<ModifyDerive<MICharacterColorPage, CharacterColorPage>>& self) {
        (void)self.setHookPriorityAfterPost("CharacterColorPage::init", "weebify.separate_dual_icons");
        (void)self.setHookPriorityAfterPost("CharacterColorPage::toggleShip", "weebify.separate_dual_icons");
    }

    bool init() {
        if (!CharacterColorPage::init()) return false;

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto dual = sdi && sdi->getSavedValue("2pselected", false);
        for (int i = 0; i < m_playerObjects->count(); i++) {
            MoreIconsAPI::updateSimplePlayer(static_cast<SimplePlayer*>(m_playerObjects->objectAtIndex(i)), (IconType)i, dual);
        }

        return true;
    }

    void toggleShip(CCObject* sender) {
        CharacterColorPage::toggleShip(sender);

        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        MoreIconsAPI::updateSimplePlayer(static_cast<SimplePlayer*>(static_cast<CCMenuItemSprite*>(sender)->getNormalImage()),
            (IconType)sender->getTag(), sdi && sdi->getSavedValue("2pselected", false));
    }
};

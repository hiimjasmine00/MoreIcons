#include "../MoreIcons.hpp"
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/CharacterColorPage.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

class $modify(MICharacterColorPage, CharacterColorPage) {
    static void onModify(ModifyBase<ModifyDerive<MICharacterColorPage, CharacterColorPage>>& self) {
        (void)self.setHookPriorityAfterPost("CharacterColorPage::init", "weebify.separate_dual_icons");
        (void)self.setHookPriorityAfterPost("CharacterColorPage::toggleShip", "weebify.separate_dual_icons");
    }

    bool init() {
        if (!CharacterColorPage::init()) return false;

        auto dual = MoreIcons::dualSelected();
        auto playerObjects = m_playerObjects->asExt<SimplePlayer>();
        for (size_t i = 0; i < playerObjects.size(); i++) {
            more_icons::updateSimplePlayer(playerObjects[i], (IconType)i, dual);
        }

        return true;
    }

    void toggleShip(CCObject* sender) {
        CharacterColorPage::toggleShip(sender);

        more_icons::updateSimplePlayer(static_cast<SimplePlayer*>(static_cast<CCMenuItemSprite*>(sender)->getNormalImage()),
            (IconType)sender->getTag(), MoreIcons::dualSelected());
    }
};

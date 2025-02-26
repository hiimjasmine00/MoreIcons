#include "../MoreIcons.hpp"
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

        for (int i = 0; i < m_playerObjects->count(); i++) {
            MoreIcons::changeSimplePlayer(static_cast<SimplePlayer*>(m_playerObjects->objectAtIndex(i)), (IconType)i);
        }

        return true;
    }

    void toggleShip(CCObject* sender) {
        CharacterColorPage::toggleShip(sender);

        MoreIcons::changeSimplePlayer(static_cast<SimplePlayer*>(static_cast<CCMenuItemSpriteExtra*>(sender)->getNormalImage()), (IconType)sender->getTag());
    }
};

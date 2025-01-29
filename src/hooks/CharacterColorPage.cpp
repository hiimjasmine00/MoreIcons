#include "../MoreIcons.hpp"
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/modify/CharacterColorPage.hpp>

using namespace geode::prelude;

class $modify(MICharacterColorPage, CharacterColorPage) {
    static void onModify(auto& self) {
        (void)self.setHookPriority("CharacterColorPage::init", -1);
        (void)self.setHookPriority("CharacterColorPage::toggleShip", -1);
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

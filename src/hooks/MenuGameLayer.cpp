#include "../api/MoreIconsAPI.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/modify/MenuGameLayer.hpp>

using namespace geode::prelude;

class $modify(MIMenuGameLayer, MenuGameLayer) {
    static void onModify(ModifyBase<ModifyDerive<MIMenuGameLayer, MenuGameLayer>>& self) {
        (void)self.setHookPriorityBeforePost("MenuGameLayer::resetPlayer", "iandyhd3.known_players");
        (void)self.setHookPriorityBeforePost("MenuGameLayer::resetPlayer", "kittenchilly.pity_title_screen_secret_icons");
    }

    void resetPlayer() {
        MenuGameLayer::resetPlayer();

        auto iconType = MoreIconsAPI::getIconType(m_playerObject);
        auto gameManager = GameManager::get();
        auto icons = gameManager->countForType(iconType);
        auto vehicle = m_playerObject->m_isShip || m_playerObject->m_isBird;
        auto cubes = vehicle ? gameManager->countForType(IconType::Cube) : 0;

        auto icon = (int)roundf((rand() / (float)RAND_MAX) * (icons + MoreIconsAPI::getCount(iconType) - 1)) + 1;
        auto cube = vehicle ? (int)roundf((rand() / (float)RAND_MAX) * (cubes + MoreIconsAPI::getCount(IconType::Cube) - 1)) + 1 : 0;

        if (icon > icons) {
            MoreIconsAPI::updatePlayerObject(m_playerObject, MoreIconsAPI::iconSpans[iconType][icon - icons - 1].name, iconType);
            if (vehicle) {
                if (cube > cubes) MoreIconsAPI::updatePlayerObject(m_playerObject, MoreIconsAPI::icons[cube - cubes - 1].name, IconType::Cube);
                else m_playerObject->updatePlayerFrame(cube);
            }
        }
        else if (m_playerObject->m_isShip) {
            m_playerObject->updatePlayerShipFrame(icon);
            if (cube > cubes) MoreIconsAPI::updatePlayerObject(m_playerObject, MoreIconsAPI::icons[cube - cubes - 1].name, IconType::Cube);
            else m_playerObject->updatePlayerFrame(cube);
        }
        else if (m_playerObject->m_isBall) m_playerObject->updatePlayerRollFrame(icon);
        else if (m_playerObject->m_isBird) {
            m_playerObject->updatePlayerBirdFrame(icon);
            if (cube > cubes) MoreIconsAPI::updatePlayerObject(m_playerObject, MoreIconsAPI::icons[cube - cubes - 1].name, IconType::Cube);
            else m_playerObject->updatePlayerFrame(cube);
        }
        else if (m_playerObject->m_isDart) m_playerObject->updatePlayerDartFrame(icon);
        else if (m_playerObject->m_isRobot) m_playerObject->updatePlayerRobotFrame(icon);
        else if (m_playerObject->m_isSpider) m_playerObject->updatePlayerSpiderFrame(icon);
        else if (m_playerObject->m_isSwing) m_playerObject->updatePlayerSwingFrame(icon);
        else m_playerObject->updatePlayerFrame(icon);
    }
};

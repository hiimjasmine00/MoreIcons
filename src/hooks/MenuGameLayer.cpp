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

        auto type = MoreIconsAPI::getIconType(m_playerObject);
        auto gameManager = MoreIconsAPI::get<GameManager>();

        auto& icons = MoreIconsAPI::icons[type];
        auto iconCount = gameManager->countForType(type);
        auto icon = (int)roundf((rand() / (float)RAND_MAX) * (iconCount + icons.size() - 1)) + 1;

        if (icon > iconCount) MoreIconsAPI::updatePlayerObject(m_playerObject, icons[icon - iconCount - 1].name, type);
        else if (m_playerObject->m_isShip) m_playerObject->updatePlayerShipFrame(icon);
        else if (m_playerObject->m_isBall) m_playerObject->updatePlayerRollFrame(icon);
        else if (m_playerObject->m_isBird) m_playerObject->updatePlayerBirdFrame(icon);
        else if (m_playerObject->m_isDart) m_playerObject->updatePlayerDartFrame(icon);
        else if (m_playerObject->m_isRobot) m_playerObject->updatePlayerRobotFrame(icon);
        else if (m_playerObject->m_isSpider) m_playerObject->updatePlayerSpiderFrame(icon);
        else if (m_playerObject->m_isSwing) m_playerObject->updatePlayerSwingFrame(icon);
        else m_playerObject->updatePlayerFrame(icon);

        if (m_playerObject->m_isShip || m_playerObject->m_isBird) {
            auto& cubes = MoreIconsAPI::icons[IconType::Cube];
            auto cubeCount = gameManager->countForType(IconType::Cube);
            auto cube = (int)roundf((rand() / (float)RAND_MAX) * (cubeCount + cubes.size() - 1)) + 1;

            if (cube > cubeCount) MoreIconsAPI::updatePlayerObject(m_playerObject, cubes[cube - cubeCount - 1].name, IconType::Cube);
            else m_playerObject->updatePlayerFrame(cube);
        }
    }
};

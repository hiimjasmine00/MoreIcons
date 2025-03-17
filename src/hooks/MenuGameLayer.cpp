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

        auto iconType = MoreIconsClass::getIconType(m_playerObject);
        auto gameManager = GameManager::get();
        auto iconCount = gameManager->countForType(iconType);
        auto hasCube = m_playerObject->m_isShip || m_playerObject->m_isBird;
        auto cubeCount = hasCube ? gameManager->countForType(IconType::Cube) : 0;

        auto randomIcon = (int)roundf((rand() / (float)RAND_MAX) * (iconCount + MoreIconsAPI::getCount(iconType) - 1)) + 1;
        auto randomCube = hasCube ? (int)roundf((rand() / (float)RAND_MAX) * (cubeCount + MoreIconsAPI::getCount(IconType::Cube) - 1)) + 1 : 0;

        if (randomIcon > iconCount) {
            MoreIconsAPI::updatePlayerObject(m_playerObject,
                MoreIconsAPI::ICONS[MoreIconsAPI::ICON_INDICES[iconType].first + randomIcon - iconCount - 1].name, iconType);
            if (hasCube) {
                if (randomCube > cubeCount)
                    MoreIconsAPI::updatePlayerObject(m_playerObject, MoreIconsAPI::ICONS[randomCube - cubeCount - 1].name, IconType::Cube);
                else m_playerObject->updatePlayerFrame(randomCube);
            }
        }
        else if (m_playerObject->m_isShip) {
            m_playerObject->updatePlayerShipFrame(randomIcon);
            if (randomCube > cubeCount)
                MoreIconsAPI::updatePlayerObject(m_playerObject, MoreIconsAPI::ICONS[randomCube - cubeCount - 1].name, IconType::Cube);
            else m_playerObject->updatePlayerFrame(randomCube);
        }
        else if (m_playerObject->m_isBall) m_playerObject->updatePlayerRollFrame(randomIcon);
        else if (m_playerObject->m_isBird) {
            m_playerObject->updatePlayerBirdFrame(randomIcon);
            if (randomCube > cubeCount)
                MoreIconsAPI::updatePlayerObject(m_playerObject, MoreIconsAPI::ICONS[randomCube - cubeCount - 1].name, IconType::Cube);
            else m_playerObject->updatePlayerFrame(randomCube);
        }
        else if (m_playerObject->m_isDart) m_playerObject->updatePlayerDartFrame(randomIcon);
        else if (m_playerObject->m_isRobot) m_playerObject->updatePlayerRobotFrame(randomIcon);
        else if (m_playerObject->m_isSpider) m_playerObject->updatePlayerSpiderFrame(randomIcon);
        else if (m_playerObject->m_isSwing) m_playerObject->updatePlayerSwingFrame(randomIcon);
        else m_playerObject->updatePlayerFrame(randomIcon);
    }
};

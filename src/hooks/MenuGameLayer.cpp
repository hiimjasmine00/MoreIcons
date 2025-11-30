#include "../api/MoreIconsAPI.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/modify/MenuGameLayer.hpp>
#include <jasmine/hook.hpp>
#include <jasmine/random.hpp>

using namespace geode::prelude;

class $modify(MIMenuGameLayer, MenuGameLayer) {
    static void onModify(ModifyBase<ModifyDerive<MIMenuGameLayer, MenuGameLayer>>& self) {
        auto loader = Loader::get();
        auto hook = jasmine::hook::get(self.m_hooks, "MenuGameLayer::resetPlayer", true);
        if (auto knownPlayers = loader->getInstalledMod("iandyhd3.known_players")) {
            if (knownPlayers->isEnabled()) {
                hook->setAutoEnable(false);
            }
            else {
                new EventListener([hook](ModStateEvent*) {
                    jasmine::hook::toggle(hook, false);
                    return ListenerResult::Propagate;
                }, ModStateFilter(knownPlayers, ModEventType::Loaded));
            }
        }
        if (auto pityIcons = loader->getInstalledMod("kittenchilly.pity_title_screen_secret_icons")) {
            if (pityIcons->isEnabled()) {
                hook->setAutoEnable(false);
            }
            else {
                new EventListener([hook](ModStateEvent*) {
                    jasmine::hook::toggle(hook, false);
                    return ListenerResult::Propagate;
                }, ModStateFilter(pityIcons, ModEventType::Loaded));
            }
        }
    }

    void resetPlayer() {
        MenuGameLayer::resetPlayer();

        auto type = MoreIconsAPI::getIconType(m_playerObject);
        auto gameManager = MoreIconsAPI::getGameManager();

        auto& icons = MoreIconsAPI::icons[type];
        auto iconCount = gameManager->countForType(type);
        int icon = round(jasmine::random::get(1, iconCount + icons.size()));

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
            int cube = round(jasmine::random::get(1, cubeCount + cubes.size()));

            if (cube > cubeCount) MoreIconsAPI::updatePlayerObject(m_playerObject, cubes[cube - cubeCount - 1].name, IconType::Cube);
            else m_playerObject->updatePlayerFrame(cube);
        }
    }
};

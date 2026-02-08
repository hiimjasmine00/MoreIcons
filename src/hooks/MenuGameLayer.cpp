#include "../utils/Get.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/modify/MenuGameLayer.hpp>
#include <jasmine/hook.hpp>
#include <jasmine/random.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

class $modify(MIMenuGameLayer, MenuGameLayer) {
    static void onModify(ModifyBase<ModifyDerive<MIMenuGameLayer, MenuGameLayer>>& self) {
        auto loader = Loader::get();
        auto hook = jasmine::hook::get(self.m_hooks, "MenuGameLayer::resetPlayer", true);
        if (auto knownPlayers = loader->getInstalledMod("iandyhd3.known_players")) {
            if (knownPlayers->isLoaded()) {
                hook->setAutoEnable(false);
            }
            else if (knownPlayers->shouldLoad()) {
                ModStateEvent(ModEventType::Loaded, knownPlayers).listen([hook] {
                    jasmine::hook::toggle(hook, false);
                    return ListenerResult::Propagate;
                }).leak();
            }
        }
        if (auto pityIcons = loader->getInstalledMod("kittenchilly.pity_title_screen_secret_icons")) {
            if (pityIcons->isLoaded()) {
                hook->setAutoEnable(false);
            }
            else if (pityIcons->shouldLoad()) {
                ModStateEvent(ModEventType::Loaded, pityIcons).listen([hook] {
                    jasmine::hook::toggle(hook, false);
                    return ListenerResult::Propagate;
                }).leak();
            }
        }
    }

    void resetPlayer() {
        MenuGameLayer::resetPlayer();

        auto object = m_playerObject;
        auto type = more_icons::getIconType(object);
        auto gameManager = Get::GameManager();

        if (auto icons = more_icons::getIcons(type)) {
            auto iconCount = gameManager->countForType(type);
            int icon = round(jasmine::random::get(1, iconCount + icons->size()));

            if (icon > iconCount) more_icons::updatePlayerObject(object, icons->data() + (icon - iconCount - 1));
            else if (object->m_isShip) object->updatePlayerShipFrame(icon);
            else if (object->m_isBall) object->updatePlayerRollFrame(icon);
            else if (object->m_isBird) object->updatePlayerBirdFrame(icon);
            else if (object->m_isDart) object->updatePlayerDartFrame(icon);
            else if (object->m_isRobot) object->updatePlayerRobotFrame(icon);
            else if (object->m_isSpider) object->updatePlayerSpiderFrame(icon);
            else if (object->m_isSwing) object->updatePlayerSwingFrame(icon);
            else object->updatePlayerFrame(icon);
        }

        if (object->m_isShip || object->m_isBird) {
            if (auto cubes = more_icons::getIcons(IconType::Cube)) {
                auto cubeCount = gameManager->countForType(IconType::Cube);
                int cube = round(jasmine::random::get(1, cubeCount + cubes->size()));

                if (cube > cubeCount) more_icons::updatePlayerObject(object, cubes->data() + (cube - cubeCount - 1));
                else object->updatePlayerFrame(cube);
            }
        }
    }
};

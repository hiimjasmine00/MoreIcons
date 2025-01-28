#include "../MoreIcons.hpp"
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(MIPlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        (void)self.setHookPriority("PlayerObject::setupStreak", -1);
    }

    bool init(int player, int ship, GJBaseGameLayer* gameLayer, CCLayer* layer, bool highGraphics) {
        if (!PlayerObject::init(player, ship, gameLayer, layer, highGraphics)) return false;

        if (!gameLayer || (gameLayer->m_player1 && gameLayer->m_player2)) return true;

        if (!gameLayer->m_player1) {
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Cube, false), IconType::Cube);
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Ship, false), IconType::Ship);
        }
        else if (!gameLayer->m_player2) {
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Cube, true), IconType::Cube);
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Ship, true), IconType::Ship);
        }

        return true;
    }

    void updatePlayerFrame(int frame) {
        PlayerObject::updatePlayerFrame(frame);

        if (!m_gameLayer || frame == 0) return MoreIconsAPI::removeUserObject(this);

        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Cube, false), IconType::Cube);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Cube, true), IconType::Cube);
        else MoreIconsAPI::removeUserObject(this);
    }

    void updatePlayerShipFrame(int frame) {
        PlayerObject::updatePlayerShipFrame(frame);

        if (!m_gameLayer) return MoreIconsAPI::removeUserObject(this);

        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Ship, false), IconType::Ship);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Ship, true), IconType::Ship);
        else MoreIconsAPI::removeUserObject(this);
    }

    void updatePlayerRollFrame(int frame) {
        PlayerObject::updatePlayerRollFrame(frame);

        if (!m_gameLayer || frame == 0) return MoreIconsAPI::removeUserObject(this);

        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Ball, false), IconType::Ball);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Ball, true), IconType::Ball);
        else MoreIconsAPI::removeUserObject(this);
    }

    void updatePlayerBirdFrame(int frame) {
        PlayerObject::updatePlayerBirdFrame(frame);

        if (!m_gameLayer) return MoreIconsAPI::removeUserObject(this);

        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Ufo, false), IconType::Ufo);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Ufo, true), IconType::Ufo);
        else MoreIconsAPI::removeUserObject(this);
    }

    void updatePlayerDartFrame(int frame) {
        PlayerObject::updatePlayerDartFrame(frame);

        if (!m_gameLayer) return MoreIconsAPI::removeUserObject(this);

        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Wave, false), IconType::Wave);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Wave, true), IconType::Wave);
        else MoreIconsAPI::removeUserObject(this);
    }

    void createRobot(int frame) {
        PlayerObject::createRobot(frame);

        if (!m_gameLayer) return MoreIconsAPI::removeUserObject(this);

        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Robot, false), IconType::Robot);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Robot, true), IconType::Robot);
        else MoreIconsAPI::removeUserObject(this);
    }

    void createSpider(int frame) {
        PlayerObject::createSpider(frame);

        if (!m_gameLayer) return MoreIconsAPI::removeUserObject(this);

        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Spider, false), IconType::Spider);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Spider, true), IconType::Spider);
        else MoreIconsAPI::removeUserObject(this);
    }

    void updatePlayerSwingFrame(int frame) {
        PlayerObject::updatePlayerSwingFrame(frame);

        if (!m_gameLayer) return MoreIconsAPI::removeUserObject(this);

        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Swing, false), IconType::Swing);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Swing, true), IconType::Swing);
        else MoreIconsAPI::removeUserObject(this);
    }

    void updatePlayerJetpackFrame(int frame) {
        PlayerObject::updatePlayerJetpackFrame(frame);

        if (!m_gameLayer) return MoreIconsAPI::removeUserObject(this);

        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Jetpack, false), IconType::Jetpack);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this)
            MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(IconType::Jetpack, true), IconType::Jetpack);
        else MoreIconsAPI::removeUserObject(this);
    }

    void setupStreak() {
        PlayerObject::setupStreak();

        if (!m_gameLayer) return;

        std::string trailFile;
        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this)
            trailFile = MoreIconsAPI::activeForType(IconType::Special, false);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this)
            trailFile = MoreIconsAPI::activeForType(IconType::Special, true);
        else return;

        if (trailFile.empty() || !MoreIconsAPI::hasIcon(trailFile, IconType::Special)) return;

        auto trailInfo = MoreIcons::TRAIL_INFO[trailFile];
        m_streakRelated1 = 14.0f;
        m_streakRelated2 = !trailInfo.tint;
        m_streakRelated3 = false;
        m_regularTrail->initWithFade(0.3f, 5.0f, 14.0f, { 255, 255, 255 }, CCTextureCache::get()->textureForKey(trailInfo.texture.c_str()));
        if (trailInfo.blend) m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
    }
};

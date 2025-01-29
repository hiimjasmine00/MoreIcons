#include "../MoreIcons.hpp"
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(MIPlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        (void)self.setHookPriority("PlayerObject::setupStreak", -1);
    }

    bool isMainPlayer() {
        return m_gameLayer && ((!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this) || (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this));
    }

    void updateIcon(IconType type) {
        if (!isMainPlayer()) return MoreIconsAPI::removeUserObject(this);

        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this) MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(type, false), type);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this) MoreIconsAPI::updatePlayerObject(this, MoreIconsAPI::activeForType(type, true), type);
    }

    bool init(int player, int ship, GJBaseGameLayer* gameLayer, CCLayer* layer, bool highGraphics) {
        if (!PlayerObject::init(player, ship, gameLayer, layer, highGraphics)) return false;

        updateIcon(IconType::Cube);
        updateIcon(IconType::Ship);

        return true;
    }

    void updatePlayerFrame(int frame) {
        PlayerObject::updatePlayerFrame(frame);

        if (frame == 0) return MoreIconsAPI::removeUserObject(this);
        
        updateIcon(IconType::Cube);
    }

    void updatePlayerShipFrame(int frame) {
        PlayerObject::updatePlayerShipFrame(frame);
        
        updateIcon(IconType::Ship);
    }

    void updatePlayerRollFrame(int frame) {
        PlayerObject::updatePlayerRollFrame(frame);

        if (frame == 0) return MoreIconsAPI::removeUserObject(this);
        
        updateIcon(IconType::Ball);
    }

    void updatePlayerBirdFrame(int frame) {
        PlayerObject::updatePlayerBirdFrame(frame);
        
        updateIcon(IconType::Ufo);
    }

    void updatePlayerDartFrame(int frame) {
        PlayerObject::updatePlayerDartFrame(frame);
        
        updateIcon(IconType::Wave);
    }

    void createRobot(int frame) {
        PlayerObject::createRobot(frame);
        
        updateIcon(IconType::Robot);
    }

    void createSpider(int frame) {
        PlayerObject::createSpider(frame);
        
        updateIcon(IconType::Spider);
    }

    void updatePlayerSwingFrame(int frame) {
        PlayerObject::updatePlayerSwingFrame(frame);
        
        updateIcon(IconType::Swing);
    }

    void updatePlayerJetpackFrame(int frame) {
        PlayerObject::updatePlayerJetpackFrame(frame);
        
        updateIcon(IconType::Jetpack);
    }

    void setupStreak() {
        PlayerObject::setupStreak();

        if (!isMainPlayer()) return;

        std::string trailFile;
        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this)
            trailFile = MoreIconsAPI::activeForType(IconType::Special, false);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this)
            trailFile = MoreIconsAPI::activeForType(IconType::Special, true);

        if (trailFile.empty() || !MoreIconsAPI::hasIcon(trailFile, IconType::Special)) return;

        auto trailInfo = MoreIcons::TRAIL_INFO[trailFile];
        m_streakRelated1 = 14.0f;
        m_streakRelated2 = !trailInfo.tint;
        m_streakRelated3 = false;
        m_regularTrail->initWithFade(0.3f, 5.0f, 14.0f, { 255, 255, 255 }, CCTextureCache::get()->textureForKey(trailInfo.texture.c_str()));
        if (trailInfo.blend) m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
    }
};

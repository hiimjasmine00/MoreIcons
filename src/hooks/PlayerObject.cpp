#include "../MoreIcons.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(MIPlayerObject, PlayerObject) {
    static void onModify(ModifyBase<ModifyDerive<MIPlayerObject, PlayerObject>>& self) {
        (void)self.setHookPriorityAfterPost("PlayerObject::setupStreak", "weebify.separate_dual_icons");
    }

    bool isMainPlayer() {
        return m_gameLayer && ((!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this) || (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this));
    }

    void updateIcon(IconType type) {
        if (!isMainPlayer()) return setUserObject("name"_spr, nullptr);

        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this) MoreIconsAPI::updatePlayerObject(this, type, false);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this) MoreIconsAPI::updatePlayerObject(this, type, true);
    }

    bool init(int player, int ship, GJBaseGameLayer* gameLayer, CCLayer* layer, bool highGraphics) {
        if (!PlayerObject::init(player, ship, gameLayer, layer, highGraphics)) return false;

        updateIcon(IconType::Cube);
        updateIcon(IconType::Ship);

        return true;
    }

    void updatePlayerFrame(int frame) {
        PlayerObject::updatePlayerFrame(frame);

        if (frame == 0) return setUserObject("name"_spr, nullptr);

        updateIcon(IconType::Cube);
    }

    void updatePlayerShipFrame(int frame) {
        PlayerObject::updatePlayerShipFrame(frame);

        updateIcon(IconType::Ship);
    }

    void updatePlayerRollFrame(int frame) {
        PlayerObject::updatePlayerRollFrame(frame);

        if (frame == 0) return setUserObject("name"_spr, nullptr);

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

    void resetTrail() {
        if (m_regularTrail->getUserObject("name"_spr)) m_regularTrail->setUserObject("name"_spr, nullptr);
        if (!MoreIcons::TRADITIONAL_PACKS || (Loader::get()->isModLoaded("acaruso.pride") && m_playerStreak == 2)) return;
        m_regularTrail->setTexture(CCTextureCache::get()->addImage(MoreIcons::vanillaTexturePath(
            fmt::format("streak_{:02}_001.png", m_playerStreak), true).string().c_str(), false));
        if (m_playerStreak == 6) m_regularTrail->enableRepeatMode(0.1f);
    }

    void setupStreak() {
        PlayerObject::setupStreak();

        if (!isMainPlayer()) return resetTrail();

        std::string trailFile;
        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this) trailFile = MoreIconsAPI::activeForType(IconType::Special, false);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this) trailFile = MoreIconsAPI::activeForType(IconType::Special, true);

        if (trailFile.empty() || !MoreIconsAPI::hasIcon(trailFile, IconType::Special)) return resetTrail();

        auto textureCache = CCTextureCache::get();
        auto trailInfo = MoreIcons::TRAIL_INFO[trailFile];
        if (trailInfo.trailID > 0) {
            m_streakStrokeWidth = 10.0f;
            auto fade = 0.3f;
            auto stroke = 10.0f;
            switch (trailInfo.trailID) {
                case 2:
                case 7:
                    stroke = 14.0f;
                    m_disableStreakTint = true;
                    m_streakStrokeWidth = 14.0f;
                    break;
                case 3:
                    m_streakStrokeWidth = 8.5f;
                    stroke = 8.5f;
                    break;
                case 4:
                    fade = 0.4f;
                    stroke = 10.0f;
                    break;
                case 5:
                    m_streakStrokeWidth = 5.0f;
                    fade = 0.6f;
                    m_alwaysShowStreak = true;
                    stroke = 5.0f;
                    break;
                case 6:
                    fade = 1.0f;
                    m_alwaysShowStreak = true;
                    m_streakStrokeWidth = 3.0f;
                    stroke = 3.0f;
                    break;
            }

            m_regularTrail->initWithFade(fade, 5.0f, stroke, { 255, 255, 255 }, textureCache->textureForKey(trailInfo.texture.c_str()));
            if (trailInfo.trailID == 6) m_regularTrail->enableRepeatMode(0.1f);
            m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
            m_regularTrail->setUserObject("name"_spr, CCString::create(trailFile));
        }
        m_streakStrokeWidth = 14.0f;
        m_disableStreakTint = !trailInfo.tint;
        m_alwaysShowStreak = false;
        m_regularTrail->initWithFade(0.3f, 5.0f, 14.0f, { 255, 255, 255 }, textureCache->textureForKey(trailInfo.texture.c_str()));
        if (trailInfo.blend) m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        m_regularTrail->setUserObject("name"_spr, CCString::create(trailFile));
    }
};

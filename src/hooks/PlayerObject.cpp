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

    void resetTrail() {
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        auto trailID = sdi && m_gameLayer && m_gameLayer->m_player2 == this
            ? sdi->getSavedValue<int>("trail", 1) : GameManager::get()->m_playerStreak.value();
        if (Loader::get()->isModLoaded("acaruso.pride") && trailID == 2) return;
        m_regularTrail->setTexture(CCTextureCache::sharedTextureCache()->addImage(
            MoreIcons::vanillaTexturePath(fmt::format("streak_{:02}_001.png", trailID), true).c_str(), false));
    }

    void setupStreak() {
        PlayerObject::setupStreak();

        if (!isMainPlayer()) return resetTrail();

        std::string trailFile;
        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this)
            trailFile = MoreIconsAPI::activeForType(IconType::Special, false);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this)
            trailFile = MoreIconsAPI::activeForType(IconType::Special, true);

        if (trailFile.empty() || !MoreIconsAPI::hasIcon(trailFile, IconType::Special)) return resetTrail();

        auto textureCache = CCTextureCache::sharedTextureCache();
        auto trailInfo = MoreIcons::TRAIL_INFO[trailFile];
        if (trailInfo.trailID > 0) {
            m_streakRelated1 = 10.0f;
            auto fade = 0.3f;
            auto stroke = 10.0f;
            switch (trailInfo.trailID) {
                case 2:
                case 7:
                    stroke = 14.0f;
                    m_streakRelated2 = true;
                    m_streakRelated1 = 14.0f;
                    break;
                case 3:
                    m_streakRelated1 = 8.5f;
                    stroke = 8.5f;
                    break;
                case 4:
                    fade = 0.4f;
                    stroke = 10.0f;
                    break;
                case 5:
                    m_streakRelated1 = 5.0f;
                    fade = 0.6f;
                    m_streakRelated3 = true;
                    stroke = 5.0f;
                    break;
                case 6:
                    fade = 1.0f;
                    m_streakRelated3 = true;
                    m_streakRelated1 = 3.0f;
                    stroke = 3.0f;
                    break;
            }

            m_regularTrail->initWithFade(fade, 5.0f, stroke, { 255, 255, 255 }, textureCache->textureForKey(trailInfo.texture.c_str()));
            m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        }
        m_streakRelated1 = 14.0f;
        m_streakRelated2 = !trailInfo.tint;
        m_streakRelated3 = false;
        m_regularTrail->initWithFade(0.3f, 5.0f, 14.0f, { 255, 255, 255 }, textureCache->textureForKey(trailInfo.texture.c_str()));
        if (trailInfo.blend) m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
    }
};

#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

using UpdateFunc = void(PlayerObject::*)(int);

class $modify(MIPlayerObject, PlayerObject) {
    static void onModify(ModifyBase<ModifyDerive<MIPlayerObject, PlayerObject>>& self) {
        (void)self.setHookPriorityAfterPost("PlayerObject::setupStreak", "weebify.separate_dual_icons");
    }

    bool p1() {
        return m_gameLayer && (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this);
    }

    bool p2() {
        return m_gameLayer && (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this);
    }

    void updateIcon(IconType type, bool dual) {
        auto icon = MoreIconsAPI::activeIcon(type, dual);
        if (!icon.empty()) MoreIconsAPI::updatePlayerObject(this, icon, type);
        else setUserObject("name"_spr, nullptr);
    }

    bool init(int player, int ship, GJBaseGameLayer* gameLayer, CCLayer* layer, bool playLayer) {
        if (!PlayerObject::init(player, ship, gameLayer, layer, playLayer)) return false;

        if (p1()) {
            updateIcon(IconType::Cube, false);
            updateIcon(IconType::Ship, false);
        }
        else if (p2()) {
            updateIcon(IconType::Cube, true);
            updateIcon(IconType::Ship, true);
        }

        return true;
    }

    void updateIcon(int frame, IconType type, UpdateFunc func) {
        auto player1 = p1();
        auto player2 = p2();
        auto mainPlayer = frame != 0 && (player1 || player2);
        std::string iconName;
        if (mainPlayer && !MoreIconsAPI::preloadIcons) {
            if (auto foundRequests = MoreIconsAPI::requestedIcons.find(m_iconRequestID); foundRequests != MoreIconsAPI::requestedIcons.end()) {
                auto& iconRequests = foundRequests->second;
                if (auto found = iconRequests.find(type); found != iconRequests.end()) iconName = found->second;
            }
        }

        if (!iconName.empty()) MoreIconsAPI::loadedIcons[{ iconName, type }]++;
        (this->*func)(frame);
        if (!mainPlayer) return setUserObject("name"_spr, nullptr);
        if (player1) updateIcon(type, false);
        else if (player2) updateIcon(type, true);
        if (!iconName.empty()) MoreIconsAPI::loadedIcons[{ iconName, type }]--;
    }

    void updatePlayerFrame(int frame) {
        updateIcon(frame, IconType::Cube, &PlayerObject::updatePlayerFrame);
    }

    void updatePlayerShipFrame(int frame) {
        updateIcon(frame, IconType::Ship, &PlayerObject::updatePlayerShipFrame);
    }

    void updatePlayerRollFrame(int frame) {
        updateIcon(frame, IconType::Ball, &PlayerObject::updatePlayerRollFrame);
    }

    void updatePlayerBirdFrame(int frame) {
        updateIcon(frame, IconType::Ufo, &PlayerObject::updatePlayerBirdFrame);
    }

    void updatePlayerDartFrame(int frame) {
        updateIcon(frame, IconType::Wave, &PlayerObject::updatePlayerDartFrame);
    }

    void createRobot(int frame) {
        updateIcon(frame, IconType::Robot, &PlayerObject::createRobot);
    }

    void createSpider(int frame) {
        updateIcon(frame, IconType::Spider, &PlayerObject::createSpider);
    }

    void updatePlayerSwingFrame(int frame) {
        updateIcon(frame, IconType::Swing, &PlayerObject::updatePlayerSwingFrame);
    }

    void updatePlayerJetpackFrame(int frame) {
        updateIcon(frame, IconType::Jetpack, &PlayerObject::updatePlayerJetpackFrame);
    }

    void resetTrail() {
        m_regularTrail->setUserObject("name"_spr, nullptr);
        if (!MoreIcons::traditionalPacks || (Loader::get()->isModLoaded("acaruso.pride") && m_playerStreak == 2)) return;
        m_regularTrail->setTexture(MoreIconsAPI::get<CCTextureCache>()->addImage(
            MoreIcons::vanillaTexturePath(fmt::format("streak_{:02}_001.png", m_playerStreak), true).c_str(), false));
        if (m_playerStreak == 6) m_regularTrail->enableRepeatMode(0.1f);
    }

    void setupStreak() {
        PlayerObject::setupStreak();

        auto info = p1() ? MoreIconsAPI::getIcon(IconType::Special, false) : p2() ? MoreIconsAPI::getIcon(IconType::Special, true) : nullptr;
        if (!info) return resetTrail();

        m_streakStrokeWidth = info->trailInfo.stroke;
        m_disableStreakTint = !info->trailInfo.tint;
        m_alwaysShowStreak = info->trailInfo.show;

        m_regularTrail->initWithFade(info->trailInfo.fade, 5.0f, info->trailInfo.stroke, { 255, 255, 255 }, info->textures[0].c_str());
        if (info->trailID == 6) m_regularTrail->enableRepeatMode(0.1f);
        if (info->trailInfo.blend) m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        m_regularTrail->setUserObject("name"_spr, CCString::create(info->name));
    }

    void updateStreakBlend(bool blend) {
        PlayerObject::updateStreakBlend(blend);

        if (auto info = p1() ? MoreIconsAPI::getIcon(IconType::Special, false) : p2() ? MoreIconsAPI::getIcon(IconType::Special, true) : nullptr)
            m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, (uint32_t)GL_ONE_MINUS_SRC_ALPHA - info->trailInfo.blend * (uint32_t)GL_SRC_ALPHA });
    }
};

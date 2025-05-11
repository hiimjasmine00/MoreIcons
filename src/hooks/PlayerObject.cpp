#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

#define UPDATE_HOOK(funcName, type) \
    void funcName(int frame) { \
        std::string iconName; \
        if (frame != 0 && isMainPlayer() && MoreIconsAPI::requestedIcons.contains(m_iconRequestID)) { \
            auto& iconRequests = MoreIconsAPI::requestedIcons[m_iconRequestID]; \
            if (iconRequests.contains(type)) iconName = iconRequests[type]; \
        } \
        if (!iconName.empty()) MoreIconsAPI::loadedIcons[{ iconName, type }]++; \
        PlayerObject::funcName(frame); \
        if (frame == 0 || !isMainPlayer()) return setUserObject("name"_spr, nullptr); \
        updateIcon(type); \
        if (!iconName.empty()) MoreIconsAPI::loadedIcons[{ iconName, type }]--; \
    }

class $modify(MIPlayerObject, PlayerObject) {
    static void onModify(ModifyBase<ModifyDerive<MIPlayerObject, PlayerObject>>& self) {
        (void)self.setHookPriorityAfterPost("PlayerObject::setupStreak", "weebify.separate_dual_icons");
    }

    bool isMainPlayer() {
        return m_gameLayer &&
            ((!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this) ||
            (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this));
    }

    void updateIcon(IconType type) {
        std::string icon;
        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this) icon = _MoreIcons::activeIcon(type, false);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this) icon = _MoreIcons::activeIcon(type, true);

        if (icon.empty() || !MoreIconsAPI::hasIcon(icon, type)) return setUserObject("name"_spr, nullptr);

        MoreIconsAPI::updatePlayerObject(this, icon, type);
    }

    bool init(int player, int ship, GJBaseGameLayer* gameLayer, CCLayer* layer, bool ignoreDamage) {
        if (!PlayerObject::init(player, ship, gameLayer, layer, ignoreDamage)) return false;

        if (!isMainPlayer()) return true;

        updateIcon(IconType::Cube);
        updateIcon(IconType::Ship);

        return true;
    }

    UPDATE_HOOK(updatePlayerFrame, IconType::Cube)
    UPDATE_HOOK(updatePlayerShipFrame, IconType::Ship)
    UPDATE_HOOK(updatePlayerRollFrame, IconType::Ball)
    UPDATE_HOOK(updatePlayerBirdFrame, IconType::Ufo)
    UPDATE_HOOK(updatePlayerDartFrame, IconType::Wave)
    UPDATE_HOOK(createRobot, IconType::Robot)
    UPDATE_HOOK(createSpider, IconType::Spider)
    UPDATE_HOOK(updatePlayerSwingFrame, IconType::Swing)
    UPDATE_HOOK(updatePlayerJetpackFrame, IconType::Jetpack)

    void resetTrail() {
        if (m_regularTrail->getUserObject("name"_spr)) m_regularTrail->setUserObject("name"_spr, nullptr);
        if (!MoreIcons::traditionalPacks || (Loader::get()->isModLoaded("acaruso.pride") && m_playerStreak == 2)) return;
        m_regularTrail->setTexture(CCTextureCache::get()->addImage(
            MoreIcons::vanillaTexturePath(fmt::format("streak_{:02}_001.png", m_playerStreak), true).c_str(), false));
        if (m_playerStreak == 6) m_regularTrail->enableRepeatMode(0.1f);
    }

    void setupStreak() {
        PlayerObject::setupStreak();

        if (!isMainPlayer()) return resetTrail();

        std::string trailFile;
        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this) trailFile = _MoreIcons::activeIcon(IconType::Special, false);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this) trailFile = _MoreIcons::activeIcon(IconType::Special, true);

        if (trailFile.empty() || !MoreIconsAPI::hasIcon(trailFile, IconType::Special)) return resetTrail();

        auto info = MoreIconsAPI::getIcon(trailFile, IconType::Special);
        if (!info) return resetTrail();

        auto fade = 0.3f;
        auto stroke = 10.0f;
        if (info->trailID > 0) switch (info->trailID) {
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
            default:
                m_streakStrokeWidth = 10.0f;
                break;
        } else {
            fade = info->fade;
            stroke = info->stroke;
            m_streakStrokeWidth = stroke;
            m_disableStreakTint = !info->tint;
            m_alwaysShowStreak = info->show;
        }

        m_regularTrail->initWithFade(fade, 5.0f, stroke, { 255, 255, 255 }, info->textures[0].c_str());
        if (info->trailID == 6) m_regularTrail->enableRepeatMode(0.1f);
        if (info->trailID > 0 || info->blend) m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        m_regularTrail->setUserObject("name"_spr, CCString::create(trailFile));
    }

    void updateStreakBlend(bool blend) {
        PlayerObject::updateStreakBlend(blend);

        if (!isMainPlayer()) return;

        std::string trailFile;
        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this) trailFile = _MoreIcons::activeIcon(IconType::Special, false);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this) trailFile = _MoreIcons::activeIcon(IconType::Special, true);

        if (trailFile.empty() || !MoreIconsAPI::hasIcon(trailFile, IconType::Special)) return;

        auto info = MoreIconsAPI::getIcon(trailFile, IconType::Special);
        if (!info || info->trailID > 0) return;

        m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, (uint32_t)GL_ONE_MINUS_SRC_ALPHA - info->blend * (uint32_t)GL_SRC_ALPHA });
    }
};

#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

#define UPDATE_HOOK(funcName, type) \
    void funcName(int frame) { \
        PlayerObject::funcName(frame); \
        if (frame == 0) return resetPlayer(!isMainPlayer()); \
        updateIcon(type); \
    }

class $modify(MIPlayerObject, PlayerObject) {
    struct Fields {
        std::set<std::pair<std::string, IconType>> m_icons;
        ~Fields() {
            for (auto& [name, type] : m_icons) MoreIconsAPI::unloadIcon(name, type);
        }
    };

    static void onModify(ModifyBase<ModifyDerive<MIPlayerObject, PlayerObject>>& self) {
        (void)self.setHookPriorityAfterPost("PlayerObject::setupStreak", "weebify.separate_dual_icons");
    }

    bool isMainPlayer() {
        return m_gameLayer && ((!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this) || (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this));
    }

    void resetPlayer(bool unload) {
        if (auto name = static_cast<CCString*>(getUserObject("name"_spr))) {
            if (auto type = static_cast<CCInteger*>(getUserObject("type"_spr))) {
                if (unload) MoreIconsAPI::unloadIcon(name->m_sString, (IconType)type->m_nValue);
                setUserObject("type"_spr, nullptr);
            }
            setUserObject("name"_spr, nullptr);
        }
    }

    void updateIcon(IconType type) {
        if (!isMainPlayer()) return resetPlayer(true);

        std::string icon;
        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this) icon = MoreIconsClass::activeIcon(type, false);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this) icon = MoreIconsClass::activeIcon(type, true);

        if (icon.empty() || !MoreIconsAPI::hasIcon(icon, type)) return resetPlayer(true);

        m_fields->m_icons.insert({ icon, type });
        MoreIconsAPI::updatePlayerObject(this, icon, type);
    }

    bool init(int player, int ship, GJBaseGameLayer* gameLayer, CCLayer* layer, bool ignoreDamage) {
        if (!PlayerObject::init(player, ship, gameLayer, layer, ignoreDamage)) return false;

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
        if (!MoreIcons::TRADITIONAL_PACKS || (Loader::get()->isModLoaded("acaruso.pride") && m_playerStreak == 2)) return;
        m_regularTrail->setTexture(CCTextureCache::get()->addImage(
            MoreIcons::vanillaTexturePath(fmt::format("streak_{:02}_001.png", m_playerStreak), true).c_str(), false));
        if (m_playerStreak == 6) m_regularTrail->enableRepeatMode(0.1f);
    }

    void setupStreak() {
        PlayerObject::setupStreak();

        if (!isMainPlayer()) return resetTrail();

        std::string trailFile;
        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this) trailFile = MoreIconsClass::activeIcon(IconType::Special, false);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this) trailFile = MoreIconsClass::activeIcon(IconType::Special, true);

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
            m_streakStrokeWidth = 14.0f;
            m_disableStreakTint = !info->tint;
            m_alwaysShowStreak = false;
            fade = 0.3f;
            stroke = 14.0f;
        }

        m_regularTrail->initWithFade(fade, 5.0f, stroke, { 255, 255, 255 }, CCTextureCache::get()->addImage(info->textures[0].c_str(), true));
        if (info->trailID == 6) m_regularTrail->enableRepeatMode(0.1f);
        if (info->trailID > 0 || info->blend) m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        m_regularTrail->setUserObject("name"_spr, CCString::create(trailFile));
    }
};

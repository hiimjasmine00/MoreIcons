#include "../MoreIcons.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(MIPlayerObject, PlayerObject) {
    struct Fields {
        std::vector<std::pair<std::string, IconType>> m_icons;
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
        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this) icon = MoreIconsAPI::activeForType(type, false);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this) icon = MoreIconsAPI::activeForType(type, true);

        if (!icon.empty() && MoreIconsAPI::hasIcon(icon, type)) m_fields->m_icons.emplace_back(icon, type);

        MoreIconsAPI::updatePlayerObject(this, icon, type);
    }

    bool init(int player, int ship, GJBaseGameLayer* gameLayer, CCLayer* layer, bool highGraphics) {
        if (!PlayerObject::init(player, ship, gameLayer, layer, highGraphics)) return false;

        updateIcon(IconType::Cube);
        updateIcon(IconType::Ship);

        return true;
    }

    void updatePlayerFrame(int frame) {
        PlayerObject::updatePlayerFrame(frame);

        if (frame == 0) return resetPlayer(!isMainPlayer());

        updateIcon(IconType::Cube);
    }

    void updatePlayerShipFrame(int frame) {
        PlayerObject::updatePlayerShipFrame(frame);

        updateIcon(IconType::Ship);
    }

    void updatePlayerRollFrame(int frame) {
        PlayerObject::updatePlayerRollFrame(frame);

        if (frame == 0) return resetPlayer(!isMainPlayer());

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
        m_regularTrail->setTexture(CCTextureCache::get()->addImage(
            MoreIcons::vanillaTexturePath(fmt::format("streak_{:02}_001.png", m_playerStreak), true).c_str(), false));
        if (m_playerStreak == 6) m_regularTrail->enableRepeatMode(0.1f);
    }

    void setupStreak() {
        PlayerObject::setupStreak();

        if (!isMainPlayer()) return resetTrail();

        std::string trailFile;
        if (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this) trailFile = MoreIconsAPI::activeForType(IconType::Special, false);
        else if (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this) trailFile = MoreIconsAPI::activeForType(IconType::Special, true);

        if (trailFile.empty() || !MoreIconsAPI::hasIcon(trailFile, IconType::Special)) return resetTrail();

        auto found = MoreIconsAPI::infoForIcon(trailFile, IconType::Special);
        if (!found.has_value()) return resetTrail();

        auto fade = 0.3f;
        auto stroke = 10.0f;
        auto& trailInfo = found.value();
        if (trailInfo.trailID > 0) switch (trailInfo.trailID) {
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
            m_disableStreakTint = !trailInfo.tint;
            m_alwaysShowStreak = false;
            fade = 0.3f;
            stroke = 14.0f;
        }

        m_regularTrail->initWithFade(fade, 5.0f, stroke, { 255, 255, 255 }, CCTextureCache::get()->addImage(trailInfo.textures[0].c_str(), true));
        if (trailInfo.trailID == 6) m_regularTrail->enableRepeatMode(0.1f);
        if (trailInfo.trailID > 0 || trailInfo.blend) m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        m_regularTrail->setUserObject("name"_spr, CCString::create(trailFile));
    }
};

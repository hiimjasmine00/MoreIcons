#include "../MoreIcons.hpp"
#include "../api/MoreIconsAPI.hpp"
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(MIPlayerObject, PlayerObject) {
    struct Fields {
        bool m_player1 = false;
        bool m_player2 = false;
    };

    static void onModify(ModifyBase<ModifyDerive<MIPlayerObject, PlayerObject>>& self) {
        (void)self.setHookPriorityAfterPost("PlayerObject::setupStreak", "weebify.separate_dual_icons");
    }

    void updateIcon(IconType type) {
        std::string icon;
        auto f = m_fields.self();
        if (f->m_player1) icon = MoreIconsAPI::activeIcon(type, false);
        else if (f->m_player2) icon = MoreIconsAPI::activeIcon(type, true);
        if (!icon.empty()) MoreIconsAPI::updatePlayerObject(this, icon, type);
        else setUserObject("name"_spr, nullptr);
    }

    bool init(int player, int ship, GJBaseGameLayer* gameLayer, CCLayer* layer, bool playLayer) {
        if (!PlayerObject::init(player, ship, gameLayer, layer, playLayer)) return false;

        if (!gameLayer) return true;

        auto f = m_fields.self();
        if (!gameLayer->m_player1 || gameLayer->m_player1 == this) f->m_player1 = true;
        else if (!gameLayer->m_player2 || gameLayer->m_player2 == this) f->m_player2 = true;

        if (f->m_player1 || f->m_player2) {
            updateIcon(IconType::Cube);
            updateIcon(IconType::Ship);
        }

        return true;
    }

    void updateIcon(int frame, IconType type, void(PlayerObject::*func)(int)) {
        auto f = m_fields.self();
        if (frame == 0 || (!f->m_player1 && !f->m_player2)) {
            (this->*func)(frame);
            return setUserObject("name"_spr, nullptr);
        }

        int* loadedIcon = nullptr;
        if (!MoreIconsAPI::preloadIcons) {
            if (auto foundRequests = MoreIconsAPI::requestedIcons.find(m_iconRequestID); foundRequests != MoreIconsAPI::requestedIcons.end()) {
                auto& iconRequests = foundRequests->second;
                if (auto found = iconRequests.find(type); found != iconRequests.end()) {
                    loadedIcon = &MoreIconsAPI::loadedIcons[{ found->second, type }];
                }
            }
        }

        if (loadedIcon) (*loadedIcon)++;
        (this->*func)(frame);
        updateIcon(type);
        if (loadedIcon) (*loadedIcon)--;
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

    void toggleRobotMode(bool enable, bool noEffects) {
        auto isRobot = m_isRobot;
        PlayerObject::toggleRobotMode(enable, noEffects);

        if (!isRobot && m_isRobot) {
            auto f = m_fields.self();
            if (!f->m_player1 && !f->m_player2) return;
            std::string iconName;
            if (f->m_player1) iconName = MoreIconsAPI::activeIcon(IconType::Robot, false);
            else if (f->m_player2) iconName = MoreIconsAPI::activeIcon(IconType::Robot, true);
            if (!iconName.empty()) m_iconSprite->setDisplayFrame(MoreIconsAPI::getFrame(fmt::format("{}_01_001.png"_spr, iconName)));
        }
    }

    void toggleSpiderMode(bool enable, bool noEffects) {
        auto isSpider = m_isSpider;
        PlayerObject::toggleSpiderMode(enable, noEffects);

        if (!isSpider && m_isSpider) {
            auto f = m_fields.self();
            if (!f->m_player1 && !f->m_player2) return;
            std::string iconName;
            if (f->m_player1) iconName = MoreIconsAPI::activeIcon(IconType::Spider, false);
            else if (f->m_player2) iconName = MoreIconsAPI::activeIcon(IconType::Spider, true);
            if (!iconName.empty()) m_iconSprite->setDisplayFrame(MoreIconsAPI::getFrame(fmt::format("{}_01_001.png"_spr, iconName)));
        }
    }

    void resetTrail() {
        m_regularTrail->setUserObject("name"_spr, nullptr);
        if (!MoreIcons::traditionalPacks || (Loader::get()->isModLoaded("acaruso.pride") && m_playerStreak == 2)) return;
        m_regularTrail->setTexture(MoreIconsAPI::getTextureCache()->addImage(
            MoreIcons::vanillaTexturePath(fmt::format("streak_{:02}_001.png", m_playerStreak), true).c_str(), false));
        if (m_playerStreak == 6) m_regularTrail->enableRepeatMode(0.1f);
    }

    IconInfo* getTrailInfo() {
        auto f = m_fields.self();
        if (f->m_player1) return MoreIconsAPI::getIcon(IconType::Special, false);
        else if (f->m_player2) return MoreIconsAPI::getIcon(IconType::Special, true);
        else return nullptr;
    }

    void setupStreak() {
        PlayerObject::setupStreak();

        auto info = getTrailInfo();
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

        if (auto info = getTrailInfo()) {
            m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, (uint32_t)(info->trailInfo.blend ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA) });
        }
    }
};

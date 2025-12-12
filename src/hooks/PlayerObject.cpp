#include "../MoreIcons.hpp"
#include "../utils/Defaults.hpp"
#include "../utils/Get.hpp"
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

class $modify(MIPlayerObject, PlayerObject) {
    static void onModify(ModifyBase<ModifyDerive<MIPlayerObject, PlayerObject>>& self) {
        (void)self.setHookPriorityAfterPost("PlayerObject::setupStreak", "weebify.separate_dual_icons");
    }

    bool isPlayer1() {
        return m_gameLayer && (!m_gameLayer->m_player1 || m_gameLayer->m_player1 == this);
    }

    bool isPlayer2() {
        return m_gameLayer && (!m_gameLayer->m_player2 || m_gameLayer->m_player2 == this);
    }

    void updateIcon(IconType type) {
        std::string icon;
        if (isPlayer1()) icon = more_icons::activeIcon(type, false);
        else if (isPlayer2()) icon = more_icons::activeIcon(type, true);
        if (!icon.empty()) more_icons::updatePlayerObject(this, icon, type);
        else setUserObject("name"_spr, nullptr);
    }

    bool init(int player, int ship, GJBaseGameLayer* gameLayer, CCLayer* layer, bool playLayer) {
        if (!PlayerObject::init(player, ship, gameLayer, layer, playLayer)) return false;

        if (isPlayer1() || isPlayer2()) {
            updateIcon(IconType::Cube);
            updateIcon(IconType::Ship);
        }

        return true;
    }

    void updateIcon(int frame, IconType type, void(PlayerObject::*func)(int)) {
        if (frame == 0 || (!isPlayer1() && !isPlayer2())) {
            (this->*func)(frame);
            return setUserObject("name"_spr, nullptr);
        }

        int* loadedIcon = nullptr;
        if (!MoreIcons::preloadIcons) {
            if (auto foundRequests = MoreIcons::requestedIcons.find(m_iconRequestID); foundRequests != MoreIcons::requestedIcons.end()) {
                auto& iconRequests = foundRequests->second;
                if (auto found = iconRequests.find(type); found != iconRequests.end()) {
                    loadedIcon = &MoreIcons::loadedIcons[{ found->second, type }];
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
            std::string iconName;
            if (isPlayer1()) iconName = more_icons::activeIcon(IconType::Robot, false);
            else if (isPlayer2()) iconName = more_icons::activeIcon(IconType::Robot, true);
            if (!iconName.empty()) m_iconSprite->setDisplayFrame(MoreIcons::getFrame("{}_01_001.png"_spr, iconName));
        }
    }

    void toggleSpiderMode(bool enable, bool noEffects) {
        auto isSpider = m_isSpider;
        PlayerObject::toggleSpiderMode(enable, noEffects);

        if (!isSpider && m_isSpider) {
            std::string iconName;
            if (isPlayer1()) iconName = more_icons::activeIcon(IconType::Spider, false);
            else if (isPlayer2()) iconName = more_icons::activeIcon(IconType::Spider, true);
            if (!iconName.empty()) m_iconSprite->setDisplayFrame(MoreIcons::getFrame("{}_01_001.png"_spr, iconName));
        }
    }

    IconInfo* getTrailInfo(IconType type) {
        if (isPlayer1()) return more_icons::getIcon(type, false);
        else if (isPlayer2()) return more_icons::getIcon(type, true);
        else return nullptr;
    }

    void setupStreak() {
        PlayerObject::setupStreak();

        if (auto info = getTrailInfo(IconType::Special)) {
            auto trailInfo = info->getSpecialInfo();
            auto blend = trailInfo.get<bool>("blend").unwrapOr(false);
            auto tint = trailInfo.get<bool>("tint").unwrapOr(false);
            auto show = trailInfo.get<bool>("show").unwrapOr(false);
            auto fade = trailInfo.get<float>("fade").unwrapOr(0.3f);
            auto stroke = trailInfo.get<float>("stroke").unwrapOr(14.0f);

            m_streakStrokeWidth = stroke;
            m_disableStreakTint = !tint;
            m_alwaysShowStreak = show;

            m_regularTrail->updateFade(fade);
            m_regularTrail->setStroke(stroke);
            m_regularTrail->setTexture(Get::TextureCache()->addImage(info->getTextureString().c_str(), false));
            if (info->getSpecialID() == 6) m_regularTrail->enableRepeatMode(0.1f);
            if (blend) m_regularTrail->setBlendFunc({ GL_SRC_ALPHA, (uint32_t)(blend ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA) });
            m_regularTrail->setUserObject("name"_spr, CCString::create(info->getName()));
        }
        else {
            m_regularTrail->setUserObject("name"_spr, nullptr);
            if (MoreIcons::traditionalPacks && (!Loader::get()->isModLoaded("acaruso.pride") || m_playerStreak != 2)) {
                m_regularTrail->setTexture(Get::TextureCache()->addImage(MoreIcons::vanillaTexturePath(
                    fmt::format("streak_{:02}_001.png", m_playerStreak), true
                ).c_str(), false));
                if (m_playerStreak == 6) m_regularTrail->enableRepeatMode(0.1f);
            }
        }

        if (auto info = getTrailInfo(IconType::ShipFire)) {
            auto fireInfo = info->getSpecialInfo();
            auto fade = fireInfo.get<float>("fade").unwrapOr(0.1f);
            auto stroke = fireInfo.get<float>("stroke").unwrapOr(20.0f);
            auto texture = Get::TextureCache()->addImage(info->getTextureString().c_str(), false);
            if (m_shipStreak) {
                m_shipStreak->updateFade(fade);
                m_shipStreak->setStroke(stroke);
                m_shipStreak->setTexture(texture);
            }
            else {
                m_shipStreak = CCMotionStreak::create(fade, 1.0f, stroke, { 255, 255, 255 }, texture);
                m_shipStreak->setM_fMaxSeg(50.0f);
                m_shipStreak->setDontOpacityFade(true);
                m_parentLayer->addChild(m_shipStreak, -3);
            }
            m_shipStreak->setBlendFunc({
                GL_SRC_ALPHA, (uint32_t)(fireInfo.get<bool>("blend").unwrapOr(true) ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA)
            });
            m_shipStreak->setUserObject("name"_spr, CCString::create(info->getName()));
        }
        else if (m_shipStreak) {
            m_shipStreak->setUserObject("name"_spr, nullptr);
            if (MoreIcons::traditionalPacks) {
                m_shipStreak->setTexture(Get::TextureCache()->addImage(MoreIcons::vanillaTexturePath(
                    fmt::format("shipfire{:02}_001.png", (int)m_shipStreakType), true
                ).c_str(), false));
            }
        }
    }

    void updateStreakBlend(bool blend) {
        PlayerObject::updateStreakBlend(blend);

        if (auto info = getTrailInfo(IconType::Special)) {
            m_regularTrail->setBlendFunc({
                GL_SRC_ALPHA, (uint32_t)(info->getSpecialInfo().get<bool>("blend").unwrapOr(false) ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA)
            });
        }
    }

    void setPosition(const CCPoint& position) {
        PlayerObject::setPosition(position);

        if (m_isPlatformer || !m_isShip || !m_shipStreak) return;

        auto info = getTrailInfo(IconType::ShipFire);
        if (!info) return;

        auto fireInfo = info->getSpecialInfo();
        m_shipStreak->setPosition(
            m_shipStreak->getParent()->convertToNodeSpace(m_mainLayer->convertToWorldSpace(m_vehicleSprite->getPosition() + CCPoint {
                fireInfo.get<float>("x").unwrapOr(-8.0f) * reverseMod(), fireInfo.get<float>("y").unwrapOr(-3.0f)
            }))
        );
    }

    void update(float dt) {
        PlayerObject::update(dt);

        if (!m_shipStreak) return;

        if (auto info = getTrailInfo(IconType::ShipFire)) {
            auto texture = info->getTextureString();
            auto fireCount = info->getFireCount();
            auto interval = info->getSpecialInfo().get<float>("interval").unwrapOr(0.05f);
            texture.replace(texture.size() - 7, 7, fmt::format("{:03}.png", (int)(m_totalTime / interval) % fireCount + 1));
            m_shipStreak->setTexture(Get::TextureCache()->addImage(texture.c_str(), false));
        }
        else if (MoreIcons::traditionalPacks) {
            auto fireCount = Defaults::getShipFireCount((int)m_shipStreakType);
            auto interval = Defaults::getShipFireInfo((int)m_shipStreakType).get<float>("interval").unwrapOr(0.05f);
            m_shipStreak->setTexture(Get::TextureCache()->addImage(MoreIcons::vanillaTexturePath(
                fmt::format("shipfire{:02}_{:03}.png", (int)m_shipStreakType, (int)(m_totalTime / interval) % fireCount + 1), true
            ).c_str(), false));
        }
    }

    void updateFireSettings() {
        if (!m_shipStreak) return;

        auto info = getTrailInfo(IconType::ShipFire);
        if (!info) return;

        auto factor = m_vehicleSize == 1.0f ? 1.0f : 0.5f;
        if (m_playerSpeed == 0.7f) factor *= 1.3f;
        else if (m_playerSpeed == 0.9f) factor *= 1.2f;
        else if (m_playerSpeed == 1.1f) factor *= 1.1f;
        else if (m_playerSpeed == 1.3f) factor *= 1.05f;

        auto fireInfo = info->getSpecialInfo();
        m_shipStreak->updateFade(fireInfo.get<float>("fade").unwrapOr(0.1f) * factor);
        m_shipStreak->setStroke(fireInfo.get<float>("stroke").unwrapOr(20.0f) * factor);
    }

    void togglePlayerScale(bool enable, bool noEffects) {
        PlayerObject::togglePlayerScale(enable, noEffects);
        updateFireSettings();
    }

    void updateTimeMod(float speed, bool noEffects) {
        PlayerObject::updateTimeMod(speed, noEffects);
        updateFireSettings();
    }
};

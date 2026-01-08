#include "../MoreIcons.hpp"
#include "../utils/Defaults.hpp"
#include "../utils/Get.hpp"
#include "../utils/Icons.hpp"
#include "../utils/Json.hpp"
#include <Geode/binding/CCCircleWave.hpp>
#include <Geode/binding/ExplodeItemNode.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <jasmine/random.hpp>
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

    IconInfo* getIconInfo(IconType type) {
        if (isPlayer1()) return more_icons::activeIcon(type, false);
        else if (isPlayer2()) return more_icons::activeIcon(type, true);
        else return nullptr;
    }

    void updateIcon(IconType type) {
        auto icon = getIconInfo(type);
        if (icon) more_icons::updatePlayerObject(this, icon);
        else Icons::setIcon(this, nullptr);
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
            return Icons::setIcon(this, nullptr);
        }

        int* loadedIcon = nullptr;
        if (!Icons::preloadIcons) {
            if (auto foundRequests = Icons::requestedIcons.find(m_iconRequestID); foundRequests != Icons::requestedIcons.end()) {
                auto& iconRequests = foundRequests->second;
                if (auto found = iconRequests.find(type); found != iconRequests.end()) {
                    loadedIcon = &Icons::loadedIcons[found->second];
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
            auto icon = getIconInfo(IconType::Robot);
            if (icon) m_iconSprite->setDisplayFrame(Icons::getFrame("{}_01_001.png"_spr, icon->getName()));
        }
    }

    void toggleSpiderMode(bool enable, bool noEffects) {
        auto isSpider = m_isSpider;
        PlayerObject::toggleSpiderMode(enable, noEffects);

        if (!isSpider && m_isSpider) {
            auto icon = getIconInfo(IconType::Spider);
            if (icon) m_iconSprite->setDisplayFrame(Icons::getFrame("{}_01_001.png"_spr, icon->getName()));
        }
    }

    void setupStreak() {
        PlayerObject::setupStreak();

        if (auto info = getIconInfo(IconType::Special)) {
            auto& trailInfo = info->getSpecialInfo();
            auto tint = Json::get(trailInfo, "tint", false);
            auto show = Json::get(trailInfo, "show", false);
            auto fade = Json::get(trailInfo, "fade", 0.3f);
            auto stroke = Json::get(trailInfo, "stroke", 14.0f);

            m_streakStrokeWidth = stroke;
            m_disableStreakTint = !tint;
            m_alwaysShowStreak = show;

            m_regularTrail->updateFade(fade);
            m_regularTrail->setStroke(stroke);
            m_regularTrail->setTexture(Get::TextureCache()->addImage(info->getTextureString().c_str(), false));
            if (info->getSpecialID() == 6) m_regularTrail->enableRepeatMode(0.1f);
            MoreIcons::blendStreak(m_regularTrail, info);
            Icons::setIcon(m_regularTrail, info);
        }
        else {
            Icons::setIcon(m_regularTrail, nullptr);
            if (Icons::traditionalPacks && (!Loader::get()->isModLoaded("acaruso.pride") || m_playerStreak != 2)) {
                m_regularTrail->setTexture(Get::TextureCache()->addImage(Icons::vanillaTexturePath(
                    fmt::format("streak_{:02}_001.png", m_playerStreak), true
                ).c_str(), false));
                if (m_playerStreak == 6) m_regularTrail->enableRepeatMode(0.1f);
            }
        }

        if (auto info = getIconInfo(IconType::ShipFire)) {
            auto& fireInfo = info->getSpecialInfo();
            auto fade = Json::get(fireInfo, "fade", 0.1f);
            auto stroke = Json::get(fireInfo, "stroke", 20.0f);
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
            MoreIcons::blendStreak(m_shipStreak, info);
            Icons::setIcon(m_shipStreak, info);
        }
        else if (m_shipStreak) {
            Icons::setIcon(m_shipStreak, nullptr);
            if (Icons::traditionalPacks) {
                m_shipStreak->setTexture(Get::TextureCache()->addImage(Icons::vanillaTexturePath(
                    fmt::format("shipfire{:02}_001.png", (int)m_shipStreakType), true
                ).c_str(), false));
            }
        }
    }

    void updateStreakBlend(bool blend) {
        PlayerObject::updateStreakBlend(blend);

        if (auto info = getIconInfo(IconType::Special)) MoreIcons::blendStreak(m_regularTrail, info);
    }

    void setPosition(const CCPoint& position) {
        PlayerObject::setPosition(position);

        if (m_isPlatformer || !m_isShip || !m_shipStreak) return;

        auto info = getIconInfo(IconType::ShipFire);
        if (!info) return;

        auto& fireInfo = info->getSpecialInfo();
        m_shipStreak->setPosition(
            m_shipStreak->getParent()->convertToNodeSpace(m_mainLayer->convertToWorldSpace(m_vehicleSprite->getPosition() + CCPoint {
                Json::get(fireInfo, "x", -8.0f) * reverseMod(), Json::get(fireInfo, "y", -3.0f)
            }))
        );
    }

    void update(float dt) {
        PlayerObject::update(dt);

        if (!m_shipStreak) return;

        if (auto info = getIconInfo(IconType::ShipFire)) {
            auto texture = info->getTextureString();
            auto fireCount = info->getFireCount();
            auto interval = Json::get(info->getSpecialInfo(), "interval", 0.05f);
            texture.replace(texture.size() - 7, 3, fmt::format("{:03}", (int)(m_totalTime / interval) % fireCount + 1));
            m_shipStreak->setTexture(Get::TextureCache()->addImage(texture.c_str(), false));
        }
        else if (Icons::traditionalPacks) {
            auto fireCount = Defaults::getShipFireCount((int)m_shipStreakType);
            auto interval = std::max(Json::get(Defaults::getShipFireInfo((int)m_shipStreakType), "interval", 0.05f), 0.001f);
            m_shipStreak->setTexture(Get::TextureCache()->addImage(Icons::vanillaTexturePath(
                fmt::format("shipfire{:02}_{:03}.png", (int)m_shipStreakType, (int)(m_totalTime / interval) % fireCount + 1), true
            ).c_str(), false));
        }
    }

    void updateFireSettings() {
        if (!m_shipStreak) return;

        auto info = getIconInfo(IconType::ShipFire);
        if (!info) return;

        auto factor = m_vehicleSize == 1.0f ? 1.0f : 0.5f;
        if (m_playerSpeed == 0.7f) factor *= 1.3f;
        else if (m_playerSpeed == 0.9f) factor *= 1.2f;
        else if (m_playerSpeed == 1.1f) factor *= 1.1f;
        else if (m_playerSpeed == 1.3f) factor *= 1.05f;

        auto& fireInfo = info->getSpecialInfo();
        m_shipStreak->updateFade(Json::get(fireInfo, "fade", 0.1f) * factor);
        m_shipStreak->setStroke(Json::get(fireInfo, "stroke", 20.0f) * factor);
    }

    void togglePlayerScale(bool enable, bool noEffects) {
        PlayerObject::togglePlayerScale(enable, noEffects);
        updateFireSettings();
    }

    void updateTimeMod(float speed, bool noEffects) {
        PlayerObject::updateTimeMod(speed, noEffects);
        updateFireSettings();
    }

    void playDeathEffect() {
        auto info = getIconInfo(IconType::DeathEffect);
        if (!info) return PlayerObject::playDeathEffect();

        auto& deathInfo = info->getSpecialInfo();
        auto& position = getPosition();
        auto vehicleSize = m_vehicleSize;
        if (vehicleSize >= 1.0f) vehicleSize *= 0.9f;

        auto fadeOut = CCFadeTo::create(0.05f, 0);
        fadeOut->setTag(4);
        runAction(fadeOut);

        auto scale = Json::get(deathInfo, "scale", 1.0f);
        auto scaleVar = Json::get(deathInfo, "scale-variance", 0.0f);
        auto rotation = Json::get(deathInfo, "rotation", 0.0f);
        auto rotationVar = Json::get(deathInfo, "rotation-variance", 0.0f);
        auto blend = Json::get(deathInfo, "blend", false);

        auto& frameNames = info->getFrameNames();
        auto effect = CCSprite::createWithSpriteFrameName(frameNames[0].c_str());
        if (blend) effect->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        effect->setPosition(position);
        auto effectScale = ((float)jasmine::random::get(-scaleVar, scaleVar) + scale) * vehicleSize;
        effect->setScale(effectScale);
        effect->setRotation((float)jasmine::random::get(-rotationVar, rotationVar) + rotation);

        auto frames = CCArray::create();
        auto spriteFrameCache = Get::SpriteFrameCache();
        for (auto it = frameNames.begin() + 1; it != frameNames.end(); ++it) {
            frames->addObject(spriteFrameCache->spriteFrameByName(it->c_str()));
        }

        auto frameDelay = Json::get(deathInfo, "frame-delay", 0.05f);
        auto frameDelayVariance = Json::get(deathInfo, "frame-delay-variance", 0.0f);

        auto delay = (float)jasmine::random::get(-frameDelayVariance, frameDelayVariance) + frameDelay;

        effect->runAction(CCSequence::createWithTwoActions(
            CCAnimate::create(CCAnimation::createWithSpriteFrames(frames, delay)), CCRemoveSelf::create()
        ));

        if (Json::get(deathInfo, "fade", true)) {
            effect->runAction(CCSequence::createWithTwoActions(
                CCDelayTime::create(Json::get(deathInfo, "fade-delay-multiplier", 6.0f) * delay),
                CCFadeOut::create(Json::get(deathInfo, "fade-time-multiplier", 6.0f) * delay)
            ));
        }

        m_gameLayer->m_objectLayer->addChild(effect, 10000);

        auto particles = CCParticleSystemQuad::create("explodeEffect.plist", false);
        particles->setPositionType(kCCPositionTypeGrouped);
        particles->setAutoRemoveOnFinish(true);
        particles->setPosition(position);
        particles->setStartColor({ 1.0f, 1.0f, 1.0f, 1.0f });
        particles->setEndColor({ 1.0f, 1.0f, 1.0f, 1.0f });
        particles->setStartColorVar({ 0.0f, 0.0f, 0.0f, 1.0f });
        particles->setEndColorVar({ 0.0f, 0.0f, 0.0f, 1.0f });
        particles->setScale(vehicleSize);
        particles->resetSystem();
        m_parentLayer->addChild(particles, 99);

        auto circleUseScale = Json::get(deathInfo, "circle-use-scale", false);
        auto circleUseDelay = Json::get(deathInfo, "circle-use-delay", false);
        auto circleFactor = circleUseScale ? effectScale : vehicleSize;
        auto circleStartRadius = Json::get(deathInfo, "circle-start-radius", 10.0f) * circleFactor;
        auto circleEndRadius = Json::get(deathInfo, "circle-end-radius", 110.0f) * circleFactor;
        auto circleDuration = Json::get(deathInfo, "circle-duration", 0.6f);
        if (circleUseDelay) circleDuration *= delay;

        auto circle = CCCircleWave::create(circleStartRadius, circleEndRadius, circleDuration, false);
        circle->m_color.r = 255;
        circle->m_color.g = 255;
        circle->m_color.b = 255;
        circle->setPosition(position);
        circle->m_opacityMod = 1.0f;
        m_parentLayer->addChild(circle, 1000);

        auto outlineUseScale = Json::get(deathInfo, "outline-use-scale", false);
        auto outlineUseDelay = Json::get(deathInfo, "outline-use-delay", false);
        auto outlineFactor = outlineUseScale ? effectScale : vehicleSize;
        auto outlineStartRadius = Json::get(deathInfo, "outline-start-radius", 10.0f) * outlineFactor;
        auto outlineEndRadius = Json::get(deathInfo, "outline-end-radius", 115.0f) * outlineFactor;
        auto outlineDuration = Json::get(deathInfo, "outline-duration", 0.4f);
        if (outlineUseDelay) outlineDuration *= delay;

        auto outline = CCCircleWave::create(outlineStartRadius, outlineEndRadius, outlineDuration, false);
        outline->m_color.r = 255;
        outline->m_color.g = 255;
        outline->m_color.b = 255;
        outline->setPosition(position);
        outline->m_circleMode = CircleMode::Outline;
        outline->m_lineWidth = 10;
        m_parentLayer->addChild(outline, 99);

        if (Get::GameManager()->getGameVariable(GameVar::PlayerExplode)) return;

        int size = vehicleSize * 40.0f;
        auto renderTexture = CCRenderTexture::create(size, size);
        auto savedPosition = getPosition();
        setPosition({ vehicleSize * 20.0f, vehicleSize * 20.0f });
        renderTexture->beginWithClear(0.0f, 0.0f, 0.0f, 0.0f);
        m_dashSpritesContainer->visit();
        visit();
        renderTexture->end();
        setPosition(savedPosition);

        auto explodeNode = ExplodeItemNode::create(renderTexture);
        explodeNode->setPosition(position);
        auto playerColor4F = to4F(to4B(m_playerColor1));
        float xVelocity = getCurrentXVelocity() * 0.72;
        int countX = jasmine::random::get(2.0, 4.0);
        int countY = jasmine::random::get(2.0, 4.0);
        auto randomValue = jasmine::random::get();
        explodeNode->createSprites(
            randomValue > 0.95 ? 1 : countX, randomValue > 0.9 ? 1 : countY, xVelocity,
            xVelocity * 0.5f, 6.0f, 3.0f, 1.4f, 0.0f, playerColor4F, playerColor4F, false
        );
        if (auto parent = getParent()) parent->addChild(explodeNode, 101);
        else m_parentLayer->addChild(explodeNode, 101);
    }
};

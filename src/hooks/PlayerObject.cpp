#include "../MoreIcons.hpp"
#include "../utils/Defaults.hpp"
#include "../utils/Get.hpp"
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

    std::string activeIcon(IconType type) {
        if (isPlayer1()) return more_icons::activeIcon(type, false);
        else if (isPlayer2()) return more_icons::activeIcon(type, true);
        else return std::string();
    }

    void updateIcon(IconType type) {
        auto icon = activeIcon(type);
        if (!icon.empty()) more_icons::updatePlayerObject(this, icon, type);
        else MoreIcons::setName(this, {});
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
            return MoreIcons::setName(this, {});
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
            auto iconName = activeIcon(IconType::Robot);
            if (!iconName.empty()) m_iconSprite->setDisplayFrame(MoreIcons::getFrame("{}_01_001.png"_spr, iconName));
        }
    }

    void toggleSpiderMode(bool enable, bool noEffects) {
        auto isSpider = m_isSpider;
        PlayerObject::toggleSpiderMode(enable, noEffects);

        if (!isSpider && m_isSpider) {
            auto iconName = activeIcon(IconType::Spider);
            if (!iconName.empty()) m_iconSprite->setDisplayFrame(MoreIcons::getFrame("{}_01_001.png"_spr, iconName));
        }
    }

    IconInfo* getIconInfo(IconType type) {
        return more_icons::getIcon(activeIcon(type), type);
    }

    void setupStreak() {
        PlayerObject::setupStreak();

        if (auto info = getIconInfo(IconType::Special)) {
            auto& trailInfo = info->getSpecialInfo();
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
            MoreIcons::blendStreak(m_regularTrail, info);
            MoreIcons::setName(m_regularTrail, info->getName());
        }
        else {
            MoreIcons::setName(m_regularTrail, {});
            if (MoreIcons::traditionalPacks && (!Loader::get()->isModLoaded("acaruso.pride") || m_playerStreak != 2)) {
                m_regularTrail->setTexture(Get::TextureCache()->addImage(MoreIcons::vanillaTexturePath(
                    fmt::format("streak_{:02}_001.png", m_playerStreak), true
                ).c_str(), false));
                if (m_playerStreak == 6) m_regularTrail->enableRepeatMode(0.1f);
            }
        }

        if (auto info = getIconInfo(IconType::ShipFire)) {
            auto& fireInfo = info->getSpecialInfo();
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
            MoreIcons::blendStreak(m_shipStreak, info);
            MoreIcons::setName(m_shipStreak, info->getName());
        }
        else if (m_shipStreak) {
            MoreIcons::setName(m_shipStreak, {});
            if (MoreIcons::traditionalPacks) {
                m_shipStreak->setTexture(Get::TextureCache()->addImage(MoreIcons::vanillaTexturePath(
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
                fireInfo.get<float>("x").unwrapOr(-8.0f) * reverseMod(), fireInfo.get<float>("y").unwrapOr(-3.0f)
            }))
        );
    }

    void update(float dt) {
        PlayerObject::update(dt);

        if (!m_shipStreak) return;

        if (auto info = getIconInfo(IconType::ShipFire)) {
            auto texture = info->getTextureString();
            auto fireCount = info->getFireCount();
            auto interval = info->getSpecialInfo().get<float>("interval").unwrapOr(0.05f);
            texture.replace(texture.size() - 7, 3, fmt::format("{:03}", (int)(m_totalTime / interval) % fireCount + 1));
            m_shipStreak->setTexture(Get::TextureCache()->addImage(texture.c_str(), false));
        }
        else if (MoreIcons::traditionalPacks) {
            auto fireCount = Defaults::getShipFireCount((int)m_shipStreakType);
            auto interval = std::max(Defaults::getShipFireInfo((int)m_shipStreakType).get<float>("interval").unwrapOr(0.05f), FLT_EPSILON);
            m_shipStreak->setTexture(Get::TextureCache()->addImage(MoreIcons::vanillaTexturePath(
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

        auto scale = deathInfo.get<float>("scale").unwrapOr(1.0f);
        auto scaleVar = deathInfo.get<float>("scale-variance").unwrapOr(0.0f);
        auto rotation = deathInfo.get<float>("rotation").unwrapOr(0.0f);
        auto rotationVar = deathInfo.get<float>("rotation-variance").unwrapOr(0.0f);
        auto blend = deathInfo.get<bool>("blend").unwrapOr(false);

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

        auto frameDelay = deathInfo.get<float>("frame-delay").unwrapOr(0.05f);
        auto frameDelayVariance = deathInfo.get<float>("frame-delay-variance").unwrapOr(0.0f);

        auto delay = (float)jasmine::random::get(-frameDelayVariance, frameDelayVariance) + frameDelay;

        effect->runAction(CCSequence::createWithTwoActions(
            CCAnimate::create(CCAnimation::createWithSpriteFrames(frames, delay)),
            CCCallFunc::create(effect, callfunc_selector(CCNode::removeMeAndCleanup))
        ));

        if (deathInfo.get<bool>("fade").unwrapOr(true)) {
            effect->runAction(CCSequence::createWithTwoActions(
                CCDelayTime::create(deathInfo.get<float>("fade-delay-multiplier").unwrapOr(6.0f) * delay),
                CCFadeOut::create(deathInfo.get<float>("fade-time-multiplier").unwrapOr(6.0f) * delay)
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

        auto circleUseScale = deathInfo.get<bool>("circle-use-scale").unwrapOr(false);
        auto circleUseDelay = deathInfo.get<bool>("circle-use-delay").unwrapOr(false);
        auto circleFactor = circleUseScale ? effectScale : vehicleSize;
        auto circleStartRadius = deathInfo.get<float>("circle-start-radius").unwrapOr(10.0f) * circleFactor;
        auto circleEndRadius = deathInfo.get<float>("circle-end-radius").unwrapOr(110.0f) * circleFactor;
        auto circleDuration = deathInfo.get<float>("circle-duration").unwrapOr(0.6f);
        if (circleUseDelay) circleDuration *= delay;

        auto circle = CCCircleWave::create(circleStartRadius, circleEndRadius, circleDuration, false);
        circle->m_color.r = 255;
        circle->m_color.g = 255;
        circle->m_color.b = 255;
        circle->setPosition(position);
        circle->m_opacityMod = 1.0f;
        m_parentLayer->addChild(circle, 1000);

        auto outlineUseScale = deathInfo.get<bool>("outline-use-scale").unwrapOr(false);
        auto outlineUseDelay = deathInfo.get<bool>("outline-use-delay").unwrapOr(false);
        auto outlineFactor = outlineUseScale ? effectScale : vehicleSize;
        auto outlineStartRadius = deathInfo.get<float>("outline-start-radius").unwrapOr(10.0f) * outlineFactor;
        auto outlineEndRadius = deathInfo.get<float>("outline-end-radius").unwrapOr(115.0f) * outlineFactor;
        auto outlineDuration = deathInfo.get<float>("outline-duration").unwrapOr(0.4f);
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

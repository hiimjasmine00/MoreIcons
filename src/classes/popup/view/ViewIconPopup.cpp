#include "ViewIconPopup.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Defaults.hpp"
#include "../../../utils/Get.hpp"
#include "../../../utils/Icons.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

ViewIconPopup* ViewIconPopup::create(IconType type, int id, IconInfo* info) {
    auto ret = new ViewIconPopup();
    if (ret->init(type, id, info)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ViewIconPopup::init(IconType type, int id, IconInfo* info) {
    if (!BasePopup::init(
        350.0f, type == IconType::Robot || type == IconType::Spider ? 200.0f : type <= IconType::Jetpack ? 170.0f : 120.0f,
        "geode.loader/GE_square03.png"
    )) return false;

    setID("ViewIconPopup");
    setTitle(info ? info->getShortName() : fmt::format("{} {}", Constants::getSingularUppercase(type), id));
    m_title->setID("view-icon-title");

    if (info && info->inTexturePack()) {
        auto subTitle = CCLabelBMFont::create(info->getPackName().c_str(), "goldFont.fnt");
        subTitle->setPosition({ 175.0f, m_size.height - 35.0f });
        subTitle->setScale(0.4f);
        subTitle->setID("view-icon-sub-title");
        m_mainLayer->addChild(subTitle);
    }

    if (type <= IconType::Jetpack) {
        auto isRobot = type == IconType::Robot || type == IconType::Spider;
        std::span<const std::initializer_list<std::string_view>> suffixes;
        if (isRobot) {
            static std::initializer_list<std::initializer_list<std::string_view>> robotSuffixes = {
                { "_01_001", "_01_2_001", "_01_glow_001", "_01_extra_001" },
                { "_02_001", "_02_2_001", "_02_glow_001" },
                { "_03_001", "_03_2_001", "_03_glow_001" },
                { "_04_001", "_04_2_001", "_04_glow_001" }
            };
            suffixes = robotSuffixes;
        }
        else if (type == IconType::Ufo) {
            static std::initializer_list<std::initializer_list<std::string_view>> ufoSuffixes = {
                { "_001", "_2_001", "_3_001", "_glow_001", "_extra_001" }
            };
            suffixes = ufoSuffixes;
        }
        else {
            static std::initializer_list<std::initializer_list<std::string_view>> cubeSuffixes = {
                { "_001", "_2_001", "_glow_001", "_extra_001" }
            };
            suffixes = cubeSuffixes;
        }

        auto player = SimplePlayer::create(1);
        if (info) more_icons::updateSimplePlayer(player, info->getName(), type);
        else player->updatePlayerFrame(id, type);
        player->setGlowOutline({ 255, 255, 255 });
        player->setPosition({ 175.0f, (isRobot ? 160.0f : 80.0f) - suffixes.size() * 30.0f });
        player->setID("player-icon");
        m_mainLayer->addChild(player);

        auto prefix = info ? fmt::format("{}"_spr, info->getName()) : MoreIcons::getIconName(id, type);
        for (size_t i = 0; i < suffixes.size(); i++) {
            auto& subSuffixes = suffixes[i];

            auto container = CCNode::create();
            container->setPosition({ 175.0f, (isRobot ? 140.0f : 100.0f) - i * 30.0f + std::max<ptrdiff_t>(i - 1, 0) * 10.0f });
            container->setAnchorPoint({ 0.5f, 0.5f });
            container->setContentSize({ 350.0f, 30.0f });
            container->setID(isRobot ? fmt::format("frame-container{}", (*subSuffixes.begin()).substr(0, 3)) : "frame-container_01");

            for (auto& suffix : subSuffixes) {
                if (auto spriteFrame = Icons::getFrame("{}{}.png", prefix, suffix)) {
                    auto sprite = CCSprite::createWithSpriteFrame(spriteFrame);
                    auto& size = sprite->getContentSize();
                    sprite->setPosition(size / 2.0f);
                    auto node = CCNode::create();
                    node->setContentSize(size);
                    node->setAnchorPoint({ 0.5f, 0.5f });
                    node->setID(fmt::format("frame-node{}", isRobot ? suffix.substr(3) : suffix));
                    node->addChild(sprite);
                    container->addChild(node);
                }
            }

            auto gap = 0.0f;
            if (i == 0) {
                switch (container->getChildrenCount()) {
                    case 2: gap = 100.0f; break;
                    case 3: gap = 50.0f; break;
                    case 4: gap = 20.0f; break;
                    case 5: gap = 10.0f; break;
                }
            }
            else gap = 20.0f;

            container->setLayout(RowLayout::create()->setGap(gap));
            container->updateLayout();

            for (auto node : container->getChildrenExt()) {
                auto height = container->getContentHeight() * node->getScale();
                node->setContentSize({ height, height });
                node->getChildByIndex(0)->setPosition({ height / 2.0f, height / 2.0f });
            }

            container->updateLayout();
            m_mainLayer->addChild(container);
        }
    }
    else if (type == IconType::Special) {
        auto streak = CCSprite::create((info ? info->getTextureString() : MoreIcons::getTrailTexture(id)).c_str());
        auto& trailInfo = info ? info->getSpecialInfo() : Defaults::getTrailInfo(id);
        if (trailInfo.get<bool>("blend").unwrapOr(true)) streak->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        streak->setPosition({ 175.0f, 50.0f });
        streak->setRotation(-90.0f);
        auto& size = streak->getContentSize();
        streak->setScaleX(trailInfo.get<float>("stroke").unwrapOr(14.0f) / size.width);
        streak->setScaleY(320.0f / size.height);
        if (trailInfo.get<bool>("tint").unwrapOr(false)) {
            streak->setColor(MoreIcons::vanillaColor2(MoreIcons::dualSelected()));
        }
        streak->setID("streak-preview");
        m_mainLayer->addChild(streak);
    }

    handleTouchPriority(this);

    return true;
}

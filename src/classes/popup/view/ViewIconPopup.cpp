#include "ViewIconPopup.hpp"
#include "../../misc/SimpleIcon.hpp"
#include "../../../MoreIcons.hpp"
#include "../../../utils/Constants.hpp"
#include "../../../utils/Defaults.hpp"
#include "../../../utils/Icons.hpp"
#include <jasmine/mod.hpp>
#include <span>

using namespace geode::prelude;
using namespace jasmine::mod;

static std::initializer_list<std::initializer_list<std::string_view>> robotSuffixes = {
    { "_01_001", "_01_2_001", "_01_glow_001", "_01_extra_001" },
    { "_02_001", "_02_2_001", "_02_glow_001" },
    { "_03_001", "_03_2_001", "_03_glow_001" },
    { "_04_001", "_04_2_001", "_04_glow_001" }
};

static std::initializer_list<std::initializer_list<std::string_view>> ufoSuffixes = {
    { "_001", "_2_001", "_3_001", "_glow_001", "_extra_001" }
};

static std::initializer_list<std::initializer_list<std::string_view>> cubeSuffixes = {
    { "_001", "_2_001", "_glow_001", "_extra_001" }
};

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
        std::span<std::initializer_list<std::string_view> const> suffixes;
        if (isRobot) suffixes = robotSuffixes;
        else if (type == IconType::Ufo) suffixes = ufoSuffixes;
        else suffixes = cubeSuffixes;

        auto prefix = info ? fmt::format("{}"_spr, info->getName()) : MoreIcons::getIconName(id, type);

        auto player = SimpleIcon::create(type, prefix);
        player->setPosition({ 175.0f, (isRobot ? 160.0f : 80.0f) - suffixes.size() * 30.0f });
        player->setID("player-icon");
        m_mainLayer->addChild(player);

        for (size_t i = 0; i < suffixes.size(); i++) {
            auto& subSuffixes = suffixes[i];

            auto container = CCNode::create();
            container->setPosition({ 175.0f, (isRobot ? 140.0f : 100.0f) - i * 30.0f + std::max<ptrdiff_t>(i - 1, 0) * 10.0f });
            container->setAnchorPoint({ 0.5f, 0.5f });
            container->setContentSize({ 350.0f, 30.0f });
            container->setID(isRobot ? fmt::format("frame-container{}", (*subSuffixes.begin()).substr(0, 3)) : "frame-container_01");
            m_mainLayer->addChild(container);

            for (auto& suffix : subSuffixes) {
                if (auto spriteFrame = Icons::getFrame("{}{}.png", prefix, suffix)) {
                    auto sprite = CCSprite::createWithSpriteFrame(spriteFrame);
                    sprite->setPosition({ 15.0f, 15.0f });
                    auto node = CCNode::create();
                    node->setContentSize({ 30.0f, 30.0f });
                    node->setAnchorPoint({ 0.5f, 0.5f });
                    node->setID(fmt::format("frame-node{}", isRobot ? suffix.substr(3) : suffix));
                    node->addChild(sprite);
                    container->addChild(node);
                }
            }

            container->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Between));
        }
    }
    else if (type == IconType::Special) {
        auto streak = CCSprite::create((info ? info->getTextureString() : fmt::format("streak_{:02}_001.png", id)).c_str());
        auto& trailInfo = info ? info->getSpecialInfo() : Defaults::getTrailInfo(id);
        if (trailInfo["blend"].asBool().unwrapOr(true)) streak->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
        streak->setPosition({ 175.0f, 50.0f });
        streak->setRotation(-90.0f);
        auto& size = streak->getContentSize();
        streak->setScaleX((float)trailInfo["stroke"].asDouble().unwrapOr(14.0) / size.width);
        streak->setScaleY(320.0f / size.height);
        if (trailInfo["tint"].asBool().unwrapOr(false)) {
            streak->setColor(MoreIcons::vanillaColor2(MoreIcons::dualSelected()));
        }
        streak->setID("streak-preview");
        m_mainLayer->addChild(streak);
    }

    handleTouchPriority(this);

    return true;
}

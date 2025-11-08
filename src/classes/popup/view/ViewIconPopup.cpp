#include "ViewIconPopup.hpp"
#include "../../../api/MoreIconsAPI.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/loader/Mod.hpp>

using namespace geode::prelude;

ViewIconPopup* ViewIconPopup::create(IconType type, int id, IconInfo* info) {
    auto ret = new ViewIconPopup();
    if (ret->initAnchored(
        350.0f,
        120.0f + (type <= IconType::Jetpack) * 50.0f + (type == IconType::Robot || type == IconType::Spider) * 30.0f,
        type,
        id,
        info,
        "geode.loader/GE_square03.png"
    )) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ViewIconPopup::setup(IconType type, int id, IconInfo* info) {
    auto miType = MoreIconsAPI::convertType(type);

    setID("ViewIconPopup");
    setTitle(fmt::format("{} Viewer", MoreIconsAPI::uppercase[miType]));
    m_title->setID("view-icon-title");
    m_mainLayer->setID("main-layer");
    m_buttonMenu->setID("button-menu");
    m_bgSprite->setID("background");
    m_closeBtn->setID("close-button");

    if (info && !info->packID.empty()) {
        auto subTitle = CCLabelBMFont::create(info->packName.c_str(), "goldFont.fnt");
        subTitle->setPosition({ 175.0f, m_size.height - 35.0f });
        subTitle->setScale(0.4f);
        subTitle->setID("view-icon-sub-title");
        m_mainLayer->addChild(subTitle);
    }

    if (type <= IconType::Jetpack) {
        auto isRobot = type == IconType::Robot || type == IconType::Spider;
        std::vector<std::vector<std::string>> suffixes;
        suffixes.reserve(isRobot * 3 + 1);
        if (isRobot) {
            suffixes.push_back({ "_01_001.png", "_01_2_001.png", "_01_glow_001.png", "_01_extra_001.png" });
            suffixes.push_back({ "_02_001.png", "_02_2_001.png", "_02_glow_001.png" });
            suffixes.push_back({ "_03_001.png", "_03_2_001.png", "_03_glow_001.png" });
            suffixes.push_back({ "_04_001.png", "_04_2_001.png", "_04_glow_001.png" });
        }
        else {
            if (type == IconType::Ufo) suffixes.push_back({ "_001.png", "_2_001.png", "_3_001.png", "_glow_001.png", "_extra_001.png" });
            else suffixes.push_back({ "_001.png", "_2_001.png", "_glow_001.png", "_extra_001.png" });
        }

        auto player = SimplePlayer::create(1);
        if (info) MoreIconsAPI::updateSimplePlayer(player, info->name, type, false);
        else player->updatePlayerFrame(id, type);
        player->setGlowOutline({ 255, 255, 255 });
        player->setPosition({ 175.0f, 80.0f + isRobot * 80.0f - suffixes.size() * 30.0f });
        player->setID("player-icon");
        m_mainLayer->addChild(player);

        auto prefix = info ? fmt::format("{}"_spr, info->name) : fmt::format("{}{:02}", MoreIconsAPI::prefixes[miType], id);
        auto spriteFrameCache = MoreIconsAPI::getSpriteFrameCache();
        for (int i = 0; i < suffixes.size(); i++) {
            auto container = CCNode::create();
            container->setPosition({ 175.0f, 100.0f + isRobot * 40.0f - i * 30.0f + std::max(i - 1, 0) * 10.0f });
            container->setAnchorPoint({ 0.5f, 0.5f });
            container->setContentSize({ 350.0f, 30.0f });
            container->setID(fmt::format("frame-container-{}", i + 1));

            auto& subSuffixes = suffixes[i];
            for (int j = 0; j < subSuffixes.size(); j++) {
                if (auto spriteFrame = MoreIconsAPI::getFrame(prefix + subSuffixes[j])) {
                    auto sprite = CCSprite::createWithSpriteFrame(spriteFrame);
                    auto& size = sprite->getContentSize();
                    sprite->setPosition(size / 2.0f);
                    auto node = CCNode::create();
                    node->setContentSize(size);
                    node->setAnchorPoint({ 0.5f, 0.5f });
                    node->setID(fmt::format("frame-node-{}", j + 1));
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

            for (auto node : CCArrayExt<CCNode*>(container->getChildren())) {
                auto height = container->getContentHeight() * node->getScale();
                node->setContentSize({ height, height });
                node->getChildByIndex(0)->setPosition({ height / 2.0f, height / 2.0f });
            }

            container->updateLayout();
            m_mainLayer->addChild(container);
        }
    }
    else if (type == IconType::Special) {
        auto streak = CCSprite::create((info ? info->textures[0] : fmt::format("streak_{:02}_001.png", id)).c_str());
        streak->setBlendFunc({
            GL_SRC_ALPHA,
            (uint32_t)GL_ONE_MINUS_SRC_ALPHA - (info && info->trailInfo.blend) * (uint32_t)GL_SRC_ALPHA
        });
        streak->setPosition({ 175.0f, 50.0f });
        streak->setRotation(-90.0f);
        auto& size = streak->getContentSize();
        streak->setScaleX(info->trailInfo.stroke / size.width);
        streak->setScaleY(320.0f / size.height);
        if (info->trailInfo.tint) {
            auto gameManager = MoreIconsAPI::getGameManager();
            auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
            streak->setColor(gameManager->colorForIdx(
                sdi && sdi->getSavedValue("2pselected", false) ? sdi->getSavedValue("color2", 0) : gameManager->m_playerColor2));
        }
        streak->setID("streak-preview");
        m_mainLayer->addChild(streak);
    }

    handleTouchPriority(this);

    return true;
}



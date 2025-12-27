#include "MoreIcons.hpp"
#include "utils/Constants.hpp"
#include "utils/Filesystem.hpp"
#include "utils/Get.hpp"
#include "utils/Icons.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJGarageLayer.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

Mod* MoreIcons::separateDualIcons = nullptr;

$on_mod(Loaded) {
    Icons::loadSettings();
    if (auto separateDualIcons = Loader::get()->getInstalledMod("weebify.separate_dual_icons")) {
        if (separateDualIcons->isEnabled()) {
            MoreIcons::separateDualIcons = separateDualIcons;
        }
        else if (separateDualIcons->shouldLoad()) {
            new EventListener([](ModStateEvent* e) {
                MoreIcons::separateDualIcons = e->getMod();
            }, ModStateFilter(separateDualIcons, ModEventType::Loaded));
        }
    }
}

Result<std::filesystem::path> MoreIcons::createTrash() {
    auto trashPath = Mod::get()->getConfigDir() / L("trash");
    GEODE_UNWRAP(file::createDirectoryAll(trashPath));
    std::error_code code;
    std::filesystem::permissions(trashPath, std::filesystem::perms::all, code);
    return Ok(trashPath);
}

bool MoreIcons::dualSelected() {
    auto sdi = separateDualIcons;
    return sdi && sdi->getSavedValue("2pselected", false);
}

ccColor3B MoreIcons::vanillaColor1(bool dual) {
    auto gameManager = Get::GameManager();
    auto sdi = dual ? separateDualIcons : nullptr;
    return gameManager->colorForIdx(sdi ? sdi->getSavedValue("color1", 0) : gameManager->m_playerColor);
}

ccColor3B MoreIcons::vanillaColor2(bool dual) {
    auto gameManager = Get::GameManager();
    auto sdi = dual ? separateDualIcons : nullptr;
    return gameManager->colorForIdx(sdi ? sdi->getSavedValue("color2", 0) : gameManager->m_playerColor2);
}

ccColor3B MoreIcons::vanillaColorGlow(bool dual) {
    auto gameManager = Get::GameManager();
    auto sdi = dual ? separateDualIcons : nullptr;
    return gameManager->colorForIdx(sdi ? sdi->getSavedValue("colorglow", 0) : gameManager->m_playerGlowColor);
}

bool MoreIcons::vanillaGlow(bool dual) {
    auto sdi = dual ? separateDualIcons : nullptr;
    return sdi ? sdi->getSavedValue("glow", false) : Get::GameManager()->m_playerGlow;
}

int MoreIcons::vanillaIcon(IconType type, bool dual) {
    auto sdi = dual ? separateDualIcons : nullptr;
    auto gameManager = sdi ? nullptr : Get::GameManager();
    switch (type) {
        case IconType::Cube: return sdi ? sdi->getSavedValue("cube", 1) : gameManager->m_playerFrame;
        case IconType::Ship: return sdi ? sdi->getSavedValue("ship", 1) : gameManager->m_playerShip;
        case IconType::Ball: return sdi ? sdi->getSavedValue("roll", 1) : gameManager->m_playerBall;
        case IconType::Ufo: return sdi ? sdi->getSavedValue("bird", 1) : gameManager->m_playerBird;
        case IconType::Wave: return sdi ? sdi->getSavedValue("dart", 1) : gameManager->m_playerDart;
        case IconType::Robot: return sdi ? sdi->getSavedValue("robot", 1) : gameManager->m_playerRobot;
        case IconType::Spider: return sdi ? sdi->getSavedValue("spider", 1) : gameManager->m_playerSpider;
        case IconType::Swing: return sdi ? sdi->getSavedValue("swing", 1) : gameManager->m_playerSwing;
        case IconType::Jetpack: return sdi ? sdi->getSavedValue("jetpack", 1) : gameManager->m_playerJetpack;
        case IconType::DeathEffect: return sdi ? sdi->getSavedValue("death", 1) : gameManager->m_playerDeathEffect;
        case IconType::Special: return sdi ? sdi->getSavedValue("trail", 1) : gameManager->m_playerStreak;
        case IconType::ShipFire: return sdi ? sdi->getSavedValue("shiptrail", 1) : gameManager->m_playerShipFire;
        default: return 0;
    }
}

void MoreIcons::updateGarage(GJGarageLayer* layer) {
    auto noLayer = layer == nullptr;
    if (noLayer) layer = Get::Director()->getRunningScene()->getChildByType<GJGarageLayer>(0);
    if (!layer) return;

    auto gameManager = Get::GameManager();
    auto player1 = layer->m_playerObject;
    auto iconType1 = gameManager->m_playerIconType;
    if (noLayer) player1->updatePlayerFrame(vanillaIcon(iconType1, false), iconType1);
    more_icons::updateSimplePlayer(player1, iconType1, false);

    if (separateDualIcons) {
        auto player2 = static_cast<SimplePlayer*>(layer->getChildByID("player2-icon"));
        auto iconType2 = (IconType)separateDualIcons->getSavedValue("lastmode", 0);
        if (noLayer) player2->updatePlayerFrame(vanillaIcon(iconType2, true), iconType2);
        more_icons::updateSimplePlayer(player2, iconType2, true);
    }

    if (noLayer) layer->selectTab(layer->m_iconType);
}

void MoreIcons::blendStreak(CCMotionStreak* streak, IconInfo* info) {
    if (info->getSpecialInfo().get<bool>("blend").unwrapOr(true)) streak->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
    else streak->setBlendFunc({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA });
}

CCSprite* MoreIcons::customIcon(IconInfo* info) {
    auto square = CCSprite::create(info->getIconString().c_str());
    if (square && !square->getUserObject("geode.texture-loader/fallback")) return square;

    square = CCSprite::createWithSpriteFrameName("playerSquare_001.png");
    square->setColor({ 150, 150, 150 });

    if (info->getType() == IconType::DeathEffect) {
        auto question = CCLabelBMFont::create("?", "bigFont.fnt");
        question->setScale(0.6f);
        question->setPosition(square->getContentSize() / 2.0f);
        square->addChild(question);
    }
    else {
        auto sprite = CCSprite::create(info->getTextureString().c_str());
        limitNodeHeight(sprite, 27.0f, 999.0f, 0.001f);
        sprite->setPosition(square->getContentSize() / 2.0f);
        square->addChild(sprite);
    }

    return square;
}

std::filesystem::path MoreIcons::getEditorDir(IconType type) {
    return Mod::get()->getConfigDir() / L("editor") / Constants::getFolderName(type);
}

std::filesystem::path MoreIcons::getIconDir(IconType type) {
    return Mod::get()->getConfigDir() / Constants::getFolderName(type);
}

std::filesystem::path MoreIcons::getIconStem(std::string_view name, IconType type) {
    return getIconDir(type) / Filesystem::strWide(name);
}

std::string MoreIcons::getIconName(int id, IconType type) {
    switch (type) {
        case IconType::Cube: return fmt::format("player_{:02}", id);
        case IconType::Ship: return fmt::format("ship_{:02}", id);
        case IconType::Ball: return fmt::format("player_ball_{:02}", id);
        case IconType::Ufo: return fmt::format("bird_{:02}", id);
        case IconType::Wave: return fmt::format("dart_{:02}", id);
        case IconType::Robot: return fmt::format("robot_{:02}", id);
        case IconType::Spider: return fmt::format("spider_{:02}", id);
        case IconType::Swing: return fmt::format("swing_{:02}", id);
        case IconType::Jetpack: return fmt::format("jetpack_{:02}", id);
        case IconType::DeathEffect: return fmt::format("PlayerExplosion_{:02}", id);
        case IconType::Special: return fmt::format("streak_{:02}", id);
        case IconType::ShipFire: return fmt::format("shipfire{:02}", id);
        default: return {};
    }
}

std::pair<std::string, std::string> MoreIcons::getIconPaths(int id, IconType type) {
    auto fileUtils = Get::FileUtils();
    std::string sheetName = Get::GameManager()->sheetNameForIcon(id, (int)type);
    return std::make_pair(
        std::string(fileUtils->fullPathForFilename(fmt::format("{}.png", sheetName).c_str(), false)),
        std::string(fileUtils->fullPathForFilename(fmt::format("{}.plist", sheetName).c_str(), false))
    );
}

std::string MoreIcons::getTrailTexture(int id) {
    return Get::FileUtils()->fullPathForFilename(fmt::format("streak_{:02}_001.png", id).c_str(), false);
}

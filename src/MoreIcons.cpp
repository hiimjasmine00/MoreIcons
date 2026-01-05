#include "MoreIcons.hpp"
#include "utils/Constants.hpp"
#include "utils/Filesystem.hpp"
#include "utils/Get.hpp"
#include "utils/Icons.hpp"
#include "utils/Json.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJGarageLayer.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/ui/TextInput.hpp>
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
    return Ok(std::move(trashPath));
}

bool MoreIcons::dualSelected() {
    return separateDualIcons && separateDualIcons->getSavedValue("2pselected", false);
}

ccColor3B MoreIcons::vanillaColor1(bool dual) {
    return Constants::getColor(dual && separateDualIcons ? separateDualIcons->getSavedValue("color1", 0) : Get::GameManager()->m_playerColor);
}

ccColor3B MoreIcons::vanillaColor2(bool dual) {
    return Constants::getColor(dual && separateDualIcons ? separateDualIcons->getSavedValue("color2", 0) : Get::GameManager()->m_playerColor2);
}

ccColor3B MoreIcons::vanillaColorGlow(bool dual) {
    return Constants::getColor(dual && separateDualIcons ? separateDualIcons->getSavedValue("colorglow", 0) : Get::GameManager()->m_playerGlowColor);
}

bool MoreIcons::vanillaGlow(bool dual) {
    return dual && separateDualIcons ? separateDualIcons->getSavedValue("glow", false) : Get::GameManager()->m_playerGlow;
}

int MoreIcons::vanillaIcon(IconType type, bool dual) {
    if (dual && separateDualIcons) {
        switch (type) {
            case IconType::Cube: return separateDualIcons->getSavedValue("cube", 1);
            case IconType::Ship: return separateDualIcons->getSavedValue("ship", 1);
            case IconType::Ball: return separateDualIcons->getSavedValue("roll", 1);
            case IconType::Ufo: return separateDualIcons->getSavedValue("bird", 1);
            case IconType::Wave: return separateDualIcons->getSavedValue("dart", 1);
            case IconType::Robot: return separateDualIcons->getSavedValue("robot", 1);
            case IconType::Spider: return separateDualIcons->getSavedValue("spider", 1);
            case IconType::Swing: return separateDualIcons->getSavedValue("swing", 1);
            case IconType::Jetpack: return separateDualIcons->getSavedValue("jetpack", 1);
            case IconType::DeathEffect: return separateDualIcons->getSavedValue("death", 1);
            case IconType::Special: return separateDualIcons->getSavedValue("trail", 1);
            case IconType::ShipFire: return separateDualIcons->getSavedValue("shiptrail", 1);
            default: return 0;
        }
    }
    else {
        auto gameManager = Get::GameManager();
        switch (type) {
            case IconType::Cube: return gameManager->m_playerFrame;
            case IconType::Ship: return gameManager->m_playerShip;
            case IconType::Ball: return gameManager->m_playerBall;
            case IconType::Ufo: return gameManager->m_playerBird;
            case IconType::Wave: return gameManager->m_playerDart;
            case IconType::Robot: return gameManager->m_playerRobot;
            case IconType::Spider: return gameManager->m_playerSpider;
            case IconType::Swing: return gameManager->m_playerSwing;
            case IconType::Jetpack: return gameManager->m_playerJetpack;
            case IconType::DeathEffect: return gameManager->m_playerDeathEffect;
            case IconType::Special: return gameManager->m_playerStreak;
            case IconType::ShipFire: return gameManager->m_playerShipFire;
            default: return 0;
        }
    }
}

void MoreIcons::updateGarage(GJGarageLayer* layer) {
    auto noLayer = layer == nullptr;
    if (noLayer) layer = Get::Director()->getRunningScene()->getChildByType<GJGarageLayer>(0);
    if (!layer) return;

    auto player1 = layer->m_playerObject;
    auto iconType1 = Get::GameManager()->m_playerIconType;
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
    if (Json::get(info->getSpecialInfo(), "blend", true)) streak->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
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

std::filesystem::path MoreIcons::getIconPath(IconInfo* info, int id, IconType type) {
    std::filesystem::path png;
    std::filesystem::path plist;
    getIconPaths(info, id, type, png, plist);
    return png;
}

std::filesystem::path getFullPath(CCFileUtils* fileUtils, const char* filename) {
    return Filesystem::strPath(std::string(fileUtils->fullPathForFilename(filename, false)));
}

void MoreIcons::getIconPaths(IconInfo* info, int id, IconType type, std::filesystem::path& png, std::filesystem::path& plist) {
    if (info) {
        png = info->getTexture();
        plist = info->getSheet();
        return;
    }

    auto fileUtils = Get::FileUtils();

    if (type == IconType::DeathEffect) {
        png = getFullPath(fileUtils, fmt::format("PlayerExplosion_{:02}.png", id).c_str());
        plist = getFullPath(fileUtils, fmt::format("PlayerExplosion_{:02}.plist", id).c_str());
        return;
    }
    else if (type == IconType::Special) {
        png = getFullPath(fileUtils, fmt::format("streak_{:02}_001.png", id).c_str());
        return;
    }
    else if (type == IconType::ShipFire) {
        png = getFullPath(fileUtils, fmt::format("shipfire{:02}_001.png", id).c_str());
        return;
    }

    auto iconName = getIconName(id, type);
    png = getFullPath(fileUtils, fmt::format("icons/{}.png", iconName).c_str());
    plist = getFullPath(fileUtils, fmt::format("icons/{}.plist", iconName).c_str());
}

std::string_view MoreIcons::getText(CCTextInputNode* input) {
    return *input->m_textField->m_pInputText;
}

std::string_view MoreIcons::getText(geode::TextInput* input) {
    return getText(input->getInputNode());
}

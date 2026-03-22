#include "MoreIcons.hpp"
#include "utils/Constants.hpp"
#include "utils/Filesystem.hpp"
#include "utils/Get.hpp"
#include "utils/Icons.hpp"
#include "utils/Json.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJGarageLayer.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/loader/Mod.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

Mod* MoreIcons::customGamemodeColors = nullptr;
Mod* MoreIcons::separateDualIcons = nullptr;

$on_mod(Loaded) {
    Icons::loadSettings();
    auto loader = Loader::get();
    if (auto separateDualIcons = loader->getInstalledMod("weebify.separate_dual_icons")) {
        if (separateDualIcons->isLoaded()) {
            MoreIcons::separateDualIcons = separateDualIcons;
        }
        else if (separateDualIcons->shouldLoad()) {
            ModStateEvent(ModEventType::Loaded, separateDualIcons).listen([separateDualIcons] {
                MoreIcons::separateDualIcons = separateDualIcons;
            }).leak();
        }
    }
    if (auto customGamemodeColors = loader->getInstalledMod("rooot.custom-gamemode-colors")) {
        if (customGamemodeColors->isLoaded()) {
            MoreIcons::customGamemodeColors = customGamemodeColors;
        }
        else if (customGamemodeColors->shouldLoad()) {
            ModStateEvent(ModEventType::Loaded, customGamemodeColors).listen([customGamemodeColors] {
                MoreIcons::customGamemodeColors = customGamemodeColors;
            }).leak();
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

namespace MoreIcons {
    ccColor3B customColor(IconType type, bool dual, std::string_view key, std::string_view dualKey, SeedValueRSV GameManager::* colorMember) {
        if (customGamemodeColors) {
            if (auto playerOverride = customGamemodeColors->getSaveContainer().get(
                dual && separateDualIcons ? "player2_override" : "player1_override"
            )) {
                std::string_view gamemode;
                switch (type) {
                    case IconType::Cube: gamemode = "cube"; break;
                    case IconType::Ship:
                    case IconType::Jetpack: gamemode = "ship"; break;
                    case IconType::Ball: gamemode = "ball"; break;
                    case IconType::Ufo: gamemode = "ufo"; break;
                    case IconType::Wave: gamemode = "wave"; break;
                    case IconType::Robot: gamemode = "robot"; break;
                    case IconType::Spider: gamemode = "spider"; break;
                    case IconType::Swing: gamemode = "swing"; break;
                    default: break;
                }
                if (!gamemode.empty()) {
                    if (auto colors = playerOverride.unwrap().get(gamemode)) {
                        auto& colorsValue = colors.unwrap();
                        if (Json::get(colorsValue, "enabled", false)) {
                            return Constants::getColor(Json::get(colorsValue, key, 0));
                        }
                    }
                }
            }
        }

        return Constants::getColor(dual && separateDualIcons ? separateDualIcons->getSavedValue(dualKey, 0) : Get::gameManager->*colorMember);
    }
}

ccColor3B MoreIcons::currentColor1(IconType type, bool dual) {
    return customColor(type, dual, "primary", "color1", &GameManager::m_playerColor);
}

ccColor3B MoreIcons::currentColor2(IconType type, bool dual) {
    return customColor(type, dual, "secondary", "color2", &GameManager::m_playerColor2);
}

ccColor3B MoreIcons::currentColorGlow(IconType type, bool dual) {
    return customColor(type, dual, "glow", "colorglow", &GameManager::m_playerGlowColor);
}

bool MoreIcons::currentGlow(bool dual) {
    return dual && separateDualIcons ? separateDualIcons->getSavedValue("glow", false) : Get::gameManager->m_playerGlow;
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
        switch (type) {
            case IconType::Cube: return Get::gameManager->m_playerFrame;
            case IconType::Ship: return Get::gameManager->m_playerShip;
            case IconType::Ball: return Get::gameManager->m_playerBall;
            case IconType::Ufo: return Get::gameManager->m_playerBird;
            case IconType::Wave: return Get::gameManager->m_playerDart;
            case IconType::Robot: return Get::gameManager->m_playerRobot;
            case IconType::Spider: return Get::gameManager->m_playerSpider;
            case IconType::Swing: return Get::gameManager->m_playerSwing;
            case IconType::Jetpack: return Get::gameManager->m_playerJetpack;
            case IconType::DeathEffect: return Get::gameManager->m_playerDeathEffect;
            case IconType::Special: return Get::gameManager->m_playerStreak;
            case IconType::ShipFire: return Get::gameManager->m_playerShipFire;
            default: return 0;
        }
    }
}

void MoreIcons::updateGarage(GJGarageLayer* layer) {
    auto noLayer = layer == nullptr;
    if (noLayer) layer = Get::director->getRunningScene()->getChildByType<GJGarageLayer>(0);
    if (!layer) return;

    auto player1 = layer->m_playerObject;
    auto iconType1 = Get::gameManager->m_playerIconType;
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

std::filesystem::path getFullPath(ZStringView filename) {
    #ifdef GEODE_IS_WINDOWS
    return std::filesystem::path(Filesystem::strWide(Get::fileUtils->fullPathForFilename(filename.c_str(), false)));
    #else
    return std::filesystem::path(std::string(Get::fileUtils->fullPathForFilename(filename.c_str(), false)));
    #endif
}

std::filesystem::path MoreIcons::getFirePath(IconInfo* info, int id, int frame) {
    return getFirePath(info, id, fmt::format("_{:03}", frame));
}

std::filesystem::path MoreIcons::getFirePath(IconInfo* info, int id, std::string_view frameSuffix) {
    if (info) {
        auto path = info->getTexture();
        auto& pathString = Filesystem::getPathString(path);
        pathString.replace(pathString.size() - 8, 4, Filesystem::strWide(frameSuffix));
        return path;
    }
    else {
        return getFullPath(fmt::format("shipfire{:02}{}.png", id, frameSuffix));
    }
}

void MoreIcons::getIconPaths(IconInfo* info, int id, IconType type, std::filesystem::path& png, std::filesystem::path& plist) {
    if (info) {
        png = info->getTexture();
        plist = info->getSheet();
        return;
    }

    if (type == IconType::DeathEffect) {
        png = getFullPath(fmt::format("PlayerExplosion_{:02}.png", id - 1));
        plist = getFullPath(fmt::format("PlayerExplosion_{:02}.plist", id - 1));
        return;
    }
    else if (type == IconType::Special) {
        png = getFullPath(fmt::format("streak_{:02}_001.png", id));
        return;
    }
    else if (type == IconType::ShipFire) {
        png = getFullPath(fmt::format("shipfire{:02}_001.png", id));
        return;
    }

    auto iconName = getIconName(id, type);
    png = getFullPath(fmt::format("icons/{}.png", iconName));
    plist = getFullPath(fmt::format("icons/{}.plist", iconName));
}

void MoreIcons::loadFromSave(IconType type) {
    auto saveContainer = Mod::get()->getSaveContainer();
    auto& mainIcon = saveContainer[more_icons::saveKey(type, false)];
    if (auto str = mainIcon.asString()) {
        more_icons::setIcon(more_icons::getIcon(str.unwrap(), type), type, false);
    }
    else {
        mainIcon = std::string();
    }

    if (separateDualIcons) {
        auto& dualIcon = saveContainer[more_icons::saveKey(type, true)];
        if (auto str = dualIcon.asString()) {
            more_icons::setIcon(more_icons::getIcon(str.unwrap(), type), type, true);
        }
        else {
            dualIcon = std::string();
        }
    }
}

void MoreIcons::setTexture(CCSprite* sprite, CCTexture2D* texture) {
    if (texture) {
        sprite->setTexture(texture);
        sprite->setTextureRect({ { 0.0f, 0.0f }, texture->getContentSize() });
    }
    else {
        sprite->setTexture(nullptr);
        sprite->setTextureRect({ 0.0f, 0.0f, 0.0f, 0.0f });
    }
}

CCSpriteFrame* MoreIcons::frameWithTexture(CCTexture2D* texture) {
    return CCSpriteFrame::createWithTexture(texture, { { 0.0f, 0.0f }, texture->getContentSize() });
}

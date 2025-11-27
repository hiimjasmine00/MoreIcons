#include "MoreIcons.hpp"
#include "api/MoreIconsAPI.hpp"
#include "classes/misc/ThreadPool.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJGarageLayer.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Mod.hpp>
#include <geode.texture-loader/include/TextureLoader.hpp>
#include <jasmine/convert.hpp>
#include <jasmine/setting.hpp>
#include <std23/function_ref.h>
#include <texpack.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
    MoreIcons::loadSettings();
}

$on_mod(DataSaved) {
    MoreIcons::saveTrails();
}

std::vector<LogData> MoreIcons::logs;
std::map<IconType, int> MoreIcons::severities = {
    { IconType::Cube, 0 },
    { IconType::Ship, 0 },
    { IconType::Ball, 0 },
    { IconType::Ufo, 0 },
    { IconType::Wave, 0 },
    { IconType::Robot, 0 },
    { IconType::Spider, 0 },
    { IconType::Swing, 0 },
    { IconType::Jetpack, 0 },
    { IconType::Special, 0 }
};
int MoreIcons::severity = 0;
bool MoreIcons::traditionalPacks = true;

std::unordered_map<std::string, Severity::type> severityMap = {
    { "Debug", Severity::Debug },
    { "Info", Severity::Info },
    { "Warning", Severity::Warning },
    { "Error", Severity::Error },
    { "None", Severity::cast(4) }
};

void MoreIcons::loadSettings() {
    auto logLevel = jasmine::setting::get<std::string>("log-level");
    auto mod = Mod::get();
    if (!mod->setSavedValue("migrated-log-level", true)) {
        auto& data = mod->getSavedSettingsData();
        if (!data.get<bool>("info-logs").unwrapOr(true)) logLevel->setValue("Warning");
        else if (!data.get<bool>("debug-logs").unwrapOr(true)) logLevel->setValue("Info");
        else logLevel->setValue("Debug");
    }
    mod->setLogLevel(severityMap[logLevel->getValue()]);
    traditionalPacks = jasmine::setting::getValue<bool>("traditional-packs");
    MoreIconsAPI::preloadIcons = jasmine::setting::getValue<bool>("preload-icons");
}

bool MoreIcons::doesExist(const std::filesystem::path& path) {
    std::error_code code;
    auto exists = std::filesystem::exists(path, code);
    if (code) log::error("{}: Failed to check existence: {}", path, code.message());
    return exists;
}

Result<> MoreIcons::renameFile(const std::filesystem::path& from, const std::filesystem::path& to, bool overwrite, bool copy) {
    std::error_code code;
    if (!MoreIcons::doesExist(from)) return Ok();
    if (overwrite && MoreIcons::doesExist(to)) {
        if (!std::filesystem::remove(to, code)) return Err("Failed to remove {}: {}", to.filename(), code.message());
    }
    if (copy) {
        auto dest = to;
        for (int i = 1; MoreIcons::doesExist(dest); i++) {
            dest.replace_filename(fmt::format("{} ({}){}", dest.stem(), i, dest.extension()));
        }
        std::filesystem::rename(from, dest, code);
    }
    else std::filesystem::rename(from, to, code);
    return code ? Err("Failed to rename {}: {}", from.filename(), code.message()) : Result<>(Ok());
}

void directoryIterator(
    const std::filesystem::path& path, std::filesystem::file_type type, std23::function_ref<bool(const std::filesystem::path&)> func
) {
    std::error_code code;
    std::filesystem::directory_iterator it(path, code);
    if (code) return log::error("{}: Failed to create directory iterator: {}", path, code.message());
    for (; it != std::filesystem::end(it); it.increment(code)) {
        std::error_code code;
        if (it->status(code).type() != type) continue;
        auto& itPath = it->path();
        if (func(itPath)) break;
    }
    if (code) return log::error("{}: Failed to iterate over directory: {}", path, code.message());
}

void migrateFolderIcons(const std::filesystem::path& path) {
    log::info("Beginning folder icon migration in {}", path);

    auto& saveContainer = Mod::get()->getSaveContainer();
    if (!saveContainer.contains("migrated-folders")) saveContainer.set("migrated-folders", matjson::Value::array());

    auto& migratedFolders = saveContainer["migrated-folders"];

    for (int i = 0; i < 9; i++) {
        auto folderPath = path / MoreIcons::folders[i];
        if (!MoreIcons::doesExist(folderPath)) continue;

        directoryIterator(folderPath, std::filesystem::file_type::directory, [i, &folderPath, &migratedFolders](const std::filesystem::path& path) {
            if (std::ranges::contains(migratedFolders, path)) return false;
            else migratedFolders.push(path);

            std::vector<std::string> names;

            directoryIterator(path, std::filesystem::file_type::regular, [i, &names](const std::filesystem::path& path) {
                if (path.extension() != ".png") return false;

                auto pathFilename = string::pathToString(path.filename());
                auto pathStem = string::pathToString(path.stem());
                if (pathStem.ends_with("_2_001")) {
                    if ((i != 5 && i != 6) || pathStem.ends_with("_01_2_001") || pathStem.ends_with("_02_2_001") ||
                        pathStem.ends_with("_03_2_001") || pathStem.ends_with("_04_2_001")) names.push_back(pathFilename);
                }
                else if (i == 3 && pathStem.ends_with("_3_001")) names.push_back(pathFilename);
                else if (pathStem.ends_with("_extra_001")) {
                    if ((i != 5 && i != 6) || pathStem.ends_with("_01_extra_001")) names.push_back(pathFilename);
                }
                else if (pathStem.ends_with("_glow_001")) {
                    if ((i != 5 && i != 6) || pathStem.ends_with("_01_glow_001") || pathStem.ends_with("_02_glow_001") ||
                        pathStem.ends_with("_03_glow_001") || pathStem.ends_with("_04_glow_001")) names.push_back(pathFilename);
                }
                else if (pathStem.ends_with("_001")) {
                    if ((i != 5 && i != 6) || pathStem.ends_with("_01_001") || pathStem.ends_with("_02_001") ||
                        pathStem.ends_with("_03_001") || pathStem.ends_with("_04_001")) names.push_back(pathFilename);
                }
                return false;
            });

            if (names.empty()) return false;

            texpack::Packer packer;

            for (auto& filename : names) {
                if (auto res = packer.frame(filename, path / filename); res.isErr()) {
                    log::error("{}: Failed to load frame {}: {}", path, filename, res.unwrapErr());
                    return false;
                }
            }

            if (auto res = packer.pack(); res.isErr()) {
                log::error("{}: Failed to pack frames: {}", path, res.unwrapErr());
                return false;
            }

            auto pngPath = folderPath / path.filename().concat(".png");
            if (auto res = MoreIcons::renameFile(pngPath, folderPath / path.filename().concat(".png")); res.isErr()) {
                log::error("{}: Failed to rename existing image: {}", path, res.unwrapErr());
            }
            if (auto res = packer.png(pngPath); res.isErr()) {
                log::error("{}: Failed to save image: {}", path, res.unwrapErr());
            }

            auto plistPath = folderPath / path.filename().concat(".plist");
            if (auto res = MoreIcons::renameFile(plistPath, folderPath / path.filename().concat(".plist")); res.isErr()) {
                log::error("{}: Failed to rename existing plist: {}", path, res.unwrapErr());
            }
            if (auto res = packer.plist(plistPath, fmt::format("icons/{}", string::pathToString(pngPath.filename())), "    "); res.isErr()) {
                log::error("{}: Failed to save plist: {}", path, res.unwrapErr());
            }

            return false;
        });
    }

    log::info("Finished folder icon migration in {}", path);
}

struct IconPack {
    std::string name;
    std::string id;
    std::filesystem::path path;
    bool vanilla;
    bool zipped;
};

std::vector<IconPack> packs;

void MoreIcons::loadPacks() {
    packs.clear();
    packs.emplace_back("More Icons", "", dirs::getGeodeDir(), false, false);
    migrateFolderIcons(Mod::get()->getConfigDir());

    for (auto& pack : texture_loader::getAppliedPacks()) {
        auto extension = pack.path.extension();
        auto zipped = extension == ".apk" || extension == ".zip";
        if (traditionalPacks) {
            if (doesExist(pack.resourcesPath / "icons")) {
                packs.emplace_back(std::move(pack.name), std::move(pack.id), std::move(pack.resourcesPath), true, zipped);
            }
            else directoryIterator(pack.resourcesPath, std::filesystem::file_type::regular, [&pack, zipped](const std::filesystem::path& path) {
                if (path.extension() != ".png") return false;

                auto filename = string::pathToString(path.filename());
                if (filename.starts_with("streak_") && filename.ends_with("_001.png")) {
                    packs.emplace_back(std::move(pack.name), std::move(pack.id), path.parent_path(), true, zipped);
                    return true;
                }

                return false;
            });
        }

        auto configPath = pack.resourcesPath / "config" / GEODE_MOD_ID;
        if (doesExist(configPath)) {
            packs.emplace_back(std::move(pack.name), std::move(pack.id), std::move(pack.resourcesPath), false, zipped);
            migrateFolderIcons(configPath);
        }
    }
}

std::string MoreIcons::vanillaTexturePath(const std::string& path, bool skipSuffix) {
    #ifdef GEODE_IS_MOBILE
    if (!skipSuffix && MoreIconsAPI::getDirector()->getContentScaleFactor() >= 4.0f) {
        if (auto highGraphicsMobile = Loader::get()->getLoadedMod("weebify.high-graphics-android")) {
            auto configDir = highGraphicsMobile->getConfigDir(false) / GEODE_GD_VERSION_STRING;
            if (doesExist(configDir)) return configDir / path;
        }
        return path;
    }
    #endif
    return string::pathToString(dirs::getResourcesDir() / path);
}

Result<std::filesystem::path> MoreIcons::createTrash() {
    auto trashPath = Mod::get()->getConfigDir() / "trash";
    GEODE_UNWRAP(file::createDirectoryAll(trashPath));
    std::error_code code;
    std::filesystem::permissions(trashPath, std::filesystem::perms::all, code);
    return Ok(trashPath);
}

std::string replaceEnd(const std::string& str, size_t end, std::string_view replace) {
    return str.substr(0, str.size() - end).append(replace);
}

IconType currentType = IconType::Cube;

template <typename... Args>
void printLog(const std::string& name, int severity, fmt::format_string<Args...> message, Args&&... args) {
    std::string logMessage = fmt::format(message, std::forward<Args>(args)...);
    log::logImpl(Severity::cast(severity), Mod::get(), "{}: {}", name, logMessage);
    MoreIcons::logs.emplace_back(name, std::move(logMessage), currentType, severity);
    auto& currentSeverity = MoreIcons::severities[currentType];
    if (currentSeverity < severity) currentSeverity = severity;
    if (MoreIcons::severity < severity) MoreIcons::severity = severity;
}

void loadIcon(const std::filesystem::path& path, const IconPack& pack) {
    auto factor = MoreIconsAPI::getDirector()->getContentScaleFactor();
    auto pathFilename = string::pathToString(path.filename());
    auto pathStem = string::pathToString(path.stem());
    auto pathString = string::pathToString(path);
    std::string name;
    std::string shortName;
    if (pathFilename.ends_with("-uhd.plist")) {
        shortName = replaceEnd(pathStem, 4, "");
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        if (factor < 4.0f && factor >= 2.0f) {
            if (!MoreIcons::doesExist(replaceEnd(pathString, 10, "-hd.plist")) && !MoreIcons::doesExist(replaceEnd(pathString, 9, ".plist"))) {
                printLog(name, Severity::Warning, "Ignoring high-quality icon on medium texture quality");
            }
            return;
        }
        else if (factor < 2.0f) {
            if (!MoreIcons::doesExist(replaceEnd(pathString, 10, ".plist"))) {
                printLog(name, Severity::Warning, "Ignoring high-quality icon on low texture quality");
            }
            return;
        }
    }
    else if (pathFilename.ends_with("-hd.plist")) {
        shortName = replaceEnd(pathStem, 3, "");
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        if (factor < 2.0f) {
            if (!MoreIcons::doesExist(replaceEnd(pathString, 9, ".plist"))) {
                printLog(name, Severity::Warning, "Ignoring medium-quality icon on low texture quality");
            }
            return;
        }

        if (MoreIcons::doesExist(replaceEnd(pathString, 9, "-uhd.plist")) && factor >= 4.0f) return;
    }
    else {
        shortName = pathStem;
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        if (MoreIcons::doesExist(replaceEnd(pathString, 6, "-uhd.plist")) && factor >= 4.0f) return;
        else if (MoreIcons::doesExist(replaceEnd(pathString, 6, "-hd.plist")) && factor >= 2.0f) return;
    }

    log::debug("Pre-loading icon {} from {}", name, pack.name);

    if (pathString.empty() && !path.empty()) printLog(name, Severity::Error, "More Icons only supports UTF-8 paths");

    auto texturePath = replaceEnd(pathString, 6, ".png");
    if (!MoreIcons::doesExist(texturePath)) {
        return printLog(name, Severity::Error, "Texture file {}.png not found", pathStem);
    }

    MoreIconsAPI::addIcon(name, shortName, currentType, texturePath, pathString, pack.id, pack.name, 0, {}, false, pack.zipped);

    log::debug("Finished pre-loading icon {} from {}", name, pack.name);
}

void loadVanillaIcon(const std::filesystem::path& path, const IconPack& pack) {
    auto factor = MoreIconsAPI::getDirector()->getContentScaleFactor();
    auto pathFilename = string::pathToString(path.filename());
    auto pathStem = string::pathToString(path.stem());
    auto pathString = string::pathToString(path);
    std::string name;
    std::string shortName;
    if (pathFilename.ends_with("-uhd.png")) {
        if (factor < 4.0f) return;
        shortName = replaceEnd(pathStem, 4, "");
        name = fmt::format("{}:{}", pack.id, shortName);
    }
    else if (pathFilename.ends_with("-hd.png")) {
        if (factor < 2.0f) return;
        if (MoreIcons::doesExist(replaceEnd(pathString, 7, "-uhd.png")) && factor >= 4.0f) return;
        shortName = replaceEnd(pathStem, 3, "");
        name = fmt::format("{}:{}", pack.id, shortName);
    }
    else {
        if (MoreIcons::doesExist(replaceEnd(pathString, 4, "-uhd.png")) && factor >= 4.0f) return;
        else if (MoreIcons::doesExist(replaceEnd(pathString, 4, "-hd.png")) && factor >= 2.0f) return;
        shortName = pathStem;
        name = fmt::format("{}:{}", pack.id, shortName);
    }

    log::debug("Pre-loading vanilla icon {} from {}", name, pack.name);

    if (pathString.empty() && !path.empty()) printLog(name, Severity::Error, "More Icons only supports UTF-8 paths");

    auto plistPath = replaceEnd(pathString, 4, ".plist");
    if (!MoreIcons::doesExist(plistPath)) {
        plistPath = MoreIcons::vanillaTexturePath(fmt::format("icons/{}.plist", pathStem), false);
    }
    if (!MoreIconsAPI::getFileUtils()->isFileExist(plistPath)) {
        return printLog(name, Severity::Error, "Plist file not found (Last attempt: {})", plistPath);
    }

    MoreIconsAPI::addIcon(name, shortName, currentType, pathString, plistPath, pack.id, pack.name, 0, {}, true, pack.zipped);

    log::debug("Finished pre-loading vanilla icon {} from {}", name, pack.name);
}

void loadTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto pathStem = string::pathToString(path.stem());
    auto pathString = string::pathToString(path);
    auto name = pack.id.empty() ? pathStem : fmt::format("{}:{}", pack.id, pathStem);

    log::debug("Pre-loading trail {} from {}", name, pack.name);

    if (pathString.empty() && !path.empty()) printLog(name, Severity::Error, "More Icons only supports UTF-8 paths");

    MoreIconsAPI::addIcon(name, pathStem, IconType::Special, pathString, "", pack.id, pack.name, 0,
        file::readFromJson<TrailInfo>(std::filesystem::path(path).replace_extension(".json")).unwrapOrDefault(), false, pack.zipped);

    log::debug("Finished pre-loading trail {} from {}", name, pack.name);
}

void loadVanillaTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto pathStem = string::pathToString(path.stem());
    auto pathString = string::pathToString(path);
    auto name = fmt::format("{}:{}", pack.id, pathStem);

    log::debug("Pre-loading vanilla trail {} from {}", name, pack.name);

    if (pathString.empty() && !path.empty()) printLog(name, Severity::Error, "More Icons only supports UTF-8 paths");

    auto trailID = jasmine::convert::getInt<int>(std::string_view(pathStem).substr(7, pathStem.size() - 11)).value_or(0);
    if (trailID == 0) trailID = -1;

    TrailInfo trailInfo;
    trailInfo.blend = true;
    switch (trailID) {
        case 1:
            trailInfo.tint = true;
            trailInfo.stroke = 10.0f;
            break;
        case 3:
            trailInfo.tint = true;
            trailInfo.stroke = 8.5f;
            break;
        case 4:
            trailInfo.tint = true;
            trailInfo.fade = 0.4f;
            trailInfo.stroke = 10.0f;
            break;
        case 5:
            trailInfo.tint = true;
            trailInfo.show = true;
            trailInfo.fade = 0.6f;
            trailInfo.stroke = 5.0f;
            break;
        case 6:
            trailInfo.tint = true;
            trailInfo.show = true;
            trailInfo.fade = 1.0f;
            trailInfo.stroke = 3.0f;
            break;
    }

    MoreIconsAPI::addIcon(name, pathStem, IconType::Special, pathString, "", pack.id, pack.name, trailID, trailInfo, true, pack.zipped);

    log::debug("Finished pre-loading vanilla trail {} from {}", name, pack.name);
}

void MoreIcons::loadIcons(IconType type) {
    currentType = type;

    auto miType = MoreIconsAPI::convertType(type);
    auto prefix = MoreIconsAPI::prefixes[miType];
    auto folder = folders[miType];
    auto name = MoreIconsAPI::lowercase[miType];

    for (int i = 0; i < packs.size(); i++) {
        auto& pack = packs[i];

        if (!pack.vanilla) {
            auto path = pack.path / "config" / GEODE_MOD_ID / folder;
            if (!doesExist(path)) {
                if (i == 0) {
                    if (auto res = file::createDirectoryAll(path); res.isErr()) log::error("{}: {}", path, res.unwrapErr());
                }
                continue;
            }

            if (i == 0) {
                std::error_code code;
                std::filesystem::permissions(path, std::filesystem::perms::all, code);
                if (code) log::error("{}: Failed to change permissions: {}", path, code.message());
            }

            log::info("Pre-loading {}s from {}", name, path);

            directoryIterator(path, std::filesystem::file_type::regular, [type, &pack](const std::filesystem::path& path) {
                if (type <= IconType::Jetpack) {
                    if (path.extension() == ".plist") loadIcon(path, pack);
                }
                else if (type == IconType::Special) {
                    if (path.extension() == ".png") loadTrail(path, pack);
                }
                return false;
            });

            log::info("Finished pre-loading {}s from {}", name, path);
        }
        else if (traditionalPacks) {
            auto path = type == IconType::Special ? pack.path : pack.path / "icons";
            if (!doesExist(path)) continue;

            log::info("Pre-loading {}s from {}", name, path);

            directoryIterator(path, std::filesystem::file_type::regular, [type, &pack, prefix](const std::filesystem::path& path) {
                if (path.extension() != ".png") return false;

                auto filename = string::pathToString(path.filename());
                if (!filename.starts_with(prefix)) return false;

                if (type <= IconType::Jetpack) {
                    if (type != IconType::Cube || !filename.starts_with("player_ball_")) {
                        auto suffix = filename.substr(strlen(prefix));
                        if (suffix != "00.png" && suffix != "00-hd.png" && suffix != "00-uhd.png") loadVanillaIcon(path, pack);
                    }
                }
                else if (type == IconType::Special) {
                    if (filename.ends_with("_001.png")) loadVanillaTrail(path, pack);
                }
                return false;
            });

            log::info("Finished pre-loading {}s from {}", name, path);
        }
    }

    MoreIconsAPI::loadIcons(type);

    if (type == IconType::Special) {
        packs.clear();
        std::ranges::sort(logs, [](const LogData& a, const LogData& b) {
            return a.severity == b.severity ? a.name < b.name : a.severity > b.severity;
        });
    }
}

void MoreIcons::saveTrails() {
    for (auto& info : MoreIconsAPI::icons[IconType::Special]) {
        if (info.trailID == 0) {
            if (auto res = file::writeToJson(std::filesystem::path(info.textures[0]).replace_extension(".json"), info.trailInfo); res.isErr()) {
                log::error("{}: Failed to save trail info: {}", info.name, res.unwrapErr());
            }
        }
    }
}

ColorInfo MoreIcons::vanillaColors(bool dual) {
    auto gameManager = MoreIconsAPI::getGameManager();
    auto sdi = dual ? Loader::get()->getLoadedMod("weebify.separate_dual_icons") : nullptr;
    return {
        .color1 = gameManager->colorForIdx(sdi ? sdi->getSavedValue("color1", 0) : gameManager->m_playerColor),
        .color2 = gameManager->colorForIdx(sdi ? sdi->getSavedValue("color2", 0) : gameManager->m_playerColor2),
        .colorGlow = gameManager->colorForIdx(sdi ? sdi->getSavedValue("colorglow", 0) : gameManager->m_playerGlowColor),
        .glow = sdi ? sdi->getSavedValue("glow", false) : gameManager->m_playerGlow
    };
}

int MoreIcons::vanillaIcon(IconType type, bool dual) {
    auto gameManager = MoreIconsAPI::getGameManager();
    auto sdi = dual ? Loader::get()->getLoadedMod("weebify.separate_dual_icons") : nullptr;
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
    if (noLayer) layer = MoreIconsAPI::getDirector()->getRunningScene()->getChildByType<GJGarageLayer>(0);
    if (!layer) return;

    auto gameManager = MoreIconsAPI::getGameManager();
    auto player1 = layer->m_playerObject;
    auto iconType1 = gameManager->m_playerIconType;
    if (noLayer) player1->updatePlayerFrame(vanillaIcon(iconType1, false), iconType1);
    MoreIconsAPI::updateSimplePlayer(player1, iconType1, false);

    if (auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons")) {
        auto player2 = static_cast<SimplePlayer*>(layer->getChildByID("player2-icon"));
        auto iconType2 = (IconType)sdi->getSavedValue("lastmode", 0);
        if (noLayer) player2->updatePlayerFrame(vanillaIcon(iconType2, true), iconType2);
        MoreIconsAPI::updateSimplePlayer(player2, iconType2, true);
    }

    if (noLayer) layer->selectTab(layer->m_iconType);
}

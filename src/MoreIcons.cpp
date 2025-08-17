#include "MoreIcons.hpp"
#include "api/MoreIconsAPI.hpp"
#include "classes/misc/ThreadPool.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJGarageLayer.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Mod.hpp>
#include <geode.texture-loader/include/TextureLoader.hpp>
#include <texpack.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
    MoreIcons::loadSettings();
}

$on_mod(DataSaved) {
    MoreIcons::saveTrails();
}

bool debugLogs = true;
bool infoLogs = true;

void MoreIcons::loadSettings() {
    auto mod = Mod::get();
    debugLogs = mod->getSettingValue<bool>("debug-logs");
    infoLogs = mod->getSettingValue<bool>("info-logs");
    traditionalPacks = mod->getSettingValue<bool>("traditional-packs");
    MoreIconsAPI::preloadIcons = mod->getSettingValue<bool>("preload-icons");
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

void directoryIterator(const std::filesystem::path& path, auto&& func) {
    std::error_code code;
    std::filesystem::directory_iterator it(path, code);
    if (code) return log::error("{}: Failed to create directory iterator: {}", path, code.message());
    for (; it != std::filesystem::end(it); it.increment(code)) {
        std::error_code code;
        auto& itPath = it->path();
        auto type = it->status(code).type();
        if constexpr (std::is_void_v<decltype(func(itPath, type))>) func(itPath, type);
        else if (func(itPath, type)) break;
    }
    if (code) return log::error("{}: Failed to iterate over directory: {}", path, code.message());
}

template <typename... Args>
void safeDebug(fmt::format_string<Args...> message, Args&&... args) {
    if (debugLogs) log::debug(message, std::forward<Args>(args)...);
}

template <typename... Args>
void safeInfo(fmt::format_string<Args...> message, Args&&... args) {
    if (infoLogs) log::info(message, std::forward<Args>(args)...);
}

void migrateFolderIcons(const std::filesystem::path& path) {
    safeInfo("Beginning folder icon migration in {}", path);

    auto& saveContainer = Mod::get()->getSaveContainer();
    if (!saveContainer.contains("migrated-folders")) saveContainer.set("migrated-folders", matjson::Value::array());

    auto& migratedFolders = saveContainer["migrated-folders"];

    for (int i = 0; i < 9; i++) {
        auto folderPath = path / MoreIcons::folders[i];
        if (!MoreIcons::doesExist(folderPath)) continue;

        directoryIterator(folderPath, [i, &folderPath, &migratedFolders](const std::filesystem::path& path, std::filesystem::file_type fileType) {
            if (fileType != std::filesystem::file_type::directory) return;

            if (std::ranges::find(migratedFolders, path) == migratedFolders.end()) migratedFolders.push(path);
            else return;

            std::vector<std::string> names;

            directoryIterator(path, [i, &names](const std::filesystem::path& path, std::filesystem::file_type fileType) {
                if (fileType != std::filesystem::file_type::regular || path.extension() != ".png") return;

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
            });

            if (names.empty()) return;

            texpack::Packer packer;

            for (auto& filename : names) {
                if (GEODE_UNWRAP_IF_ERR(err, packer.frame(filename, path / filename))) {
                    return log::error("{}: Failed to load frame {}: {}", path, filename, err);
                }
            }

            if (GEODE_UNWRAP_IF_ERR(err, packer.pack())) {
                return log::error("{}: Failed to pack frames: {}", path, err);
            }

            auto pngPath = folderPath / path.filename().concat(".png");
            MoreIcons::renameFile(pngPath, folderPath / path.filename().concat(".png")).inspectErr([&path](const std::string& err) {
                log::error("{}: Failed to rename existing image: {}", path, err);
            });
            packer.png(pngPath).inspectErr([&path](const std::string& err) {
                log::error("{}: Failed to save image: {}", path, err);
            });

            auto plistPath = folderPath / path.filename().concat(".plist");
            MoreIcons::renameFile(plistPath, folderPath / path.filename().concat(".plist")).inspectErr([&path](const std::string& err) {
                log::error("{}: Failed to rename existing plist: {}", path, err);
            });
            packer.plist(plistPath, "icons/" + string::pathToString(pngPath.filename()), "    ").inspectErr([&path](const std::string& err) {
                log::error("{}: Failed to save plist: {}", path, err);
            });
        });
    }

    safeInfo("Finished folder icon migration in {}", path);
}

void MoreIcons::loadPacks() {
    packs.clear();
    packs.emplace_back("More Icons", "", dirs::getGeodeDir(), false, false);
    migrateFolderIcons(Mod::get()->getConfigDir());

    for (auto& pack : texture_loader::getAppliedPacks()) {
        auto extension = pack.path.extension();
        auto zipped = extension == ".apk" || extension == ".zip";
        if (traditionalPacks) {
            if (doesExist(pack.resourcesPath / "icons")) packs.emplace_back(pack.name, pack.id, pack.resourcesPath, true, zipped);
            else directoryIterator(pack.resourcesPath, [&pack, zipped](const std::filesystem::path& path, std::filesystem::file_type fileType) {
                if (fileType != std::filesystem::file_type::regular) return false;

                if (path.extension() != ".png") return false;

                auto filename = string::pathToString(path.filename());
                if (filename.starts_with("streak_") && filename.ends_with("_001.png")) {
                    packs.emplace_back(pack.name, pack.id, path.parent_path(), true, zipped);
                    return true;
                }

                return false;
            });
        }

        auto configPath = pack.resourcesPath / "config" / GEODE_MOD_ID;
        if (doesExist(configPath)) {
            packs.emplace_back(pack.name, pack.id, pack.resourcesPath, false, zipped);
            migrateFolderIcons(configPath);
        }
    }
}

std::string MoreIcons::vanillaTexturePath(const std::string& path, bool skipSuffix) {
    #ifdef GEODE_IS_MOBILE
    if (!skipSuffix && MoreIconsAPI::get<CCDirector>()->getContentScaleFactor() >= 4.0f) {
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
    auto logMessage = fmt::format(message, std::forward<Args>(args)...);
    log::logImpl(Severity::cast(severity), Mod::get(), "{}: {}", name, logMessage);
    MoreIcons::logs.emplace_back(name, logMessage, currentType, severity);
    auto& currentSeverity = MoreIcons::severities[currentType];
    if (currentSeverity < severity) currentSeverity = severity;
    if (MoreIcons::severity < severity) MoreIcons::severity = severity;
}

void loadIcon(const std::filesystem::path& path, const IconPack& pack) {
    auto factor = MoreIconsAPI::get<CCDirector>()->getContentScaleFactor();
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

    safeDebug("Pre-loading icon {} from {}", name, pack.name);

    if (pathString.empty() && !path.empty()) printLog(name, Severity::Error, "More Icons only supports UTF-8 paths");

    auto texturePath = replaceEnd(pathString, 6, ".png");
    if (!MoreIcons::doesExist(texturePath)) {
        return printLog(name, Severity::Error, "Texture file {}.png not found", pathStem);
    }

    MoreIconsAPI::addIcon(name, shortName, currentType, texturePath, pathString, pack.id, pack.name, 0, {}, false, pack.zipped);

    safeDebug("Finished pre-loading icon {} from {}", name, pack.name);
}

void loadVanillaIcon(const std::filesystem::path& path, const IconPack& pack) {
    auto factor = MoreIconsAPI::get<CCDirector>()->getContentScaleFactor();
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

    safeDebug("Pre-loading vanilla icon {} from {}", name, pack.name);

    if (pathString.empty() && !path.empty()) printLog(name, Severity::Error, "More Icons only supports UTF-8 paths");

    auto plistPath = replaceEnd(pathString, 4, ".plist");
    if (!MoreIcons::doesExist(plistPath)) {
        plistPath = MoreIcons::vanillaTexturePath(fmt::format("icons/{}.plist", pathStem), false);
    }
    if (!MoreIconsAPI::get<CCFileUtils>()->isFileExist(plistPath)) {
        return printLog(name, Severity::Error, "Plist file not found (Last attempt: {})", plistPath);
    }

    MoreIconsAPI::addIcon(name, shortName, currentType, pathString, plistPath, pack.id, pack.name, 0, {}, true, pack.zipped);

    safeDebug("Finished pre-loading vanilla icon {} from {}", name, pack.name);
}

void loadTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto pathStem = string::pathToString(path.stem());
    auto pathString = string::pathToString(path);
    auto name = pack.id.empty() ? pathStem : fmt::format("{}:{}", pack.id, pathStem);

    safeDebug("Pre-loading trail {} from {}", name, pack.name);

    if (pathString.empty() && !path.empty()) printLog(name, Severity::Error, "More Icons only supports UTF-8 paths");

    MoreIconsAPI::addIcon(name, pathStem, IconType::Special, pathString, "", pack.id, pack.name, 0,
        file::readFromJson<TrailInfo>(std::filesystem::path(path).replace_extension(".json")).unwrapOrDefault(), false, pack.zipped);

    safeDebug("Finished pre-loading trail {} from {}", name, pack.name);
}

void loadVanillaTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto pathStem = string::pathToString(path.stem());
    auto pathString = string::pathToString(path);
    auto name = fmt::format("{}:{}", pack.id, pathStem);

    safeDebug("Pre-loading vanilla trail {} from {}", name, pack.name);

    if (pathString.empty() && !path.empty()) printLog(name, Severity::Error, "More Icons only supports UTF-8 paths");

    auto trailID = 0;
    std::from_chars(pathStem.data() + 7, pathStem.data() + (pathStem.size() - 4), trailID);
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

    safeDebug("Finished pre-loading vanilla trail {} from {}", name, pack.name);
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
                    if (GEODE_UNWRAP_IF_ERR(err, file::createDirectoryAll(path))) log::error("{}: {}", path, err);
                }
                continue;
            }

            if (i == 0) {
                std::error_code code;
                std::filesystem::permissions(path, std::filesystem::perms::all, code);
                if (code) log::error("{}: Failed to change permissions: {}", path, code.message());
            }

            safeInfo("Pre-loading {}s from {}", name, path);

            directoryIterator(path, [type, &pack](const std::filesystem::path& path, std::filesystem::file_type fileType) {
                if (fileType != std::filesystem::file_type::regular) return;

                if (type <= IconType::Jetpack) {
                    if (path.extension() == ".plist") loadIcon(path, pack);
                }
                else if (type == IconType::Special) {
                    if (path.extension() == ".png") loadTrail(path, pack);
                }
            });

            safeInfo("Finished pre-loading {}s from {}", name, path);
        }
        else if (traditionalPacks) {
            auto path = type == IconType::Special ? pack.path : pack.path / "icons";
            if (!doesExist(path)) continue;

            safeInfo("Pre-loading {}s from {}", name, path);

            directoryIterator(path, [type, &pack, prefix](const std::filesystem::path& path, std::filesystem::file_type fileType) {
                if (fileType != std::filesystem::file_type::regular) return;

                if (path.extension() != ".png") return;

                auto filename = string::pathToString(path.filename());
                if (!filename.starts_with(prefix)) return;

                if (type <= IconType::Jetpack) {
                    if (type != IconType::Cube || !filename.starts_with("player_ball_")) {
                        auto suffix = filename.substr(strlen(prefix));
                        if (suffix != "00.png" && suffix != "00-hd.png" && suffix != "00-uhd.png") loadVanillaIcon(path, pack);
                    }
                }
                else if (type == IconType::Special) {
                    if (filename.ends_with("_001.png")) loadVanillaTrail(path, pack);
                }
            });

            safeInfo("Finished pre-loading {}s from {}", name, path);
        }
    }

    MoreIconsAPI::loadIcons(type, infoLogs);

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
            file::writeToJson(std::filesystem::path(info.textures[0]).replace_extension(".json"), info.trailInfo)
                .inspectErr([&info](const std::string& err) {
                    log::error("{}: Failed to save trail info: {}", info.name, err);
                });
        }
    }
}

ColorInfo MoreIcons::vanillaColors(bool dual) {
    auto gameManager = MoreIconsAPI::get<GameManager>();
    auto sdi = dual ? Loader::get()->getLoadedMod("weebify.separate_dual_icons") : nullptr;
    return {
        .color1 = gameManager->colorForIdx(sdi ? sdi->getSavedValue("color1", 0) : gameManager->m_playerColor),
        .color2 = gameManager->colorForIdx(sdi ? sdi->getSavedValue("color2", 0) : gameManager->m_playerColor2),
        .colorGlow = gameManager->colorForIdx(sdi ? sdi->getSavedValue("colorglow", 0) : gameManager->m_playerGlowColor),
        .glow = sdi ? sdi->getSavedValue("glow", false) : gameManager->m_playerGlow
    };
}

int MoreIcons::vanillaIcon(IconType type, bool dual) {
    auto gameManager = MoreIconsAPI::get<GameManager>();
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
    if (noLayer) layer = MoreIconsAPI::get<CCDirector>()->getRunningScene()->getChildByType<GJGarageLayer>(0);
    if (!layer) return;

    auto gameManager = MoreIconsAPI::get<GameManager>();
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

    layer->selectTab(layer->m_iconType);
}

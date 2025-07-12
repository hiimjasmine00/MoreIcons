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

std::filesystem::path operator+(const std::filesystem::path& lhs, const std::filesystem::path& rhs) {
    auto ret = lhs;
    ret += rhs;
    return ret;
}

bool MoreIcons::doesExist(const std::filesystem::path& path) {
    std::error_code code;
    auto exists = std::filesystem::exists(path, code);
    if (code) log::error("{}: Failed to check existence: {}", path, code.message());
    return exists;
}

$on_mod(Loaded) {
    MoreIcons::loadSettings();
    #ifdef GEODE_IS_ANDROID
    auto assetsDir = Mod::get()->getTempDir() / "assets";
    if (exists(assetsDir)) {
        std::error_code code;
        std::filesystem::remove_all(assetsDir, code);
        if (code) log::error("{}: Failed to delete: {}", assetsDir, code.message());
    }
    #endif
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

void directoryIterator(const std::filesystem::path& path, auto&& func) {
    std::error_code code;
    std::filesystem::directory_iterator it(path, code);
    if (code) return log::error("{}: Failed to create directory iterator: {}", path, code.message());
    for (; it != std::filesystem::end(it); it.increment(code)) {
        if constexpr (std::is_void_v<decltype(func(*it))>) func(*it);
        else if (func(*it)) break;
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

    constexpr std::array folders = {
        "icon", "ship", "ball", "ufo", "wave", "robot", "spider", "swing", "jetpack"
    };

    auto& saveContainer = Mod::get()->getSaveContainer();
    if (!saveContainer.contains("migrated-folders")) saveContainer["migrated-folders"] = matjson::Value::array();

    auto& migratedFolders = saveContainer["migrated-folders"];

    for (int i = 0; i < folders.size(); i++) {
        auto& folder = folders[i];
        auto folderPath = path / folder;
        if (!MoreIcons::doesExist(folderPath)) continue;

        auto isRobot = i == 5 || i == 6;
        directoryIterator(folderPath, [i, isRobot, &migratedFolders](const std::filesystem::directory_entry& entry) {
            if (!entry.is_directory()) return;

            auto& entryPath = entry.path();
            migratedFolders.asArray().inspect([&entryPath](std::vector<matjson::Value>& vec) {
                if (std::ranges::find(vec, entryPath) == vec.end()) vec.push_back(entryPath);
            });

            std::vector<std::string> names;

            directoryIterator(entryPath, [i, isRobot, &names](const std::filesystem::directory_entry& entry) {
                if (!entry.is_regular_file()) return;

                auto& entryPath = entry.path();
                if (entryPath.extension() != ".png") return;

                auto name = string::pathToString(entryPath.stem());
                if (name.ends_with("_2_001")) {
                    if (!isRobot || name.ends_with("_01_2_001") || name.ends_with("_02_2_001") ||
                        name.ends_with("_03_2_001") || name.ends_with("_04_2_001")) names.push_back(name);
                }
                else if (i == 3 && name.ends_with("_3_001")) names.push_back(name);
                else if (name.ends_with("_extra_001")) {
                    if (!isRobot || name.ends_with("_01_extra_001")) names.push_back(name);
                }
                else if (name.ends_with("_glow_001")) {
                    if (!isRobot || name.ends_with("_01_glow_001") || name.ends_with("_02_glow_001") ||
                        name.ends_with("_03_glow_001") || name.ends_with("_04_glow_001")) names.push_back(name);
                }
                else if (name.ends_with("_001")) {
                    if (!isRobot || name.ends_with("_01_001") || name.ends_with("_02_001") ||
                        name.ends_with("_03_001") || name.ends_with("_04_001")) names.push_back(name);
                }
            });

            if (names.empty()) return;

            texpack::Packer packer;

            auto frameFailed = false;
            for (auto& name : names) {
                auto filename = name + ".png";
                if (GEODE_UNWRAP_IF_ERR(err, packer.frame(filename, entryPath / filename))) {
                    log::error("{}: Failed to load {}: {}", entryPath, filename, err);
                    frameFailed = true;
                    break;
                }
            }

            if (frameFailed) return;

            if (GEODE_UNWRAP_IF_ERR(err, packer.pack())) return log::error("{}: Failed to pack frames: {}", entryPath, err);

            auto pngPath = entryPath + ".png";
            if (MoreIcons::doesExist(pngPath)) {
                std::error_code code;
                std::filesystem::rename(pngPath, pngPath + ".bak", code);
                if (code) log::error("{}: Failed to rename existing image: {}", entryPath, code.message());
            }
            if (GEODE_UNWRAP_IF_ERR(err, packer.png(pngPath))) log::error("{}: Failed to save image: {}", entryPath, err);

            auto plistPath = entryPath + ".plist";
            if (MoreIcons::doesExist(plistPath)) {
                std::error_code code;
                std::filesystem::rename(plistPath, plistPath + ".bak", code);
                if (code) log::error("{}: Failed to rename existing plist: {}", entryPath, code.message());
            }
            if (GEODE_UNWRAP_IF_ERR(err, packer.plist(plistPath, fmt::format("icons/{}", entryPath.filename()), "    ")))
                log::error("{}: Failed to save plist: {}", entryPath, err);
        });
    }

    safeInfo("Finished folder icon migration in {}", path);
}

void MoreIcons::loadPacks() {
    packs.clear();
    packs.push_back({ "More Icons", "", dirs::getGeodeDir(), false, false });
    migrateFolderIcons(Mod::get()->getConfigDir());

    for (auto& pack : texture_loader::getAppliedPacks()) {
        auto extension = pack.path.extension();
        auto zipped = extension == ".apk" || extension == ".zip";
        if (traditionalPacks) {
            if (doesExist(pack.resourcesPath / "icons")) packs.push_back({ pack.name, pack.id, pack.resourcesPath, true, zipped });
            else directoryIterator(pack.resourcesPath, [&pack, zipped](const std::filesystem::directory_entry& entry) {
                if (!entry.is_regular_file()) return false;

                auto& entryPath = entry.path();
                if (entryPath.extension() != ".png") return false;

                auto filename = string::pathToString(entryPath.filename());
                if (filename.starts_with("streak_") && filename.ends_with("_001.png")) {
                    packs.push_back({ pack.name, pack.id, entryPath.parent_path(), true, zipped });
                    return true;
                }

                return false;
            });
        }

        auto configPath = pack.resourcesPath / "config" / GEODE_MOD_ID;
        if (doesExist(configPath)) {
            packs.push_back({ pack.name, pack.id, pack.resourcesPath, false, zipped });
            migrateFolderIcons(configPath);
        }
    }
}

std::string MoreIcons::vanillaTexturePath(const std::string& path, bool skipSuffix) {
    #ifdef GEODE_IS_MOBILE
    if (!skipSuffix && CCDirector::get()->getContentScaleFactor() >= 4.0f) {
        if (auto highGraphicsMobile = Loader::get()->getLoadedMod("weebify.high-graphics-android")) {
            auto configDir = highGraphicsMobile->getConfigDir(false) / GEODE_STR(GEODE_GD_VERSION);
            if (doesExist(configDir)) return configDir / path;
        }
        return path;
    }
    #endif
    return string::pathToString(dirs::getResourcesDir() / path);
}

Result<std::filesystem::path> MoreIcons::createTrash() {
    std::error_code code;
    auto trashPath = Mod::get()->getConfigDir() / "trash";
    auto exists = doesExist(trashPath);
    if (!exists) exists = std::filesystem::create_directories(trashPath, code);
    if (!exists) return Err(code.message());
    else {
        std::filesystem::permissions(trashPath, std::filesystem::perms::all, code);
        return Ok(trashPath);
    }
}

std::string replaceEnd(const std::string& str, size_t end, std::string_view replace) {
    return str.substr(0, str.size() - end).append(replace);
}

IconType currentType = IconType::Cube;

template <typename... Args>
void printLog(const std::string& name, int severity, fmt::format_string<Args...> message, Args&&... args) {
    auto logMessage = fmt::format(message, std::forward<Args>(args)...);
    log::logImpl(Severity::cast(severity), Mod::get(), "{}: {}", name, logMessage);
    MoreIcons::logs.push_back({ name, logMessage, currentType, severity });
    auto& currentSeverity = MoreIcons::severities[currentType];
    if (currentSeverity < severity) currentSeverity = severity;
    if (MoreIcons::severity < severity) MoreIcons::severity = severity;
}

void loadIcon(const std::filesystem::path& path, const IconPack& pack) {
    auto factor = CCDirector::get()->getContentScaleFactor();
    auto pathFilename = string::pathToString(path.filename());
    auto pathStem = string::pathToString(path.stem());
    auto pathString = string::pathToString(path);
    std::string name;
    std::string shortName;
    if (pathFilename.ends_with("-uhd.plist")) {
        shortName = replaceEnd(pathStem, 4, "");
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        if (factor < 4.0f && factor >= 2.0f) {
            if (!MoreIcons::doesExist(replaceEnd(pathString, 10, "-hd.plist")) && !MoreIcons::doesExist(replaceEnd(pathString, 9, ".plist")))
                printLog(name, Severity::Warning, "Ignoring high-quality icon on medium texture quality");
            return;
        }
        else if (factor < 2.0f) {
            if (!MoreIcons::doesExist(replaceEnd(pathString, 10, ".plist")))
                printLog(name, Severity::Warning, "Ignoring high-quality icon on low texture quality");
            return;
        }
    }
    else if (pathFilename.ends_with("-hd.plist")) {
        shortName = replaceEnd(pathStem, 3, "");
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        if (factor < 2.0f) {
            if (!MoreIcons::doesExist(replaceEnd(pathString, 9, ".plist")))
                printLog(name, Severity::Warning, "Ignoring medium-quality icon on low texture quality");
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

    auto texturePath = replaceEnd(pathString, 6, ".png");
    if (!MoreIcons::doesExist(texturePath)) return printLog(name, Severity::Error, "Texture file {}.png not found", pathStem);

    if (MoreIconsAPI::hasIcon(name, currentType)) return printLog(name, Severity::Warning, "Duplicate icon name");

    MoreIconsAPI::addIcon({
        .name = name,
        .textures = { texturePath },
        .frameNames = {},
        .sheetName = pathString,
        .packName = pack.name,
        .packID = pack.id,
        .type = currentType,
        .trailID = 0,
        .blend = false,
        .tint = false,
        .show = false,
        .fade = 0.0f,
        .stroke = 0.0f,
        .shortName = shortName,
        .vanilla = false,
        .zipped = pack.zipped
    });

    safeDebug("Finished pre-loading icon {} from {}", name, pack.name);
}

void loadVanillaIcon(const std::filesystem::path& path, const IconPack& pack) {
    auto factor = CCDirector::get()->getContentScaleFactor();
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

    auto plistPath = replaceEnd(pathString, 4, ".plist");
    if (!MoreIcons::doesExist(plistPath)) plistPath = MoreIcons::vanillaTexturePath(fmt::format("icons/{}.plist", pathStem), false);
    if (!CCFileUtils::get()->isFileExist(plistPath)) return printLog(name, Severity::Error, "Plist file not found (Last attempt: {})", plistPath);

    if (auto icon = MoreIconsAPI::getIcon(name, currentType))
        MoreIconsAPI::icons.erase(MoreIconsAPI::icons.begin() + (icon - MoreIconsAPI::icons.data()));

    MoreIconsAPI::addIcon({
        .name = name,
        .textures = { pathString },
        .frameNames = {},
        .sheetName = plistPath,
        .packName = pack.name,
        .packID = pack.id,
        .type = currentType,
        .trailID = 0,
        .blend = false,
        .tint = false,
        .show = false,
        .fade = 0.0f,
        .stroke = 0.0f,
        .shortName = shortName,
        .vanilla = true,
        .zipped = pack.zipped
    });

    safeDebug("Finished pre-loading vanilla icon {} from {}", name, pack.name);
}

void loadTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto pathStem = string::pathToString(path.stem());
    auto name = pack.id.empty() ? pathStem : fmt::format("{}:{}", pack.id, pathStem);

    safeDebug("Pre-loading trail {} from {}", name, pack.name);

    if (MoreIconsAPI::hasIcon(name, IconType::Special)) return printLog(name, Severity::Warning, "Duplicate trail name");

    auto json = file::readJson(path.parent_path() / (pathStem + ".json")).unwrapOr(matjson::makeObject({
        { "blend", false },
        { "tint", false },
        { "show", false },
        { "fade", 0.3f },
        { "stroke", 14.0f }
    }));

    MoreIconsAPI::addIcon({
        .name = name,
        .textures = { string::pathToString(path) },
        .frameNames = {},
        .sheetName = "",
        .packName = pack.name,
        .packID = pack.id,
        .type = IconType::Special,
        .trailID = 0,
        .blend = json.get("blend").andThen([](const matjson::Value& v) { return v.asBool(); }).unwrapOr(false),
        .tint = json.get("tint").andThen([](const matjson::Value& v) { return v.asBool(); }).unwrapOr(false),
        .show = json.get("show").andThen([](const matjson::Value& v) { return v.asBool(); }).unwrapOr(false),
        .fade = json.get("fade").andThen([](const matjson::Value& v) { return v.as<float>(); }).unwrapOr(0.3f),
        .stroke = json.get("stroke").andThen([](const matjson::Value& v) { return v.as<float>(); }).unwrapOr(14.0f),
        .shortName = pathStem,
        .vanilla = false,
        .zipped = pack.zipped
    });

    safeDebug("Finished pre-loading trail {} from {}", name, pack.name);
}

void loadVanillaTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto pathStem = string::pathToString(path.stem());
    auto name = fmt::format("{}:{}", pack.id, pathStem);

    safeDebug("Pre-loading vanilla trail {} from {}", name, pack.name);
    auto trailID = numFromString<int>(pathStem.substr(7, pathStem.size() - 11)).unwrapOr(0);
    if (trailID == 0) trailID = -1;

    if (auto icon = MoreIconsAPI::getIcon(name, IconType::Special))
        MoreIconsAPI::icons.erase(MoreIconsAPI::icons.begin() + (icon - MoreIconsAPI::icons.data()));

    MoreIconsAPI::addIcon({
        .name = name,
        .textures = { string::pathToString(path) },
        .frameNames = {},
        .sheetName = "",
        .packName = pack.name,
        .packID = pack.id,
        .type = IconType::Special,
        .trailID = trailID,
        .blend = false,
        .tint = false,
        .show = false,
        .fade = 0.0f,
        .stroke = 0.0f,
        .shortName = pathStem,
        .vanilla = true,
        .zipped = pack.zipped
    });

    safeDebug("Finished pre-loading vanilla trail {} from {}", name, pack.name);
}

void MoreIcons::loadIcons(IconType type) {
    currentType = type;

    constexpr std::array types = {
        std::make_tuple("", "", ""),
        std::make_tuple("player_", "icon", "icons"),
        std::make_tuple("", "", ""),
        std::make_tuple("", "", ""),
        std::make_tuple("ship_", "ship", "ships"),
        std::make_tuple("player_ball_", "ball", "balls"),
        std::make_tuple("bird_", "ufo", "UFOs"),
        std::make_tuple("dart_", "wave", "waves"),
        std::make_tuple("robot_", "robot", "robots"),
        std::make_tuple("spider_", "spider", "spiders"),
        std::make_tuple("streak_", "trail", "trails"),
        std::make_tuple("PlayerExplosion_", "death", "death effects"),
        std::make_tuple("", "", ""),
        std::make_tuple("swing_", "swing", "swings"),
        std::make_tuple("jetpack_", "jetpack", "jetpacks"),
        std::make_tuple("shipfire", "fire", "ship fires")
    };
    auto& [prefix, folder, name] = types[(int)GameManager::get()->iconTypeToUnlockType(type)];

    for (int i = 0; i < packs.size(); i++) {
        auto& pack = packs[i];

        if (!pack.vanilla) {
            auto path = pack.path / "config" / GEODE_MOD_ID / folder;
            if (!doesExist(path)) {
                if (i == 0) {
                    std::error_code code;
                    std::filesystem::create_directories(path, code);
                    if (code) log::error("{}: Failed to create directory: {}", path, code.message());
                }
                continue;
            }

            if (i == 0) {
                std::error_code code;
                std::filesystem::permissions(path, std::filesystem::perms::all, code);
                if (code) log::error("{}: Failed to change permissions: {}", path, code.message());
            }

            safeInfo("Pre-loading {} from {}", name, path);

            directoryIterator(path, [type, &pack](const std::filesystem::directory_entry& entry) {
                if (!entry.is_regular_file()) return;

                auto& entryPath = entry.path();
                if (type <= IconType::Jetpack) {
                    if (entryPath.extension() == ".plist") loadIcon(entryPath, pack);
                }
                else if (type == IconType::Special) {
                    if (entryPath.extension() == ".png") loadTrail(entryPath, pack);
                }
            });

            safeInfo("Finished pre-loading {} from {}", name, path);
        }
        else if (traditionalPacks) {
            auto path = type == IconType::Special ? pack.path : pack.path / "icons";
            if (!doesExist(path)) continue;

            safeInfo("Pre-loading {} from {}", name, path);

            directoryIterator(path, [type, &pack, prefix](const std::filesystem::directory_entry& entry) {
                if (!entry.is_regular_file()) return;

                auto& entryPath = entry.path();
                if (entryPath.extension() != ".png") return;

                auto filename = string::pathToString(entryPath.filename());
                if (!filename.starts_with(prefix)) return;

                if (type <= IconType::Jetpack) {
                    if (type != IconType::Cube || !filename.starts_with("player_ball_")) {
                        auto suffix = filename.substr(strlen(prefix));
                        if (suffix != "00.png" && suffix != "00-hd.png" && suffix != "00-uhd.png") loadVanillaIcon(entryPath, pack);
                    }
                }
                else if (type == IconType::Special) {
                    if (filename.ends_with("_001.png")) loadVanillaTrail(entryPath, pack);
                }
            });

            safeInfo("Finished pre-loading {} from {}", name, path);
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
    for (auto& info : MoreIconsAPI::icons) {
        if (info.type != IconType::Special || info.trailID != 0) continue;
        (void)file::writeToJson(replaceEnd(info.textures[0], 4, ".json"), matjson::makeObject({
            { "blend", info.blend },
            { "tint", info.tint },
            { "show", info.show },
            { "fade", info.fade },
            { "stroke", info.stroke }
        })).inspectErr([info](const std::string& err) { log::error("{}: Failed to save trail info: {}", info.name, err); });
    }
}

void MoreIcons::updateGarage(GJGarageLayer* layer) {
    auto hasLayer = layer != nullptr;
    if (!hasLayer) layer = CCScene::get()->getChildByType<GJGarageLayer>(0);
    if (!layer) return;

    auto gameManager = GameManager::get();
    auto player1 = layer->m_playerObject;
    auto iconType1 = gameManager->m_playerIconType;
    if (!hasLayer) player1->updatePlayerFrame(gameManager->activeIconForType(iconType1), iconType1);
    MoreIconsAPI::updateSimplePlayer(player1, iconType1, false);

    if (auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons")) {
        auto player2 = static_cast<SimplePlayer*>(layer->getChildByID("player2-icon"));
        auto iconType2 = (IconType)sdi->getSavedValue("lastmode", 0);
        constexpr std::array types = { "cube", "ship", "roll", "bird", "dart", "robot", "spider", "swing", "jetpack" };
        if (!hasLayer) player2->updatePlayerFrame(sdi->getSavedValue(types[(int)iconType2], 1), iconType2);
        MoreIconsAPI::updateSimplePlayer(player2, iconType2, true);
    }

    layer->selectTab(layer->m_iconType);
}

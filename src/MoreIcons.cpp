#include "MoreIcons.hpp"
#include "api/MoreIconsAPI.hpp"
#include "classes/misc/ThreadPool.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/utils/ranges.hpp>
#include <geode.texture-loader/include/TextureLoader.hpp>
#include <texpack.hpp>

using namespace geode::prelude;

bool doesExist(const std::filesystem::path& path) {
    std::error_code code;
    auto exists = std::filesystem::exists(path, code);
    if (code) log::error("{}: Failed to check existence: {}", path, code.message());
    return exists;
}

$on_mod(Loaded) {
    MoreIcons::loadSettings();
    #ifdef GEODE_IS_ANDROID
    auto assetsDir = Mod::get()->getTempDir() / "assets";
    if (doesExist(assetsDir)) {
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

#define DIRECTORY_ITERATOR(path) \
    std::error_code code; \
    std::filesystem::directory_iterator it(path, code); \
    if (code) { \
        log::error("{}: Failed to create directory iterator: {}", path, code.message()); \
        continue; \
    } \
    for (; it != std::filesystem::end(it); it.increment(code))

#define DIRECTORY_ITERATOR_END(path) if (code) log::error("{}: Failed to iterate over directory: {}", path, code.message());

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
    if (!saveContainer.contains("migrated-folders")) saveContainer["migrated-folders"] = std::vector<matjson::Value>();

    auto& migratedFolders = saveContainer["migrated-folders"];

    for (int i = 0; i < folders.size(); i++) {
        auto& folder = folders[i];
        auto folderPath = path / folder;
        if (!doesExist(folderPath)) continue;

        auto isRobot = i == 5 || i == 6;
        DIRECTORY_ITERATOR(folderPath) {
            auto& entry = *it;
            if (!entry.is_directory()) continue;

            auto& entryPath = entry.path();
            if (migratedFolders.isArray() && ranges::contains(migratedFolders.asArray().unwrap(), entryPath)) continue;

            migratedFolders.push(entryPath);

            std::vector<std::string> names;

            DIRECTORY_ITERATOR(entryPath) {
                auto& entry2 = *it;
                if (!entry2.is_regular_file()) continue;

                auto& entry2Path = entry2.path();
                if (entry2Path.extension() != ".png") continue;

                auto name = string::pathToString(entry2Path.stem());
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
            }
            DIRECTORY_ITERATOR_END(entryPath)

            if (names.empty()) continue;

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

            if (frameFailed) continue;

            if (GEODE_UNWRAP_IF_ERR(err, packer.pack())) {
                log::error("{}: Failed to pack frames: {}", entryPath, err);
                continue;
            }

            auto pngPath = std::filesystem::path(entryPath).concat(".png");
            if (doesExist(pngPath)) {
                std::error_code code;
                std::filesystem::rename(pngPath, std::filesystem::path(pngPath).concat(".bak"), code);
                if (code) log::error("{}: Failed to rename existing image: {}", entryPath, code.message());
            }
            if (GEODE_UNWRAP_IF_ERR(err, packer.png(pngPath))) log::error("{}: Failed to save image: {}", entryPath, err);

            auto plistPath = std::filesystem::path(entryPath).concat(".plist");
            if (doesExist(plistPath)) {
                std::error_code code;
                std::filesystem::rename(plistPath, std::filesystem::path(plistPath).concat(".bak"), code);
                if (code) log::error("{}: Failed to rename existing plist: {}", entryPath, code.message());
            }
            if (GEODE_UNWRAP_IF_ERR(err, packer.plist(plistPath, fmt::format("icons/{}", entryPath.filename()), "    ")))
                log::error("{}: Failed to save plist: {}", entryPath, err);
        }
        DIRECTORY_ITERATOR_END(folderPath)
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
            else {
                DIRECTORY_ITERATOR(pack.resourcesPath) {
                    auto& entry = *it;
                    if (!entry.is_regular_file()) continue;

                    auto& entryPath = entry.path();
                    if (entryPath.extension() != ".png") continue;

                    auto filename = string::pathToString(entryPath.filename());
                    if (filename.starts_with("streak_") && filename.ends_with("_001.png")) {
                        packs.push_back({ pack.name, pack.id, entryPath.parent_path(), true, zipped });
                        break;
                    }
                }
                DIRECTORY_ITERATOR_END(pack.resourcesPath)
            }
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
    if (CCDirector::get()->getContentScaleFactor() >= 4.0f && !skipSuffix) {
        if (auto highGraphicsMobile = Loader::get()->getLoadedMod("weebify.high-graphics-android")) {
            auto configDir = highGraphicsMobile->getConfigDir(false) / GEODE_STR(GEODE_GD_VERSION);
            if (doesExist(configDir)) return configDir / path;
        }
        return path;
    }
    #endif
    return string::pathToString(dirs::getResourcesDir() / path);
}

std::filesystem::path MoreIcons::createTrash() {
    std::error_code code;
    auto trashPath = Mod::get()->getConfigDir() / "trash";
    auto exists = doesExist(trashPath);
    if (!exists) exists = std::filesystem::create_directory(trashPath, code);
    if (!exists) {
        Notification::create(fmt::format("Failed to create trash directory: {}.", code.message()), NotificationIcon::Error)->show();
        return "";
    }
    else {
        std::filesystem::permissions(trashPath, std::filesystem::perms::all, code);
        return trashPath;
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
            if (!doesExist(replaceEnd(pathString, 10, "-hd.plist")) && !doesExist(replaceEnd(pathString, 9, ".plist")))
                printLog(name, Severity::Warning, "Ignoring high-quality icon on medium texture quality");
            return;
        }
        else if (factor < 2.0f) {
            if (!doesExist(replaceEnd(pathString, 10, ".plist")))
                printLog(name, Severity::Warning, "Ignoring high-quality icon on low texture quality");
            return;
        }
    }
    else if (pathFilename.ends_with("-hd.plist")) {
        shortName = replaceEnd(pathStem, 3, "");
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        if (factor < 2.0f) {
            if (!doesExist(replaceEnd(pathString, 9, ".plist")))
                printLog(name, Severity::Warning, "Ignoring medium-quality icon on low texture quality");
            return;
        }

        if (doesExist(replaceEnd(pathString, 9, "-uhd.plist")) && factor >= 4.0f) return;
    }
    else {
        shortName = pathStem;
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        if (doesExist(replaceEnd(pathString, 6, "-uhd.plist")) && factor >= 4.0f) return;
        else if (doesExist(replaceEnd(pathString, 6, "-hd.plist")) && factor >= 2.0f) return;
    }

    safeDebug("Pre-loading icon {} from {}", name, pack.name);

    auto texturePath = replaceEnd(pathString, 6, ".png");
    if (!doesExist(texturePath)) return printLog(name, Severity::Error, "Texture file {}.png not found", pathStem);

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
        if (doesExist(replaceEnd(pathString, 7, "-uhd.png")) && factor >= 4.0f) return;
        shortName = replaceEnd(pathStem, 3, "");
        name = fmt::format("{}:{}", pack.id, shortName);
    }
    else {
        if (doesExist(replaceEnd(pathString, 4, "-uhd.png")) && factor >= 4.0f) return;
        else if (doesExist(replaceEnd(pathString, 4, "-hd.png")) && factor >= 2.0f) return;
        shortName = pathStem;
        name = fmt::format("{}:{}", pack.id, shortName);
    }

    safeDebug("Pre-loading vanilla icon {} from {}", name, pack.name);

    auto plistPath = replaceEnd(pathString, 4, ".plist");
    if (!doesExist(plistPath)) plistPath = MoreIcons::vanillaTexturePath(fmt::format("icons/{}.plist", pathStem), false);
    if (!CCFileUtils::get()->isFileExist(plistPath)) return printLog(name, Severity::Error, "Plist file not found (Last attempt: {})", plistPath);

    ranges::remove(MoreIconsAPI::icons, [&name](const IconInfo& icon) { return icon.name == name && icon.type == currentType; });

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

    auto json = file::readJson(std::filesystem::path(path).replace_extension(".json")).unwrapOr(matjson::makeObject({
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
        .blend = json.contains("blend") ? json["blend"].asBool().unwrapOr(false) : false,
        .tint = json.contains("tint") ? json["tint"].asBool().unwrapOr(false) : false,
        .show = json.contains("show") ? json["show"].asBool().unwrapOr(false) : false,
        .fade = json.contains("fade") ? (float)json["fade"].asDouble().unwrapOr(0.3) : 0.3f,
        .stroke = json.contains("stroke") ? (float)json["stroke"].asDouble().unwrapOr(14.0) : 14.0f,
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
    auto trailID = numFromString<int>(pathStem.substr(pathStem.find('_') + 1, pathStem.rfind('_') - pathStem.find('_') - 1)).unwrapOr(0);
    if (trailID == 0) trailID = -1;

    ranges::remove(MoreIconsAPI::icons, [&name](const IconInfo& icon) { return icon.name == name && icon.type == IconType::Special; });

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

            DIRECTORY_ITERATOR(path) {
                auto& entry = *it;
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                if (type <= IconType::Jetpack) {
                    if (entryPath.extension() == ".plist") loadIcon(entryPath, pack);
                }
                else if (type == IconType::Special) {
                    if (entryPath.extension() == ".png") loadTrail(entryPath, pack);
                }
            }
            DIRECTORY_ITERATOR_END(path)

            safeInfo("Finished pre-loading {} from {}", name, path);
        }
        else if (traditionalPacks) {
            auto path = type == IconType::Special ? pack.path : pack.path / "icons";
            if (!doesExist(path)) continue;

            safeInfo("Pre-loading {} from {}", name, path);

            DIRECTORY_ITERATOR(path) {
                auto& entry = *it;
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                if (entryPath.extension() != ".png") continue;

                auto filename = string::pathToString(entryPath.filename());
                if (!filename.starts_with(prefix)) continue;

                if (type <= IconType::Jetpack) {
                    if (type != IconType::Cube || !filename.starts_with("player_ball_")) {
                        auto suffix = filename.substr(strlen(prefix));
                        if (suffix != "00.png" && suffix != "00-hd.png" && suffix != "00-uhd.png") loadVanillaIcon(entryPath, pack);
                    }
                }
                else if (type == IconType::Special) {
                    if (filename.ends_with("_001.png")) loadVanillaTrail(entryPath, pack);
                }
            }
            DIRECTORY_ITERATOR_END(path)

            safeInfo("Finished pre-loading {} from {}", name, path);
        }
    }

    MoreIconsAPI::loadIcons(type, infoLogs);

    if (type == IconType::Special) {
        packs.clear();
    }
}

void MoreIcons::saveTrails() {
    for (auto& info : MoreIconsAPI::icons) {
        if (info.type != IconType::Special || info.trailID != 0) continue;
        (void)file::writeToJson(std::filesystem::path(info.textures[0]).replace_extension(".json"), matjson::makeObject({
            { "blend", info.blend },
            { "tint", info.tint },
            { "show", info.show },
            { "fade", info.fade },
            { "stroke", info.stroke }
        })).inspectErr([info](const std::string& err) { log::error("{}: Failed to save trail info: {}", info.name, err); });
    }
}

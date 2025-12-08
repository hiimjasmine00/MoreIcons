#define FMT_CPP_LIB_FILESYSTEM 0
#include "MoreIcons.hpp"
#include "classes/misc/ThreadPool.hpp"
#include "utils/Get.hpp"
#include "utils/Load.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJGarageLayer.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/ui/Notification.hpp>
#include <geode.texture-loader/include/TextureLoader.hpp>
#include <jasmine/convert.hpp>
#include <jasmine/setting.hpp>
#include <MoreIconsV2.hpp>
#include <std23/function_ref.h>
#include <texpack.hpp>

using namespace geode::prelude;

std::string std::filesystem::format_as(const std::filesystem::path& p) {
    return utils::string::pathToString(p.native());
}

$on_mod(Loaded) {
    MoreIcons::loadSettings();
}

std::map<IconType, std::vector<IconInfo>> MoreIcons::icons = {
    { IconType::Cube, {} },
    { IconType::Ship, {} },
    { IconType::Ball, {} },
    { IconType::Ufo, {} },
    { IconType::Wave, {} },
    { IconType::Robot, {} },
    { IconType::Spider, {} },
    { IconType::Swing, {} },
    { IconType::Jetpack, {} },
    { IconType::Special, {} }
};
std::map<int, std::map<IconType, std::string>> MoreIcons::requestedIcons;
std::map<std::pair<std::string, IconType>, int> MoreIcons::loadedIcons;
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
bool MoreIcons::preloadIcons = false;

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
    preloadIcons = jasmine::setting::getValue<bool>("preload-icons");
}

bool MoreIcons::doesExist(const std::filesystem::path& path) {
    std::error_code code;
    auto exists = std::filesystem::exists(path, code);
    if (code) log::error("{}: Failed to check existence: {}", path, code.message());
    return exists;
}

Result<std::filesystem::path> MoreIcons::renameFile(
    const std::filesystem::path& from, const std::filesystem::path& to, bool overwrite, bool copy
) {
    std::error_code code;
    if (!doesExist(from)) return Ok(std::filesystem::path());
    if (overwrite && doesExist(to)) {
        if (!std::filesystem::remove(to, code)) return Err("Failed to remove {}: {}", to.filename(), code.message());
    }
    auto dest = to;
    if (copy) {
        for (int i = 1; doesExist(dest); i++) {
            dest.replace_filename(dest.stem().native() + MI_PATH(" (") +
                GEODE_WINDOWS(string::utf8ToWide)(fmt::to_string(i)) + MI_PATH(")") + dest.extension().native());
        }
    }
    std::filesystem::rename(from, dest, code);
    if (code) return Err("Failed to rename {}: {}", from.filename(), code.message());
    return Ok(dest.filename());
}

void MoreIcons::iterate(
    const std::filesystem::path& path, std::filesystem::file_type type, std23::function_ref<void(const std::filesystem::path&)> func
) {
    std::error_code code;
    std::filesystem::directory_iterator it(path, code);
    if (code) return log::error("{}: Failed to create directory iterator: {}", path, code.message());
    for (; it != std::filesystem::end(it); it.increment(code)) {
        std::error_code code;
        if (it->status(code).type() != type) continue;
        func(it->path());
    }
    if (code) return log::error("{}: Failed to iterate over directory: {}", path, code.message());
}

void boolIterate(const std::filesystem::path& path, std::filesystem::file_type type, std23::function_ref<bool(const std::filesystem::path&)> func) {
    std::error_code code;
    std::filesystem::directory_iterator it(path, code);
    if (code) return log::error("{}: Failed to create directory iterator: {}", path, code.message());
    for (; it != std::filesystem::end(it); it.increment(code)) {
        std::error_code code;
        if (it->status(code).type() != type) continue;
        if (func(it->path())) break;
    }
    if (code) return log::error("{}: Failed to iterate over directory: {}", path, code.message());
}

std::filesystem::path::string_type MoreIcons::getPathString(std::filesystem::path path) {
    return std::move(const_cast<std::filesystem::path::string_type&>(path.native()));
}

void migrateFolderIcons(const std::filesystem::path& path) {
    log::info("Beginning folder icon migration in {}", path);

    auto& saveContainer = Mod::get()->getSaveContainer();
    if (!saveContainer.contains("migrated-folders")) saveContainer.set("migrated-folders", matjson::Value::array());

    auto& migratedFolders = saveContainer["migrated-folders"];

    for (int i = 0; i < 9; i++) {
        auto folderPath = path / MoreIcons::folders[i];
        if (!MoreIcons::doesExist(folderPath)) continue;

        MoreIcons::iterate(folderPath, std::filesystem::file_type::directory, [
            i, &folderPath, &migratedFolders
        ](const std::filesystem::path& path) {
            if (std::ranges::contains(migratedFolders, path)) return;
            else migratedFolders.push(path);

            std::vector<std::filesystem::path> names;

            MoreIcons::iterate(path, std::filesystem::file_type::regular, [i, &names](const std::filesystem::path& path) {
                if (!path.native().ends_with(MI_PATH("_001.png"))) return;

                auto end = MoreIcons::getPathString(path.filename());
                end = std::move(end).substr(0, end.size() - 4);

                auto isRobot = i == 5 || i == 6;
                if (end.ends_with(MI_PATH("_2"))) {
                    if (isRobot) {
                        end = std::move(end).substr(0, end.size() - 2);
                        if (!end.ends_with(MI_PATH("_01")) && !end.ends_with(MI_PATH("_02")) &&
                            !end.ends_with(MI_PATH("_03")) && !end.ends_with(MI_PATH("_04"))) return;
                    }
                }
                else if (end.ends_with(MI_PATH("_extra"))) {
                    if (isRobot) {
                        end = std::move(end).substr(0, end.size() - 6);
                        if (!end.ends_with(MI_PATH("_01"))) return;
                    }
                }
                else if (end.ends_with(MI_PATH("_glow"))) {
                    if (isRobot) {
                        end = std::move(end).substr(0, end.size() - 5);
                        if (!end.ends_with(MI_PATH("_01")) && !end.ends_with(MI_PATH("_02")) &&
                            !end.ends_with(MI_PATH("_03")) && !end.ends_with(MI_PATH("_04"))) return;
                    }
                }
                else if (isRobot && !end.ends_with(MI_PATH("_01")) && !end.ends_with(MI_PATH("_02")) &&
                    !end.ends_with(MI_PATH("_03")) && !end.ends_with(MI_PATH("_04"))) return;

                names.push_back(path.filename());
            });

            if (names.empty()) return;

            texpack::Packer packer;

            for (auto& filename : names) {
                if (auto res = packer.frame(string::pathToString(filename), path / filename); res.isErr()) {
                    return log::error("{}: Failed to load frame {}: {}", path, filename, res.unwrapErr());
                }
            }

            if (auto res = packer.pack(); res.isErr()) return log::error("{}: Failed to pack frames: {}", path, res.unwrapErr());

            auto stem = MoreIcons::getPathString(folderPath / path.filename());

            std::filesystem::path pngPath = stem + MI_PATH(".png");
            if (auto res = MoreIcons::renameFile(pngPath, pngPath); res.isErr()) {
                log::error("{}: Failed to rename existing image: {}", path, res.unwrapErr());
            }
            if (auto res = packer.png(pngPath); res.isErr()) {
                log::error("{}: Failed to save image: {}", path, res.unwrapErr());
            }

            std::filesystem::path plistPath = stem + MI_PATH(".plist");
            if (auto res = MoreIcons::renameFile(plistPath, plistPath); res.isErr()) {
                log::error("{}: Failed to rename existing plist: {}", path, res.unwrapErr());
            }
            if (auto res = packer.plist(plistPath, string::pathToString(MI_PATH("icons") / pngPath.filename()), "    "); res.isErr()) {
                log::error("{}: Failed to save plist: {}", path, res.unwrapErr());
            }
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
    packs.emplace_back("More Icons", std::string(), dirs::getGeodeDir(), false, false);
    migrateFolderIcons(Mod::get()->getConfigDir());

    for (auto& pack : texture_loader::getAppliedPacks()) {
        auto& str = pack.path.native();
        auto zipped = str.ends_with(MI_PATH(".apk")) || str.ends_with(MI_PATH(".zip"));
        if (traditionalPacks) {
            if (doesExist(pack.resourcesPath / MI_PATH("icons"))) {
                packs.emplace_back(std::move(pack.name), std::move(pack.id), std::move(pack.resourcesPath), true, zipped);
            }
            else boolIterate(pack.resourcesPath, std::filesystem::file_type::regular, [&pack, zipped](const std::filesystem::path& path) {
                if (!path.native().ends_with(MI_PATH(".png"))) return false;

                auto filename = getPathString(path.filename());
                if (!filename.starts_with(MI_PATH("streak_")) || !filename.ends_with(MI_PATH("_001.png"))) return false;

                packs.emplace_back(std::move(pack.name), std::move(pack.id), path.parent_path(), true, zipped);
                return true;
            });
        }

        auto configPath = pack.resourcesPath / MI_PATH("config") / MI_PATH_ID;
        if (doesExist(configPath)) {
            packs.emplace_back(std::move(pack.name), std::move(pack.id), std::move(pack.resourcesPath), false, zipped);
            migrateFolderIcons(configPath);
        }
    }
}

std::filesystem::path MoreIcons::strPath(const std::string& path) {
    return std::filesystem::path(GEODE_WINDOWS(string::utf8ToWide)(path));
}

std::filesystem::path vanillaTexturePath(const std::filesystem::path& path, bool skipSuffix) {
    #ifdef GEODE_IS_MOBILE
    if (!skipSuffix && Get::Director()->getContentScaleFactor() >= 4.0f) {
        if (auto highGraphicsMobile = Loader::get()->getLoadedMod("weebify.high-graphics-android")) {
            auto configDir = highGraphicsMobile->getConfigDir(false) / GEODE_GD_VERSION_STRING;
            if (MoreIcons::doesExist(configDir)) return configDir / path;
        }
        return path;
    }
    #endif
    return dirs::getResourcesDir() / path;
}

std::string MoreIcons::vanillaTexturePath(const std::string& path, bool skipSuffix) {
    return string::pathToString(::vanillaTexturePath(strPath(path), skipSuffix));
}

Result<std::filesystem::path> MoreIcons::createTrash() {
    auto trashPath = Mod::get()->getConfigDir() / MI_PATH("trash");
    GEODE_UNWRAP(file::createDirectoryAll(trashPath));
    std::error_code code;
    std::filesystem::permissions(trashPath, std::filesystem::perms::all, code);
    return Ok(trashPath);
}

IconType currentType = IconType::Cube;

template <typename... Args>
void printLog(const std::string& name, int severity, fmt::format_string<Args...> message, Args&&... args) {
    std::string logMessage = fmt::format(message, std::forward<Args>(args)...);
    log::logImpl(Severity::cast(severity), Mod::get(), "{}: {}", name, logMessage);
    MoreIcons::logs.emplace(std::ranges::find_if(MoreIcons::logs, [&name, severity](const LogData& log) {
        return log.severity == severity ? log.name > name : log.severity < severity;
    }), name, std::move(logMessage), currentType, severity);
    auto& currentSeverity = MoreIcons::severities[currentType];
    if (currentSeverity < severity) currentSeverity = severity;
    if (MoreIcons::severity < severity) MoreIcons::severity = severity;
}

void loadIcon(const std::filesystem::path& path, const IconPack& pack) {
    auto factor = Get::Director()->getContentScaleFactor();
    auto stem = MoreIcons::getPathString(path.stem());
    std::string name;
    std::string shortName;
    if (stem.ends_with(MI_PATH("-uhd"))) {
        auto wideName = stem.substr(0, stem.size() - 4);
        #ifdef GEODE_IS_WINDOWS
        shortName = string::wideToUtf8(wideName);
        #else
        shortName = wideName;
        #endif
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        auto start = MoreIcons::getPathString(path.parent_path() / wideName);
        if (factor < 4.0f && factor >= 2.0f) {
            if (!MoreIcons::doesExist(start + MI_PATH("-hd.plist")) && !MoreIcons::doesExist(start + MI_PATH(".plist"))) {
                printLog(name, Severity::Warning, "Ignoring high-quality icon on medium texture quality");
            }
            return;
        }
        else if (factor < 2.0f) {
            if (!MoreIcons::doesExist(start + MI_PATH(".plist"))) {
                printLog(name, Severity::Warning, "Ignoring high-quality icon on low texture quality");
            }
            return;
        }
    }
    else if (stem.ends_with(MI_PATH("-hd"))) {
        auto wideName = stem.substr(0, stem.size() - 3);
        #ifdef GEODE_IS_WINDOWS
        shortName = string::wideToUtf8(wideName);
        #else
        shortName = wideName;
        #endif
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        auto start = MoreIcons::getPathString(path.parent_path() / wideName);
        if (factor < 2.0f) {
            if (!MoreIcons::doesExist(start + MI_PATH(".plist"))) {
                printLog(name, Severity::Warning, "Ignoring medium-quality icon on low texture quality");
            }
            return;
        }

        if (factor >= 4.0f && MoreIcons::doesExist(start + MI_PATH("-uhd.plist"))) return;
    }
    else {
        shortName = GEODE_WINDOWS(string::wideToUtf8)(stem);
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        auto start = MoreIcons::getPathString(path.parent_path() / stem);
        if (factor >= 4.0f && MoreIcons::doesExist(start + MI_PATH("-uhd.plist"))) return;
        else if (factor >= 2.0f && MoreIcons::doesExist(start + MI_PATH("-hd.plist"))) return;
    }

    log::debug("Pre-loading icon {} from {}", name, pack.name);

    auto pathString = string::pathToString(path);
    if (pathString.empty() && !path.empty()) return printLog(name, Severity::Error, "More Icons only supports UTF-8 paths");

    auto texturePath = std::filesystem::path(path).replace_extension(MI_PATH(".png"));
    if (!MoreIcons::doesExist(texturePath)) {
        return printLog(name, Severity::Error, "Texture file {} not found", texturePath.filename());
    }

    more_icons::addIcon(name, shortName, currentType, string::pathToString(texturePath),
        pathString, pack.id, pack.name, 0, {}, false, pack.zipped);

    log::debug("Finished pre-loading icon {} from {}", name, pack.name);
}

void loadVanillaIcon(const std::filesystem::path& path, const IconPack& pack) {
    auto factor = Get::Director()->getContentScaleFactor();
    auto stem = MoreIcons::getPathString(path.stem());
    std::string name;
    std::string shortName;
    if (stem.ends_with(MI_PATH("-uhd"))) {
        if (factor < 4.0f) return;
        shortName = GEODE_WINDOWS(string::wideToUtf8)(stem.substr(0, stem.size() - 4));
        name = fmt::format("{}:{}", pack.id, shortName);
    }
    else if (stem.ends_with(MI_PATH("-hd"))) {
        if (factor < 2.0f) return;
        auto wideName = stem.substr(0, stem.size() - 3);
        auto start = MoreIcons::getPathString(path.parent_path() / wideName);
        if (factor >= 4.0f && MoreIcons::doesExist(start + MI_PATH("-uhd.png"))) return;
        shortName = GEODE_WINDOWS(string::wideToUtf8)(wideName);
        name = fmt::format("{}:{}", pack.id, shortName);
    }
    else {
        auto start = MoreIcons::getPathString(path.parent_path() / stem);
        if (factor >= 4.0f && MoreIcons::doesExist(start + MI_PATH("-uhd.png"))) return;
        else if (factor >= 2.0f && MoreIcons::doesExist(start + MI_PATH("-hd.png"))) return;
        shortName = GEODE_WINDOWS(string::wideToUtf8)(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
    }

    log::debug("Pre-loading vanilla icon {} from {}", name, pack.name);

    auto pathString = string::pathToString(path);
    if (pathString.empty() && !path.empty()) return printLog(name, Severity::Error, "More Icons only supports UTF-8 paths");

    auto plistPath = std::filesystem::path(path).replace_extension(MI_PATH(".plist"));
    if (!MoreIcons::doesExist(plistPath)) {
        plistPath = vanillaTexturePath(std::filesystem::path::string_type(MI_PATH("icons"))
            + std::filesystem::path::preferred_separator + stem + MI_PATH(".plist"), false);
    }
    auto plistString = string::pathToString(plistPath);
    if (!Get::FileUtils()->isFileExist(plistString)) {
        return printLog(name, Severity::Error, "Plist file not found (Last attempt: {})", plistString);
    }

    more_icons::addIcon(name, shortName, currentType, pathString, plistString, pack.id, pack.name, 0, {}, true, pack.zipped);

    log::debug("Finished pre-loading vanilla icon {} from {}", name, pack.name);
}

void loadTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto pathStem = string::pathToString(path.stem());
    auto pathString = string::pathToString(path);
    auto name = pack.id.empty() ? pathStem : fmt::format("{}:{}", pack.id, pathStem);

    log::debug("Pre-loading trail {} from {}", name, pack.name);

    if (pathString.empty() && !path.empty()) return printLog(name, Severity::Error, "More Icons only supports UTF-8 paths");

    more_icons::addIcon(name, pathStem, IconType::Special, pathString, {}, pack.id, pack.name, 0,
        file::readFromJson<TrailInfo>(std::filesystem::path(path).replace_extension(MI_PATH(".json"))).unwrapOrDefault(), false, pack.zipped);

    log::debug("Finished pre-loading trail {} from {}", name, pack.name);
}

void loadVanillaTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto pathStem = string::pathToString(path.stem());
    auto pathString = string::pathToString(path);
    auto name = fmt::format("{}:{}", pack.id, pathStem);

    log::debug("Pre-loading vanilla trail {} from {}", name, pack.name);

    if (pathString.empty() && !path.empty()) return printLog(name, Severity::Error, "More Icons only supports UTF-8 paths");

    auto trailID = jasmine::convert::getInt<int>(std::string_view(pathStem).substr(7, pathStem.size() - 11)).value_or(0);
    if (trailID == 0) trailID = -1;

    more_icons::addIcon(name, pathStem, IconType::Special, pathString, {}, pack.id, pack.name, trailID,
        MoreIcons::getTrailInfo(trailID), true, pack.zipped);

    log::debug("Finished pre-loading vanilla trail {} from {}", name, pack.name);
}

#ifdef GEODE_IS_WINDOWS
static constexpr std::array wprefixes = {
    L"player_", L"ship_", L"player_ball_", L"bird_", L"dart_", L"robot_", L"spider_",
    L"swing_", L"jetpack_", L"PlayerExplosion_", L"streak_", L"", L"shipfire"
};
#else
static constexpr std::array wprefixes = MoreIcons::prefixes;
#endif

void MoreIcons::loadIcons(IconType type) {
    currentType = type;

    auto miType = convertType(type);
    auto prefix = wprefixes[miType];
    auto folder = folders[miType];
    auto name = lowercase[miType];

    for (auto it = packs.begin(); it != packs.end(); it++) {
        auto& pack = *it;

        if (!pack.vanilla) {
            auto path = pack.path / MI_PATH("config") / MI_PATH_ID / folder;
            if (!doesExist(path)) {
                if (it == packs.begin()) {
                    if (auto res = file::createDirectoryAll(path); res.isErr()) log::error("{}: {}", path, res.unwrapErr());
                }
                continue;
            }

            if (it == packs.begin()) {
                std::error_code code;
                std::filesystem::permissions(path, std::filesystem::perms::all, code);
                if (code) log::error("{}: Failed to change permissions: {}", path, code.message());
            }

            log::info("Pre-loading {}s from {}", name, path);

            iterate(path, std::filesystem::file_type::regular, [type, &pack](const std::filesystem::path& path) {
                auto& str = path.native();
                if (type <= IconType::Jetpack) {
                    if (str.ends_with(MI_PATH(".plist"))) loadIcon(path, pack);
                }
                else if (type == IconType::Special) {
                    if (str.ends_with(MI_PATH(".png"))) loadTrail(path, pack);
                }
            });

            log::info("Finished pre-loading {}s from {}", name, path);
        }
        else if (traditionalPacks) {
            auto path = type == IconType::Special ? pack.path : pack.path / MI_PATH("icons");
            if (!doesExist(path)) continue;

            log::info("Pre-loading {}s from {}", name, path);

            iterate(path, std::filesystem::file_type::regular, [type, &pack, prefix](const std::filesystem::path& path) {
                if (!path.native().ends_with(MI_PATH(".png"))) return false;

                auto filename = getPathString(path.filename());
                if (!filename.starts_with(prefix)) return false;

                if (type <= IconType::Jetpack) {
                    if (type != IconType::Cube || !filename.starts_with(MI_PATH("player_ball_"))) {
                        if (
                            !filename.ends_with(MI_PATH("00.png")) &&
                            !filename.ends_with(MI_PATH("00-hd.png")) &&
                            !filename.ends_with(MI_PATH("00-uhd.png"))
                        ) {
                            loadVanillaIcon(path, pack);
                        }
                    }
                }
                else if (type == IconType::Special) {
                    if (filename.ends_with(MI_PATH("_001.png"))) loadVanillaTrail(path, pack);
                }
                return false;
            });

            log::info("Finished pre-loading {}s from {}", name, path);
        }
    }

    if (type == IconType::Special) packs.clear();

    if (!preloadIcons) return;

    auto& iconsVec = icons[type];
    auto end = iconsVec.data() + iconsVec.size();
    auto size = iconsVec.size();
    log::info("Pre-loading {} {} textures", size, name);

    std::vector<std::pair<ImageResult, IconInfo*>> images;
    images.reserve(size);
    std::mutex imageMutex;

    auto& threadPool = ThreadPool::get();
    for (auto info = iconsVec.data(); info != end; info++) {
        threadPool.pushTask([info, &images, &imageMutex] {
            if (auto res = Load::createFrames(strPath(info->textures[0]), strPath(info->sheetName), info->name, info->type)) {
                std::unique_lock lock(imageMutex);

                images.emplace_back(std::move(res).unwrap(), info);
            }
            else log::error("{}: {}", info->name, res.unwrapErr());
        });
    }
    threadPool.wait();

    std::unique_lock lock(imageMutex);

    while (!images.empty()) {
        auto& [image, info] = images.front();
        Load::addFrames(image, info->frameNames);
        images.erase(images.begin());
    }

    log::info("Finished pre-loading {} {} textures", size, name);
}

TrailInfo MoreIcons::getTrailInfo(int trailID) {
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
    return trailInfo;
}

bool MoreIcons::dualSelected() {
    auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
    return sdi && sdi->getSavedValue("2pselected", false);
}

ccColor3B MoreIcons::vanillaColor1(bool dual) {
    auto gameManager = Get::GameManager();
    auto sdi = dual ? Loader::get()->getLoadedMod("weebify.separate_dual_icons") : nullptr;
    return gameManager->colorForIdx(sdi ? sdi->getSavedValue("color1", 0) : gameManager->m_playerColor);
}

ccColor3B MoreIcons::vanillaColor2(bool dual) {
    auto gameManager = Get::GameManager();
    auto sdi = dual ? Loader::get()->getLoadedMod("weebify.separate_dual_icons") : nullptr;
    return gameManager->colorForIdx(sdi ? sdi->getSavedValue("color2", 0) : gameManager->m_playerColor2);
}

ccColor3B MoreIcons::vanillaColorGlow(bool dual) {
    auto gameManager = Get::GameManager();
    auto sdi = dual ? Loader::get()->getLoadedMod("weebify.separate_dual_icons") : nullptr;
    return gameManager->colorForIdx(sdi ? sdi->getSavedValue("colorglow", 0) : gameManager->m_playerGlowColor);
}

bool MoreIcons::vanillaGlow(bool dual) {
    auto gameManager = Get::GameManager();
    auto sdi = dual ? Loader::get()->getLoadedMod("weebify.separate_dual_icons") : nullptr;
    return sdi ? sdi->getSavedValue("glow", false) : gameManager->m_playerGlow;
}

int MoreIcons::vanillaIcon(IconType type, bool dual) {
    auto gameManager = Get::GameManager();
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
    if (noLayer) layer = Get::Director()->getRunningScene()->getChildByType<GJGarageLayer>(0);
    if (!layer) return;

    auto gameManager = Get::GameManager();
    auto player1 = layer->m_playerObject;
    auto iconType1 = gameManager->m_playerIconType;
    if (noLayer) player1->updatePlayerFrame(vanillaIcon(iconType1, false), iconType1);
    more_icons::updateSimplePlayer(player1, iconType1, false);

    if (auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons")) {
        auto player2 = static_cast<SimplePlayer*>(layer->getChildByID("player2-icon"));
        auto iconType2 = (IconType)sdi->getSavedValue("lastmode", 0);
        if (noLayer) player2->updatePlayerFrame(vanillaIcon(iconType2, true), iconType2);
        more_icons::updateSimplePlayer(player2, iconType2, true);
    }

    if (noLayer) layer->selectTab(layer->m_iconType);
}

CCTexture2D* MoreIcons::createAndAddFrames(IconInfo* info) {
    if (auto res = Load::createFrames(strPath(info->textures[0]), strPath(info->sheetName), info->name, info->type)) {
        return Load::addFrames(res.unwrap(), info->frameNames);
    }
    else if (res.isErr()) {
        log::error("{}: {}", info->name, res.unwrapErr());
    }
    return nullptr;
}

CCSprite* MoreIcons::customTrail(const char* png) {
    auto square = CCSprite::createWithSpriteFrameName("playerSquare_001.png");
    square->setColor({ 150, 150, 150 });

    auto streak = CCSprite::create(png);
    limitNodeHeight(streak, 27.0f, 999.0f, 0.001f);
    streak->setRotation(-90.0f);
    streak->setPosition(square->getContentSize() / 2.0f);
    square->addChild(streak);

    return square;
}

std::filesystem::path MoreIcons::getEditorDir(IconType type) {
    return Mod::get()->getConfigDir() / MI_PATH("editor") / folders[convertType(type)];
}

CCSpriteFrame* MoreIcons::getFrame(const char* name) {
    auto spriteFrame = static_cast<CCSpriteFrame*>(Get::SpriteFrameCache()->spriteFrameByName(name));
    if (!spriteFrame || spriteFrame->getTag() == 105871529) spriteFrame = nullptr;
    return spriteFrame;
}

std::filesystem::path MoreIcons::getIconDir(IconType type) {
    return Mod::get()->getConfigDir() / folders[convertType(type)];
}

std::filesystem::path MoreIcons::getIconStem(const std::string& name, IconType type) {
    return getIconDir(type) / strPath(name);
}

void MoreIcons::notifyFailure(const std::string& message) {
    Notification::create(message, NotificationIcon::Error)->show();
}

void MoreIcons::notifyInfo(const std::string& message) {
    Notification::create(message, NotificationIcon::Info)->show();
}

void MoreIcons::notifySuccess(const std::string& message) {
    Notification::create(message, NotificationIcon::Success)->show();
}


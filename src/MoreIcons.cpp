#define FMT_CPP_LIB_FILESYSTEM 0
#include "MoreIcons.hpp"
#include "api/IconInfoImpl.hpp"
#include "classes/misc/ThreadPool.hpp"
#include "utils/Defaults.hpp"
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
#include <MoreIcons.hpp>
#include <std23/function_ref.h>

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
    { IconType::DeathEffect, {} },
    { IconType::Special, {} },
    { IconType::ShipFire, {} }
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
    { IconType::DeathEffect, 0 },
    { IconType::Special, 0 },
    { IconType::ShipFire, 0 }
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
            auto filename = std::basic_string(MoreIcons::getPathFilename(dest));
            auto dot = filename.find_last_of(MI_PATH('.'));
            if (dot == -1) dot = filename.size();
            filename.insert(dot, GEODE_WINDOWS(string::utf8ToWide)(fmt::format(" ({})", i)));
            dest.replace_filename(filename);
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

std::basic_string_view<std::filesystem::path::value_type> MoreIcons::getPathFilename(const std::filesystem::path& path) {
    std::basic_string_view filename = path.native();
    filename.remove_prefix(filename.find_last_of(std::filesystem::path::preferred_separator) + 1);
    return filename;
}

std::filesystem::path::string_type MoreIcons::getPathString(std::filesystem::path path) {
    return std::move(const_cast<std::filesystem::path::string_type&>(path.native()));
}

void migrateTrails(const std::filesystem::path& path) {
    log::info("Beginning trail migration in {}", path);

    auto& saveContainer = Mod::get()->getSaveContainer();
    if (!saveContainer.contains("migrated-trails")) saveContainer.set("migrated-trails", matjson::Value::array());

    auto& migratedTrails = saveContainer["migrated-trails"];

    MoreIcons::iterate(path, std::filesystem::file_type::regular, [&migratedTrails](const std::filesystem::path& path) {
        auto filename = MoreIcons::getPathFilename(path);
        if (!filename.ends_with(MI_PATH(".png"))) return;

        if (std::ranges::contains(migratedTrails, path)) return;
        else migratedTrails.push(path);

        auto parentDir = path.parent_path();
        auto stem = filename.substr(0, filename.size() - 4);
        auto directory = parentDir / stem;

        if (auto res = file::createDirectoryAll(directory); res.isErr()) {
            return log::error("Failed to create trail directory {}: {}", stem, res.unwrapErr());
        }

        if (auto res = MoreIcons::renameFile(path, directory / MI_PATH("trail.png")); res.isErr()) {
            return log::error("Failed to move {}: {}", filename, res.unwrapErr());
        }

        auto jsonName = std::basic_string(stem) + MI_PATH(".json");
        if (auto res = MoreIcons::renameFile(parentDir / jsonName, directory / MI_PATH("settings.json")); res.isErr()) {
            return log::error("Failed to move {}: {}", jsonName, res.unwrapErr());
        }
    });

    log::info("Finished trail migration in {}", path);
}

struct IconPack {
    std::string name;
    std::string id;
    std::filesystem::path path;
    bool vanilla;
    bool zipped;
};

std::vector<IconPack> packs;
float factor = 0.0f;

void MoreIcons::loadPacks() {
    factor = Get::Director()->getContentScaleFactor();
    packs.clear();
    packs.emplace_back("More Icons", std::string(), dirs::getGeodeDir(), false, false);
    migrateTrails(Mod::get()->getConfigDir().make_preferred() / MI_PATH("trail"));

    for (auto& pack : texture_loader::getAppliedPacks()) {
        auto& name = pack.name;
        auto& id = pack.id;
        auto& resourcesPath = pack.resourcesPath;

        resourcesPath.make_preferred();

        std::basic_string_view str = pack.path.native();
        auto zipped = str.ends_with(MI_PATH(".apk")) || str.ends_with(MI_PATH(".zip"));
        if (traditionalPacks) {
            if (doesExist(resourcesPath / MI_PATH("icons"))) {
                packs.emplace_back(name, id, resourcesPath, true, zipped);
            }
            else {
                std::error_code code;
                std::filesystem::directory_iterator it(resourcesPath, code);
                if (code) log::error("{}: Failed to create directory iterator: {}", resourcesPath, code.message());
                else {
                    for (; it != std::filesystem::end(it); it.increment(code)) {
                        std::error_code code;
                        if (it->status(code).type() != std::filesystem::file_type::regular) continue;

                        auto filename = MoreIcons::getPathFilename(it->path());
                        if (
                            (filename.starts_with(MI_PATH("streak_")) && filename.ends_with(MI_PATH("_001.png"))) ||
                            (filename.starts_with(MI_PATH("PlayerExplosion_")) && filename.ends_with(MI_PATH(".png"))) ||
                            (filename.starts_with(MI_PATH("shipfire")) && filename.ends_with(MI_PATH("_001.png")))
                        ) {
                            packs.emplace_back(name, id, resourcesPath, true, zipped);
                            break;
                        }
                    }
                }
                if (code) log::error("{}: Failed to iterate over directory: {}", resourcesPath, code.message());
            }
        }

        auto configPath = resourcesPath / MI_PATH_ID;
        if (doesExist(configPath)) {
            packs.emplace_back(name, id, resourcesPath, false, zipped);
            migrateTrails(configPath / MI_PATH("trail"));
        }
    }
}

#ifdef GEODE_IS_WINDOWS
std::filesystem::path MoreIcons::strPath(std::string_view path) {
    auto count = MultiByteToWideChar(CP_UTF8, 0, path.data(), path.size(), nullptr, 0);
    std::wstring str(count, 0);
    if (count != 0) MultiByteToWideChar(CP_UTF8, 0, path.data(), path.size(), &str[0], count);
    return std::filesystem::path(std::move(str));
}

Result<> checkPath(const std::filesystem::path& path) {
    auto& wstr = path.native();
    auto count = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), nullptr, 0, nullptr, nullptr);
    if (count == 0) return Err(formatSystemError(GetLastError()));

    std::string str(count, 0);
    auto result = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, &str[0], count, nullptr, nullptr);
    if (result == 0) return Err(formatSystemError(GetLastError()));
    else return Ok();
}

std::string MoreIcons::strNarrow(std::wstring_view wstr) {
    auto count = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string str(count, 0);
    if (count != 0) WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, &str[0], count, nullptr, nullptr);
    return str;
}
#else
std::filesystem::path MoreIcons::strPath(std::string_view path) {
    return std::filesystem::path(path);
}

std::string MoreIcons::strNarrow(std::string_view str) {
    return std::string(str);
}
#endif

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
    std::basic_string_view stem = path.native();
    auto sep = stem.find_last_of(std::filesystem::path::preferred_separator) + 1;
    auto start = std::basic_string(stem.substr(0, sep));
    stem.remove_prefix(sep);
    stem.remove_suffix(6);

    std::string name;
    std::string shortName;
    TextureQuality quality;
    if (stem.ends_with(MI_PATH("-uhd"))) {
        stem.remove_suffix(4);
        shortName = MoreIcons::strNarrow(stem);
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        start += stem;
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
        quality = kTextureQualityHigh;
    }
    else if (stem.ends_with(MI_PATH("-hd"))) {
        stem.remove_suffix(3);
        shortName = MoreIcons::strNarrow(stem);
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        start += stem;
        if (factor < 2.0f) {
            if (!MoreIcons::doesExist(start + MI_PATH(".plist"))) {
                printLog(name, Severity::Warning, "Ignoring medium-quality icon on low texture quality");
            }
            return;
        }

        if (factor >= 4.0f && MoreIcons::doesExist(start + MI_PATH("-uhd.plist"))) return;
        quality = kTextureQualityMedium;
    }
    else {
        shortName = MoreIcons::strNarrow(stem);
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        start += stem;
        if (factor >= 4.0f && MoreIcons::doesExist(start + MI_PATH("-uhd.plist"))) return;
        else if (factor >= 2.0f && MoreIcons::doesExist(start + MI_PATH("-hd.plist"))) return;
        quality = kTextureQualityLow;
    }

    log::debug("Pre-loading icon {} from {}", name, pack.name);

    #ifdef GEODE_IS_WINDOWS
    if (auto res = checkPath(path); res.isErr()) {
        return printLog(name, Severity::Error, "Failed to convert path: {}", res.unwrapErr());
    }
    #endif

    auto texturePath = std::filesystem::path(path).replace_extension(MI_PATH(".png"));
    if (!MoreIcons::doesExist(texturePath)) {
        return printLog(name, Severity::Error, "Texture file {} not found", texturePath.filename());
    }

    more_icons::addIcon(name, shortName, currentType, texturePath, path, quality, pack.id, pack.name, false, pack.zipped);

    log::debug("Finished pre-loading icon {} from {}", name, pack.name);
}

void loadVanillaIcon(const std::filesystem::path& path, const IconPack& pack) {
    std::basic_string_view stem = path.native();
    auto sep = stem.find_last_of(std::filesystem::path::preferred_separator) + 1;
    auto start = std::basic_string(stem.substr(0, sep));
    stem.remove_prefix(sep);
    stem.remove_suffix(4);

    auto plistPath = std::filesystem::path(path).replace_extension(MI_PATH(".plist"));
    auto vanillaPath = !MoreIcons::doesExist(plistPath);
    if (vanillaPath) plistPath = vanillaTexturePath(std::filesystem::path(MI_PATH("icons")).append(stem).concat(MI_PATH(".plist")), false);

    std::string name;
    std::string shortName;
    TextureQuality quality;
    if (stem.ends_with(MI_PATH("-uhd"))) {
        if (factor < 4.0f) return;
        stem.remove_suffix(4);
        shortName = MoreIcons::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityHigh;
    }
    else if (stem.ends_with(MI_PATH("-hd"))) {
        if (factor < 2.0f) return;
        stem.remove_suffix(3);
        start += stem;
        if (factor >= 4.0f && MoreIcons::doesExist(start + MI_PATH("-uhd.png"))) return;
        shortName = MoreIcons::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityMedium;
    }
    else {
        start += stem;
        if (factor >= 4.0f && MoreIcons::doesExist(start + MI_PATH("-uhd.png"))) return;
        else if (factor >= 2.0f && MoreIcons::doesExist(start + MI_PATH("-hd.png"))) return;
        shortName = MoreIcons::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityLow;
    }

    log::debug("Pre-loading vanilla icon {} from {}", name, pack.name);

    #ifdef GEODE_IS_WINDOWS
    if (auto res = checkPath(path); res.isErr()) {
        return printLog(name, Severity::Error, "Failed to convert path: {}", res.unwrapErr());
    }
    #endif

    #ifdef GEODE_IS_ANDROID
    auto doesntExist = vanillaPath && !Get::FileUtils()->isFileExist(plistPath.native());
    #else
    auto doesntExist = vanillaPath && !MoreIcons::doesExist(plistPath);
    #endif
    if (doesntExist) return printLog(name, Severity::Error, "Plist file not found (Last attempt: {})", plistPath);

    more_icons::addIcon(name, shortName, currentType, path, plistPath, quality, pack.id, pack.name, true, pack.zipped);

    log::debug("Finished pre-loading vanilla icon {} from {}", name, pack.name);
}

void loadTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto shortName = MoreIcons::strNarrow(MoreIcons::getPathFilename(path));
    auto name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);
    auto texturePath = path / MI_PATH("trail.png");

    if (!MoreIcons::doesExist(texturePath)) {
        return printLog(name, Severity::Error, "Texture file {} not found", texturePath.filename());
    }

    log::debug("Pre-loading trail {} from {}", name, pack.name);

    #ifdef GEODE_IS_WINDOWS
    if (auto res = checkPath(path); res.isErr()) {
        return printLog(name, Severity::Error, "Failed to convert path: {}", res.unwrapErr());
    }
    #endif

    auto jsonPath = path / MI_PATH("settings.json");

    auto iconPath = MoreIcons::getPathString(path / MI_PATH("icon"));
    if (factor >= 4.0f && MoreIcons::doesExist(iconPath + MI_PATH("-uhd.png"))) iconPath += MI_PATH("-uhd.png");
    else if (factor >= 2.0f && MoreIcons::doesExist(iconPath + MI_PATH("-hd.png"))) iconPath += MI_PATH("-hd.png");
    else if (MoreIcons::doesExist(iconPath + MI_PATH(".png"))) iconPath += MI_PATH(".png");
    else iconPath.clear();

    more_icons::addTrail(name, shortName, path, jsonPath, iconPath, pack.id, pack.name, 0,
        file::readJson(jsonPath).unwrapOr(matjson::makeObject({
            { "blend", false },
            { "tint", false },
            { "show", false },
            { "fade", 0.3f },
            { "stroke", 14.0f }
        })), false, pack.zipped);

    log::debug("Finished pre-loading trail {} from {}", name, pack.name);
}

void loadVanillaTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto wideName = MoreIcons::getPathFilename(path);
    wideName.remove_suffix(4);
    auto shortName = MoreIcons::strNarrow(wideName);
    auto name = fmt::format("{}:{}", pack.id, shortName);

    log::debug("Pre-loading vanilla trail {} from {}", name, pack.name);

    #ifdef GEODE_IS_WINDOWS
    if (auto res = checkPath(path); res.isErr()) {
        return printLog(name, Severity::Error, "Failed to convert path: {}", res.unwrapErr());
    }
    #endif

    auto trailID = jasmine::convert::getInt<int>(std::string_view(shortName).substr(7, shortName.size() - 11)).value_or(0);
    if (trailID == 0) trailID = -1;

    more_icons::addTrail(name, shortName, path, {}, {}, pack.id, pack.name, trailID, Defaults::getTrailInfo(trailID), true, pack.zipped);

    log::debug("Finished pre-loading vanilla trail {} from {}", name, pack.name);
}

void loadDeathEffect(const std::filesystem::path& path, const IconPack& pack) {
    auto shortName = MoreIcons::strNarrow(MoreIcons::getPathFilename(path));
    auto name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);
    auto uhdPath = path / MI_PATH("effect-uhd.plist");
    auto hdPath = path / MI_PATH("effect-hd.plist");
    auto sdPath = path / MI_PATH("effect.plist");

    TextureQuality quality;
    std::filesystem::path plistPath;
    if (factor >= 4.0f && MoreIcons::doesExist(uhdPath)) {
        plistPath = std::move(uhdPath);
        quality = kTextureQualityHigh;
    }
    else if (factor >= 2.0f && MoreIcons::doesExist(hdPath)) {
        plistPath = std::move(hdPath);
        quality = kTextureQualityMedium;
    }
    else if (MoreIcons::doesExist(sdPath)) {
        plistPath = std::move(sdPath);
        quality = kTextureQualityLow;
    }
    else {
        return printLog(name, Severity::Warning, "No compatible death effect plist found");
    }

    log::debug("Pre-loading death effect {} from {}", name, pack.name);

    #ifdef GEODE_IS_WINDOWS
    if (auto res = checkPath(path); res.isErr()) {
        return printLog(name, Severity::Error, "Failed to convert path: {}", res.unwrapErr());
    }
    #endif

    auto texturePath = std::filesystem::path(plistPath).replace_extension(MI_PATH(".png"));
    if (!MoreIcons::doesExist(texturePath)) {
        return printLog(name, Severity::Error, "Texture file {} not found", texturePath.filename());
    }

    auto jsonPath = path / MI_PATH("settings.json");

    auto iconPath = MoreIcons::getPathString(path / MI_PATH("icon"));
    if (factor >= 4.0f && MoreIcons::doesExist(iconPath + MI_PATH("-uhd.png"))) iconPath += MI_PATH("-uhd.png");
    else if (factor >= 2.0f && MoreIcons::doesExist(iconPath + MI_PATH("-hd.png"))) iconPath += MI_PATH("-hd.png");
    else if (MoreIcons::doesExist(iconPath + MI_PATH(".png"))) iconPath += MI_PATH(".png");
    else iconPath.clear();

    more_icons::addDeathEffect(name, shortName, texturePath, plistPath, jsonPath, iconPath,
        quality, pack.id, pack.name, 0, file::readJson(jsonPath).unwrapOrDefault(), false, pack.zipped);

    log::debug("Finished pre-loading death effect {} from {}", name, pack.name);
}

void loadVanillaDeathEffect(const std::filesystem::path& path, const IconPack& pack) {
    std::basic_string_view stem = path.native();
    auto sep = stem.find_last_of(std::filesystem::path::preferred_separator) + 1;
    auto start = std::basic_string(stem.substr(0, sep));
    stem.remove_prefix(sep);
    stem.remove_suffix(4);

    auto plistPath = std::filesystem::path(path).replace_extension(MI_PATH(".plist"));
    auto vanillaPath = !MoreIcons::doesExist(plistPath);
    if (vanillaPath) plistPath = vanillaTexturePath(std::filesystem::path(MI_PATH("icons")).append(stem).concat(MI_PATH(".plist")), false);

    std::string name;
    std::string shortName;
    TextureQuality quality;
    if (stem.ends_with(MI_PATH("-uhd"))) {
        if (factor < 4.0f) return;
        stem.remove_suffix(4);
        shortName = MoreIcons::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityHigh;
    }
    else if (stem.ends_with(MI_PATH("-hd"))) {
        if (factor < 2.0f) return;
        stem.remove_suffix(3);
        start += stem;
        if (factor >= 4.0f && MoreIcons::doesExist(start + MI_PATH("-uhd.png"))) return;
        shortName = MoreIcons::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityMedium;
    }
    else {
        start += stem;
        if (factor >= 4.0f && MoreIcons::doesExist(start + MI_PATH("-uhd.png"))) return;
        else if (factor >= 2.0f && MoreIcons::doesExist(start + MI_PATH("-hd.png"))) return;
        shortName = MoreIcons::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityLow;
    }

    log::debug("Pre-loading vanilla death effect {} from {}", name, pack.name);

    #ifdef GEODE_IS_WINDOWS
    if (auto res = checkPath(path); res.isErr()) {
        return printLog(name, Severity::Error, "Failed to convert path: {}", res.unwrapErr());
    }
    #endif

    #ifdef GEODE_IS_ANDROID
    auto doesntExist = vanillaPath && !Get::FileUtils()->isFileExist(plistPath.native());
    #else
    auto doesntExist = vanillaPath && !MoreIcons::doesExist(plistPath);
    #endif
    if (doesntExist) return printLog(name, Severity::Error, "Plist file not found (Last attempt: {})", plistPath);

    auto effectID = jasmine::convert::getInt<int>(std::string_view(shortName).substr(16)).value_or(0);
    if (effectID == 0) effectID = -1;
    else effectID++;

    more_icons::addDeathEffect(name, shortName, path, plistPath, {}, {}, quality, pack.id, pack.name,
        effectID, {}, true, pack.zipped);

    log::debug("Finished pre-loading vanilla death effect {} from {}", name, pack.name);
}

void loadShipFire(const std::filesystem::path& path, const IconPack& pack) {
    auto shortName = MoreIcons::strNarrow(MoreIcons::getPathFilename(path));
    auto name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

    auto fireCount = 0;
    MoreIcons::iterate(path, std::filesystem::file_type::regular, [&fireCount](const std::filesystem::path& path) {
        auto filename = MoreIcons::getPathFilename(path);
        if (filename == GEODE_WINDOWS(string::utf8ToWide)(fmt::format("fire_{:03}.png", fireCount + 1))) fireCount++;
    });
    if (fireCount == 0) return printLog(name, Severity::Error, "No ship fire frames found");

    log::debug("Pre-loading ship fire {} from {}", name, pack.name);

    #ifdef GEODE_IS_WINDOWS
    if (auto res = checkPath(path); res.isErr()) {
        return printLog(name, Severity::Error, "Failed to convert path: {}", res.unwrapErr());
    }
    #endif

    auto jsonPath = path / MI_PATH("settings.json");

    auto iconPath = MoreIcons::getPathString(path / MI_PATH("icon"));
    if (factor >= 4.0f && MoreIcons::doesExist(iconPath + MI_PATH("-uhd.png"))) iconPath += MI_PATH("-uhd.png");
    else if (factor >= 2.0f && MoreIcons::doesExist(iconPath + MI_PATH("-hd.png"))) iconPath += MI_PATH("-hd.png");
    else if (MoreIcons::doesExist(iconPath + MI_PATH(".png"))) iconPath += MI_PATH(".png");
    else iconPath.clear();

    more_icons::addShipFire(name, shortName, path / MI_PATH("fire_001.png"), jsonPath, iconPath,
        pack.id, pack.name, 0, file::readJson(jsonPath).unwrapOrDefault(), fireCount, false, pack.zipped);

    log::debug("Finished pre-loading ship fire {} from {}", name, pack.name);
}

void loadVanillaShipFire(const std::filesystem::path& path, const IconPack& pack) {
    auto wideName = MoreIcons::getPathFilename(path);
    wideName.remove_suffix(8);
    auto shortName = MoreIcons::strNarrow(wideName);
    auto name = fmt::format("{}:{}", pack.id, shortName);

    constexpr std::array fireCounts = { 0, 9, 10, 6, 16, 5 };

    log::debug("Pre-loading vanilla ship fire {} from {}", name, pack.name);

    #ifdef GEODE_IS_WINDOWS
    if (auto res = checkPath(path); res.isErr()) {
        return printLog(name, Severity::Error, "Failed to convert path: {}", res.unwrapErr());
    }
    #endif

    auto fireID = jasmine::convert::getInt<int>(std::string_view(shortName).substr(12)).value_or(0);
    if (fireID == 0) fireID = -1;

    more_icons::addShipFire(name, shortName, path, {}, {}, pack.id, pack.name,
        fireID, {}, true, pack.zipped);

    log::debug("Finished pre-loading vanilla ship fire {} from {}", name, pack.name);
}

#ifdef GEODE_IS_WINDOWS
static constexpr std::array wprefixes = {
    L"player_", L"ship_", L"player_ball_", L"bird_", L"dart_", L"robot_", L"spider_",
    L"swing_", L"jetpack_", L"PlayerExplosion_", L"streak_", L"", L"shipfire"
};
#else
static constexpr std::array wprefixes = MoreIcons::prefixes;
#endif

Result<ImageResult> createFrames(IconInfo* info) {
    auto impl = IconInfoImpl::getImpl(info);
    return Load::createFrames(impl->m_texture, impl->m_sheet, impl->m_name, impl->m_type);
}

CCTexture2D* addFrames(const ImageResult& image, IconInfo* info) {
    return Load::addFrames(image, IconInfoImpl::getImpl(info)->m_frameNames);
}

void MoreIcons::loadIcons(IconType type) {
    currentType = type;

    auto miType = convertType(type);
    auto prefix = wprefixes[miType];
    auto folder = folders[miType];
    auto name = lowercase[miType];

    for (auto it = packs.begin(); it != packs.end(); ++it) {
        auto& pack = *it;

        if (!pack.vanilla) {
            auto path = pack.path / MI_PATH_ID / folder;
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

            if (type <= IconType::Jetpack) {
                iterate(path, std::filesystem::file_type::regular, [&pack](const std::filesystem::path& path) {
                    if (path.native().ends_with(MI_PATH(".plist"))) loadIcon(path, pack);
                });
            }
            else if (type >= IconType::DeathEffect) {
                iterate(path, std::filesystem::file_type::directory, [type, &pack](const std::filesystem::path& path) {
                    if (type == IconType::DeathEffect) loadDeathEffect(path, pack);
                    else if (type == IconType::Special) loadTrail(path, pack);
                    else if (type == IconType::ShipFire) loadShipFire(path, pack);
                });
            }

            log::info("Finished pre-loading {}s from {}", name, path);
        }
        else if (traditionalPacks) {
            auto path = type == IconType::Special ? pack.path : pack.path / MI_PATH("icons");
            if (!doesExist(path)) continue;

            log::info("Pre-loading {}s from {}", name, path);

            iterate(path, std::filesystem::file_type::regular, [type, &pack, prefix](const std::filesystem::path& path) {
                auto filename = getPathFilename(path);
                if (!filename.ends_with(MI_PATH(".png")) || !filename.starts_with(prefix)) return false;

                auto stem = filename;
                stem.remove_suffix(4);

                if (type <= IconType::Jetpack) {
                    if (type != IconType::Cube || !stem.starts_with(MI_PATH("player_ball_"))) {
                        if (!stem.ends_with(MI_PATH("00")) && !stem.ends_with(MI_PATH("00-hd")) && !stem.ends_with(MI_PATH("00-uhd"))) {
                            loadVanillaIcon(path, pack);
                        }
                    }
                }
                else if (type == IconType::DeathEffect) {
                    loadVanillaDeathEffect(path, pack);
                }
                else if (type == IconType::Special) {
                    if (filename.ends_with(MI_PATH("_001.png"))) loadVanillaTrail(path, pack);
                }
                else if (type == IconType::ShipFire) {
                    if (filename.ends_with(MI_PATH("_001.png"))) loadVanillaShipFire(path, pack);
                }
                return false;
            });

            log::info("Finished pre-loading {}s from {}", name, path);
        }
    }

    if (type == IconType::ShipFire) packs.clear();

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
            if (auto res = createFrames(info)) {
                std::unique_lock lock(imageMutex);

                images.emplace_back(std::move(res).unwrap(), info);
            }
            else log::error("{}: {}", info->getName(), res.unwrapErr());
        });
    }
    threadPool.wait();

    std::unique_lock lock(imageMutex);

    while (!images.empty()) {
        auto& [image, info] = images.front();
        addFrames(image, info);
        images.erase(images.begin());
    }

    log::info("Finished pre-loading {} {} textures", size, name);
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
    if (auto res = createFrames(info)) {
        return addFrames(res.unwrap(), info);
    }
    else if (res.isErr()) {
        log::error("{}: {}", info->getName(), res.unwrapErr());
    }
    return nullptr;
}

CCSprite* MoreIcons::customIcon(IconInfo* info) {
    auto square = CCSprite::create(info->getIconString().c_str());
    if (square && !square->getUserObject("geode.texture-loader/fallback")) return square;

    square = CCSprite::createWithSpriteFrameName("playerSquare_001.png");
    square->setColor({ 150, 150, 150 });

    auto question = CCLabelBMFont::create("?", "bigFont.fnt");
    question->setScale(0.6f);
    question->setPosition(square->getContentSize() / 2.0f);
    square->addChild(question);

    return square;
}

std::filesystem::path MoreIcons::getEditorDir(IconType type) {
    return Mod::get()->getConfigDir() / MI_PATH("editor") / folders[convertType(type)];
}

CCSpriteFrame* MoreIcons::getFrame(const char* name) {
    auto spriteFrame = Get::SpriteFrameCache()->spriteFrameByName(name);
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


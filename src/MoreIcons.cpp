#define FMT_CPP_LIB_FILESYSTEM 0
#include "MoreIcons.hpp"
#include "classes/misc/ThreadPool.hpp"
#include "utils/Constants.hpp"
#include "utils/Defaults.hpp"
#include "utils/Get.hpp"
#include "utils/Load.hpp"
#include "utils/Log.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJGarageLayer.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/loader/Dirs.hpp>
#include <geode.texture-loader/include/TextureLoader.hpp>
#include <jasmine/convert.hpp>
#include <jasmine/setting.hpp>
#include <MoreIcons.hpp>
#include <std23/function_ref.h>

using namespace geode::prelude;

std::string std::filesystem::format_as(const std::filesystem::path& p) {
    return utils::string::pathToString(p);
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
    return std::filesystem::exists(path, code);
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
            auto filename = getPathString(to.filename());
            auto dot = filename.find_last_of(L('.'));
            if (dot == -1) dot = filename.size();
            filename.insert(dot, fmt::format(L(" ({})"), i));
            dest.replace_filename(filename);
        }
    }
    std::filesystem::rename(from, dest, code);
    if (code) return Err("Failed to rename {}: {}", from.filename(), code.message());
    return Ok(std::move(dest));
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

std::basic_string_view<std::filesystem::path::value_type> MoreIcons::getPathFilename(const std::filesystem::path& path, size_t removeCount) {
    auto& str = path.native();
    auto sep = str.find_last_of(std::filesystem::path::preferred_separator) + 1;
    return std::basic_string_view(str.data() + sep, str.size() - sep - removeCount);
}

std::pair<
    std::basic_string_view<std::filesystem::path::value_type>,
    std::basic_string_view<std::filesystem::path::value_type>
> splitPath(const std::filesystem::path& path, size_t removeCount) {
    auto& str = path.native();
    auto sep = str.find_last_of(std::filesystem::path::preferred_separator) + 1;
    return {
        std::basic_string_view(str.data(), sep),
        std::basic_string_view(str.data() + sep, str.size() - sep - removeCount)
    };
}

std::filesystem::path::string_type MoreIcons::getPathString(std::filesystem::path&& path) {
    return std::move(const_cast<std::filesystem::path::string_type&>(path.native()));
}

void migrateTrails(const std::filesystem::path& path) {
    if (!MoreIcons::doesExist(path)) return;

    log::info("Beginning trail migration in {}", path);

    auto& saveContainer = Mod::get()->getSaveContainer();
    if (!saveContainer.contains("migrated-trails")) saveContainer.set("migrated-trails", matjson::Value::array());

    auto& migratedTrails = saveContainer["migrated-trails"];

    MoreIcons::iterate(path, std::filesystem::file_type::regular, [&migratedTrails](const std::filesystem::path& path) {
        auto filename = MoreIcons::getPathFilename(path);
        if (!filename.ends_with(L(".png"))) return;

        if (std::ranges::contains(migratedTrails, path)) return;
        else migratedTrails.push(path);

        auto parentDir = path.parent_path();
        auto stem = filename.substr(0, filename.size() - 4);
        auto directory = parentDir / stem;

        if (auto res = file::createDirectoryAll(directory); res.isErr()) {
            return log::error("Failed to create trail directory {}: {}", MoreIcons::strNarrow(stem), res.unwrapErr());
        }

        if (auto res = MoreIcons::renameFile(path, directory / L("trail.png")); res.isErr()) {
            return log::error("Failed to move {}: {}", MoreIcons::strNarrow(filename), res.unwrapErr());
        }

        std::filesystem::path jsonName = fmt::format(L("{}.json"), stem);
        if (auto res = MoreIcons::renameFile(parentDir / jsonName, directory / L("settings.json")); res.isErr()) {
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
    migrateTrails(std::move(Mod::get()->getConfigDir().make_preferred()) / L("trail"));

    for (auto& pack : texture_loader::getAppliedPacks()) {
        auto& name = pack.name;
        auto& id = pack.id;
        auto& resourcesPath = pack.resourcesPath;

        resourcesPath.make_preferred();

        auto& str = pack.path.native();
        auto zipped = str.ends_with(L(".apk")) || str.ends_with(L(".zip"));
        if (traditionalPacks) {
            if (doesExist(resourcesPath / L("icons"))) {
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

                        auto filename = getPathFilename(it->path());
                        if (
                            (filename.starts_with(L("streak_")) && filename.ends_with(L("_001.png"))) ||
                            (filename.starts_with(L("PlayerExplosion_")) && filename.ends_with(L(".png"))) ||
                            (filename.starts_with(L("shipfire")) && filename.ends_with(L("_001.png")))
                        ) {
                            packs.emplace_back(name, id, resourcesPath, true, zipped);
                            break;
                        }
                    }
                }
                if (code) log::error("{}: Failed to iterate over directory: {}", resourcesPath, code.message());
            }
        }

        auto configPath = resourcesPath / WIDE_CONFIG;
        if (doesExist(configPath)) {
            packs.emplace_back(name, id, resourcesPath, false, zipped);
            migrateTrails(std::move(configPath) / L("trail"));
        }
    }
}

#ifdef GEODE_IS_WINDOWS
std::wstring MoreIcons::strWide(std::string_view path) {
    auto count = MultiByteToWideChar(CP_UTF8, 0, path.data(), path.size(), nullptr, 0);
    std::wstring str(count, L'\0');
    if (count != 0) MultiByteToWideChar(CP_UTF8, 0, path.data(), path.size(), &str[0], count);
    return str;
}

Result<> checkPath(const std::filesystem::path& path) {
    auto& wstr = path.native();
    auto count = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), nullptr, 0, nullptr, nullptr);
    if (count == 0) return Err(formatSystemError(GetLastError()));

    std::string str(count, '\0');
    auto result = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, &str[0], count, nullptr, nullptr);
    if (result == 0) return Err(formatSystemError(GetLastError()));
    else return Ok();
}

std::string MoreIcons::strNarrow(std::wstring_view wstr) {
    auto count = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string str(count, '\0');
    if (count != 0) WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, &str[0], count, nullptr, nullptr);
    return str;
}
#else
std::string MoreIcons::strWide(std::string_view str) {
    return std::string(str);
}

Result<> checkPath(const std::filesystem::path& path) {
    return Ok();
}

std::string MoreIcons::strNarrow(std::string_view str) {
    return std::string(str);
}
#endif

std::filesystem::path MoreIcons::strPath(std::string_view path) {
    return std::filesystem::path(strWide(path));
}

std::filesystem::path MoreIcons::getResourcesDir(bool uhd) {
    #ifdef GEODE_IS_MOBILE
    if (uhd) return dirs::getModConfigDir() / "weebify.high-graphics-android" / GEODE_GD_VERSION_STRING;
    #endif
    return dirs::getResourcesDir();
}

std::filesystem::path vanillaTexturePath(const std::filesystem::path& path, bool skipSuffix) {
    return MoreIcons::getResourcesDir(!skipSuffix && Get::Director()->getContentScaleFactor() >= 4.0f) / path;
}

std::string MoreIcons::vanillaTexturePath(std::string_view path, bool skipSuffix) {
    return string::pathToString(::vanillaTexturePath(strPath(path), skipSuffix));
}

Result<std::filesystem::path> MoreIcons::createTrash() {
    auto trashPath = Mod::get()->getConfigDir() / L("trash");
    GEODE_UNWRAP(file::createDirectoryAll(trashPath));
    std::error_code code;
    std::filesystem::permissions(trashPath, std::filesystem::perms::all, code);
    return Ok(trashPath);
}

void loadIcon(const std::filesystem::path& path, const IconPack& pack) {
    auto [parent, stem] = splitPath(path, 6);

    std::string name;
    std::string shortName;
    TextureQuality quality;
    if (stem.ends_with(L("-uhd"))) {
        stem.remove_suffix(4);
        shortName = MoreIcons::strNarrow(stem);
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        if (factor < 4.0f && factor >= 2.0f) {
            if (
                !MoreIcons::doesExist(fmt::format(L("{}{}-hd.plist"), parent, stem)) &&
                !MoreIcons::doesExist(fmt::format(L("{}{}.plist"), parent, stem))
            ) {
                Log::warn(std::move(name), "Ignoring high-quality icon on medium texture quality");
            }
            return;
        }
        else if (factor < 2.0f) {
            if (!MoreIcons::doesExist(fmt::format(L("{}{}.plist"), parent, stem))) {
                Log::warn(std::move(name), "Ignoring high-quality icon on low texture quality");
            }
            return;
        }
        quality = kTextureQualityHigh;
    }
    else if (stem.ends_with(L("-hd"))) {
        stem.remove_suffix(3);
        shortName = MoreIcons::strNarrow(stem);
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        if (factor < 2.0f) {
            if (!MoreIcons::doesExist(fmt::format(L("{}{}.plist"), parent, stem))) {
                Log::warn(std::move(name), "Ignoring medium-quality icon on low texture quality");
            }
            return;
        }

        if (factor >= 4.0f && MoreIcons::doesExist(fmt::format(L("{}{}-uhd.plist"), parent, stem))) return;
        quality = kTextureQualityMedium;
    }
    else {
        shortName = MoreIcons::strNarrow(stem);
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        if (factor >= 4.0f && MoreIcons::doesExist(fmt::format(L("{}{}-uhd.plist"), parent, stem))) return;
        else if (factor >= 2.0f && MoreIcons::doesExist(fmt::format(L("{}{}-hd.plist"), parent, stem))) return;
        quality = kTextureQualityLow;
    }

    log::debug("Pre-loading icon {} from {}", name, pack.name);

    if (auto res = checkPath(path); res.isErr()) {
        return Log::error(std::move(name), fmt::format("Failed to convert path: {}", res.unwrapErr()));
    }

    auto texturePath = std::filesystem::path(path).replace_extension(L(".png"));
    if (!MoreIcons::doesExist(texturePath)) {
        return Log::error(std::move(name), fmt::format("Texture file {} not found", texturePath.filename()));
    }

    auto icon = more_icons::addIcon(
        std::move(name), std::move(shortName), Log::currentType, std::move(texturePath), path, quality, pack.id, pack.name, false, pack.zipped
    );

    log::debug("Finished pre-loading icon {} from {}", icon->getName(), pack.name);
}

void loadVanillaIcon(const std::filesystem::path& path, const IconPack& pack) {
    auto [parent, stem] = splitPath(path, 4);

    auto plistPath = std::filesystem::path(path).replace_extension(L(".plist"));
    auto vanillaPath = !MoreIcons::doesExist(plistPath);
    if (vanillaPath) plistPath = vanillaTexturePath(L("icons") / plistPath.filename(), false);

    std::string name;
    std::string shortName;
    TextureQuality quality;
    if (stem.ends_with(L("-uhd"))) {
        if (factor < 4.0f) return;
        stem.remove_suffix(4);
        shortName = MoreIcons::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityHigh;
    }
    else if (stem.ends_with(L("-hd"))) {
        if (factor < 2.0f) return;
        stem.remove_suffix(3);
        if (factor >= 4.0f && MoreIcons::doesExist(fmt::format(L("{}{}-uhd.png"), parent, stem))) return;
        shortName = MoreIcons::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityMedium;
    }
    else {
        if (factor >= 4.0f && MoreIcons::doesExist(fmt::format(L("{}{}-uhd.png"), parent, stem))) return;
        else if (factor >= 2.0f && MoreIcons::doesExist(fmt::format(L("{}{}-hd.png"), parent, stem))) return;
        shortName = MoreIcons::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityLow;
    }

    log::debug("Pre-loading vanilla icon {} from {}", name, pack.name);

    if (auto res = checkPath(path); res.isErr()) {
        return Log::error(std::move(name), fmt::format("Failed to convert path: {}", res.unwrapErr()));
    }

    auto doesntExist = vanillaPath && !Load::doesExist(plistPath);
    if (doesntExist) return Log::error(std::move(name), fmt::format("Plist file not found (Last attempt: {})", plistPath));

    auto icon = more_icons::addIcon(
        std::move(name), std::move(shortName), Log::currentType, path, std::move(plistPath), quality, pack.id, pack.name, true, pack.zipped
    );

    log::debug("Finished pre-loading vanilla icon {} from {}", icon->getName(), pack.name);
}

void loadTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto shortName = MoreIcons::strNarrow(MoreIcons::getPathFilename(path));
    auto name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);
    auto texturePath = path / L("trail.png");

    if (!MoreIcons::doesExist(texturePath)) {
        return Log::error(std::move(name), fmt::format("Texture file {} not found", texturePath.filename()));
    }

    log::debug("Pre-loading trail {} from {}", name, pack.name);

    if (auto res = checkPath(path); res.isErr()) {
        return Log::error(std::move(name), fmt::format("Failed to convert path: {}", res.unwrapErr()));
    }

    auto jsonPath = path / L("settings.json");

    auto iconPng = path / L("icon.png");
    auto iconHdPng = factor >= 2.0f ? path / L("icon-hd.png") : std::filesystem::path();
    auto iconUhdPng = factor >= 4.0f ? path / L("icon-uhd.png") : std::filesystem::path();

    std::filesystem::path iconPath;
    if (factor >= 4.0f && MoreIcons::doesExist(iconUhdPng)) iconPath = std::move(iconUhdPng);
    else if (factor >= 2.0f && MoreIcons::doesExist(iconHdPng)) iconPath = std::move(iconHdPng);
    else if (MoreIcons::doesExist(iconPng)) iconPath = std::move(iconPng);
    else iconPath.clear();

    auto trailInfo = file::readJson(jsonPath).unwrapOr(Defaults::getTrailInfo(0));
    auto icon = more_icons::addTrail(
        std::move(name), std::move(shortName), std::move(texturePath), std::move(jsonPath), std::move(iconPath),
        pack.id, pack.name, 0, std::move(trailInfo), false, pack.zipped
    );

    log::debug("Finished pre-loading trail {} from {}", icon->getName(), pack.name);
}

void loadVanillaTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto shortName = MoreIcons::strNarrow(MoreIcons::getPathFilename(path, 4));
    auto name = fmt::format("{}:{}", pack.id, shortName);

    log::debug("Pre-loading vanilla trail {} from {}", name, pack.name);

    if (auto res = checkPath(path); res.isErr()) {
        return Log::error(std::move(name), fmt::format("Failed to convert path: {}", res.unwrapErr()));
    }

    auto trailID = jasmine::convert::getInt<int>(std::string_view(shortName.data() + 7, shortName.size() - 11)).value_or(0);
    if (trailID == 0) trailID = -1;

    auto icon = more_icons::addTrail(
        std::move(name), std::move(shortName), path, {}, {}, pack.id, pack.name, trailID, Defaults::getTrailInfo(trailID), true, pack.zipped
    );

    log::debug("Finished pre-loading vanilla trail {} from {}", icon->getName(), pack.name);
}

void loadDeathEffect(const std::filesystem::path& path, const IconPack& pack) {
    auto shortName = MoreIcons::strNarrow(MoreIcons::getPathFilename(path));
    auto name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);
    auto uhdPath = path / L("effect-uhd.plist");
    auto hdPath = path / L("effect-hd.plist");
    auto sdPath = path / L("effect.plist");

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
        return Log::warn(std::move(name), "No compatible death effect plist found");
    }

    log::debug("Pre-loading death effect {} from {}", name, pack.name);

    if (auto res = checkPath(path); res.isErr()) {
        return Log::error(std::move(name), fmt::format("Failed to convert path: {}", res.unwrapErr()));
    }

    auto texturePath = std::filesystem::path(plistPath).replace_extension(L(".png"));
    if (!MoreIcons::doesExist(texturePath)) {
        return Log::error(std::move(name), fmt::format("Texture file {} not found", texturePath.filename()));
    }

    auto jsonPath = path / L("settings.json");

    auto iconPng = path / L("icon.png");
    auto iconHdPng = factor >= 2.0f ? path / L("icon-hd.png") : std::filesystem::path();
    auto iconUhdPng = factor >= 4.0f ? path / L("icon-uhd.png") : std::filesystem::path();

    std::filesystem::path iconPath;
    if (factor >= 4.0f && MoreIcons::doesExist(iconUhdPng)) iconPath = std::move(iconUhdPng);
    else if (factor >= 2.0f && MoreIcons::doesExist(iconHdPng)) iconPath = std::move(iconHdPng);
    else if (MoreIcons::doesExist(iconPng)) iconPath = std::move(iconPng);
    else iconPath.clear();

    auto deathInfo = file::readJson(jsonPath).unwrapOr(Defaults::getDeathEffectInfo(0));
    auto icon = more_icons::addDeathEffect(
        std::move(name), std::move(shortName), std::move(texturePath), std::move(plistPath), std::move(jsonPath), std::move(iconPath),
        quality, pack.id, pack.name, 0, std::move(deathInfo), false, pack.zipped
    );

    log::debug("Finished pre-loading death effect {} from {}", icon->getName(), pack.name);
}

void loadVanillaDeathEffect(const std::filesystem::path& path, const IconPack& pack) {
    auto [parent, stem] = splitPath(path, 4);

    auto plistPath = std::filesystem::path(path).replace_extension(L(".plist"));
    auto vanillaPath = !MoreIcons::doesExist(plistPath);
    if (vanillaPath) plistPath = vanillaTexturePath(plistPath.filename(), false);

    std::string name;
    std::string shortName;
    TextureQuality quality;
    if (stem.ends_with(L("-uhd"))) {
        if (factor < 4.0f) return;
        stem.remove_suffix(4);
        shortName = MoreIcons::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityHigh;
    }
    else if (stem.ends_with(L("-hd"))) {
        if (factor < 2.0f) return;
        stem.remove_suffix(3);
        if (factor >= 4.0f && MoreIcons::doesExist(fmt::format(L("{}{}-uhd.png"), parent, stem))) return;
        shortName = MoreIcons::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityMedium;
    }
    else {
        if (factor >= 4.0f && MoreIcons::doesExist(fmt::format(L("{}{}-uhd.png"), parent, stem))) return;
        else if (factor >= 2.0f && MoreIcons::doesExist(fmt::format(L("{}{}-hd.png"), parent, stem))) return;
        shortName = MoreIcons::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityLow;
    }

    log::debug("Pre-loading vanilla death effect {} from {}", name, pack.name);

    if (auto res = checkPath(path); res.isErr()) {
        return Log::error(std::move(name), fmt::format("Failed to convert path: {}", res.unwrapErr()));
    }

    auto doesntExist = vanillaPath && !Load::doesExist(plistPath);
    if (doesntExist) return Log::error(std::move(name), fmt::format("Plist file not found (Last attempt: {})", plistPath));

    auto effectID = jasmine::convert::getInt<int>(std::string_view(shortName.data() + 16, shortName.size() - 16)).value_or(0);
    if (effectID == 0) effectID = -1;
    else effectID++;

    auto icon = more_icons::addDeathEffect(
        std::move(name), std::move(shortName), path, std::move(plistPath), {}, {}, quality, pack.id, pack.name,
        effectID, Defaults::getDeathEffectInfo(effectID), true, pack.zipped
    );

    log::debug("Finished pre-loading vanilla death effect {} from {}", icon->getName(), pack.name);
}

void loadShipFire(const std::filesystem::path& path, const IconPack& pack) {
    auto shortName = MoreIcons::strNarrow(MoreIcons::getPathFilename(path));
    auto name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

    auto fireCount = 0;
    MoreIcons::iterate(path, std::filesystem::file_type::regular, [&fireCount](const std::filesystem::path& path) {
        auto filename = MoreIcons::getPathFilename(path);
        if (filename == fmt::format(L("fire_{:03}.png"), fireCount + 1)) fireCount++;
    });
    if (fireCount == 0) return Log::error(std::move(name), "No ship fire frames found");

    log::debug("Pre-loading ship fire {} from {}", name, pack.name);

    if (auto res = checkPath(path); res.isErr()) {
        return Log::error(std::move(name), fmt::format("Failed to convert path: {}", res.unwrapErr()));
    }

    auto jsonPath = path / L("settings.json");

    auto iconPng = path / L("icon.png");
    auto iconHdPng = factor >= 2.0f ? path / L("icon-hd.png") : std::filesystem::path();
    auto iconUhdPng = factor >= 4.0f ? path / L("icon-uhd.png") : std::filesystem::path();

    std::filesystem::path iconPath;
    if (factor >= 4.0f && MoreIcons::doesExist(iconUhdPng)) iconPath = std::move(iconUhdPng);
    else if (factor >= 2.0f && MoreIcons::doesExist(iconHdPng)) iconPath = std::move(iconHdPng);
    else if (MoreIcons::doesExist(iconPng)) iconPath = std::move(iconPng);
    else iconPath.clear();

    auto fireInfo = file::readJson(jsonPath).unwrapOr(Defaults::getShipFireInfo(0));
    auto icon = more_icons::addShipFire(
        std::move(name), std::move(shortName), path / L("fire_001.png"), std::move(jsonPath), std::move(iconPath),
        pack.id, pack.name, 0, std::move(fireInfo), fireCount, false, pack.zipped
    );

    log::debug("Finished pre-loading ship fire {} from {}", icon->getName(), pack.name);
}

void loadVanillaShipFire(const std::filesystem::path& path, const IconPack& pack) {
    auto shortName = MoreIcons::strNarrow(MoreIcons::getPathFilename(path, 8));
    auto name = fmt::format("{}:{}", pack.id, shortName);

    auto fireID = jasmine::convert::getInt<int>(std::string_view(shortName.data() + 12, shortName.size() - 12)).value_or(0);
    if (fireID == 0) fireID = -1;

    auto fireCount = Defaults::getShipFireCount(fireID);

    log::debug("Pre-loading vanilla ship fire {} from {}", name, pack.name);

    if (auto res = checkPath(path); res.isErr()) {
        return Log::error(std::move(name), fmt::format("Failed to convert path: {}", res.unwrapErr()));
    }

    auto icon = more_icons::addShipFire(
        std::move(name), std::move(shortName), path, {}, {}, pack.id, pack.name, fireID,
        Defaults::getShipFireInfo(fireID), Defaults::getShipFireCount(fireID), true, pack.zipped
    );

    log::debug("Finished pre-loading vanilla ship fire {} from {}", icon->getName(), pack.name);
}

Result<ImageResult> createFrames(IconInfo* info) {
    return Load::createFrames(info->getTexture(), info->getSheet(), info->getName(), info->getType());
}

CCTexture2D* addFrames(const ImageResult& image, IconInfo* info) {
    return Load::addFrames(image, const_cast<std::vector<std::string>&>(info->getFrameNames()));
}

void MoreIcons::loadIcons(IconType type) {
    Log::currentType = type;

    constexpr std::array prefixes = {
        L("player_"), L("ship_"), L("player_ball_"), L("bird_"), L("dart_"), L("robot_"), L("spider_"),
        L("swing_"), L("jetpack_"), L("PlayerExplosion_"), L("streak_"), L(""), L("shipfire")
    };

    std::basic_string_view<std::filesystem::path::value_type> prefix;
    switch (type) {
        case IconType::Cube: prefix = L("player_"); break;
        case IconType::Ship: prefix = L("ship_"); break;
        case IconType::Ball: prefix = L("player_ball_"); break;
        case IconType::Ufo: prefix = L("bird_"); break;
        case IconType::Wave: prefix = L("dart_"); break;
        case IconType::Robot: prefix = L("robot_"); break;
        case IconType::Spider: prefix = L("spider_"); break;
        case IconType::Swing: prefix = L("swing_"); break;
        case IconType::Jetpack: prefix = L("jetpack_"); break;
        case IconType::DeathEffect: prefix = L("PlayerExplosion_"); break;
        case IconType::Special: prefix = L("streak_"); break;
        case IconType::ShipFire: prefix = L("shipfire"); break;
        default: prefix = L(""); break;
    }
    auto folder = Constants::getFolderName(type);
    auto name = Constants::getIconLabel(type, false, true);

    for (auto it = packs.begin(); it != packs.end(); ++it) {
        auto& pack = *it;

        if (!pack.vanilla) {
            auto path = pack.path / WIDE_CONFIG / folder;
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

            log::info("Pre-loading {} from {}", name, path);

            if (type <= IconType::Jetpack) {
                iterate(path, std::filesystem::file_type::regular, [&pack](const std::filesystem::path& path) {
                    if (path.native().ends_with(L(".plist"))) loadIcon(path, pack);
                });
            }
            else if (type >= IconType::DeathEffect) {
                iterate(path, std::filesystem::file_type::directory, [type, &pack](const std::filesystem::path& path) {
                    if (type == IconType::DeathEffect) loadDeathEffect(path, pack);
                    else if (type == IconType::Special) loadTrail(path, pack);
                    else if (type == IconType::ShipFire) loadShipFire(path, pack);
                });
            }

            log::info("Finished pre-loading {} from {}", name, path);
        }
        else if (traditionalPacks) {
            auto path = type == IconType::Special ? pack.path : pack.path / L("icons");
            if (!doesExist(path)) continue;

            log::info("Pre-loading {} from {}", name, path);

            iterate(path, std::filesystem::file_type::regular, [type, &pack, prefix](const std::filesystem::path& path) {
                auto filename = getPathFilename(path);
                if (!filename.ends_with(L(".png")) || !filename.starts_with(prefix)) return;

                filename.remove_suffix(4);

                if (type <= IconType::Jetpack) {
                    if (type != IconType::Cube || !filename.starts_with(L("player_ball_"))) {
                        if (!filename.ends_with(L("00")) && !filename.ends_with(L("00-hd")) && !filename.ends_with(L("00-uhd"))) {
                            loadVanillaIcon(path, pack);
                        }
                    }
                }
                else if (type == IconType::DeathEffect) {
                    loadVanillaDeathEffect(path, pack);
                }
                else if (type == IconType::Special) {
                    if (filename.ends_with(L("_001"))) loadVanillaTrail(path, pack);
                }
                else if (type == IconType::ShipFire) {
                    if (filename.ends_with(L("_001"))) loadVanillaShipFire(path, pack);
                }
            });

            log::info("Finished pre-loading {} from {}", name, path);
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

void MoreIcons::blendStreak(CCMotionStreak* streak, IconInfo* info) {
    if (info->getSpecialInfo().get<bool>("blend").unwrapOr(true)) streak->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });
    else streak->setBlendFunc({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA });
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

CCSpriteFrame* MoreIcons::getFrame(const char* name) {
    auto spriteFrame = Get::SpriteFrameCache()->spriteFrameByName(name);
    if (!spriteFrame || spriteFrame->getTag() == 105871529) spriteFrame = nullptr;
    return spriteFrame;
}

std::filesystem::path MoreIcons::getIconDir(IconType type) {
    return Mod::get()->getConfigDir() / Constants::getFolderName(type);
}

std::filesystem::path MoreIcons::getIconStem(std::string_view name, IconType type) {
    return getIconDir(type) / strPath(name);
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

void MoreIcons::setName(CCNode* node, std::string_view name) {
    auto str = name.empty() ? nullptr : new CCString();
    if (str) {
        str->m_sString = gd::string(name.data(), name.size());
        str->autorelease();
    }
    node->setUserObject("name"_spr, str);
}

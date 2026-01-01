#include "Icons.hpp"
#include "Constants.hpp"
#include "Defaults.hpp"
#include "Filesystem.hpp"
#include "Get.hpp"
#include "Load.hpp"
#include "Log.hpp"
#include "../classes/misc/ThreadPool.hpp"
#include <Geode/loader/Dirs.hpp>
#include <geode.texture-loader/include/TextureLoader.hpp>
#include <jasmine/convert.hpp>
#include <jasmine/setting.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;

std::map<IconType, std::vector<IconInfo>> Icons::icons = {
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
std::map<int, std::map<IconType, std::string>> Icons::requestedIcons;
std::map<std::pair<std::string, IconType>, int> Icons::loadedIcons;
std::vector<IconPack> Icons::packs;
bool Icons::traditionalPacks = true;
bool Icons::preloadIcons = false;

void Icons::loadSettings() {
    auto logLevel = jasmine::setting::get<std::string>("log-level");
    auto mod = Mod::get();
    if (!mod->setSavedValue("migrated-log-level", true)) {
        const auto& data = mod->getSavedSettingsData();
        if (!data["info-logs"].asBool().unwrapOr(true)) logLevel->setValue("Warning");
        else if (!data["debug-logs"].asBool().unwrapOr(true)) logLevel->setValue("Info");
        else logLevel->setValue("Debug");
    }

    auto logLevelValue = logLevel->getValue();
    if (logLevelValue == "Info") mod->setLogLevel(Severity::Info);
    else if (logLevelValue == "Warning") mod->setLogLevel(Severity::Warning);
    else if (logLevelValue == "Error") mod->setLogLevel(Severity::Error);
    else if (logLevelValue == "None") mod->setLogLevel(Severity::cast(4));
    else mod->setLogLevel(Severity::Debug);
    traditionalPacks = jasmine::setting::getValue<bool>("traditional-packs");
    preloadIcons = jasmine::setting::getValue<bool>("preload-icons");
}

std::pair<Filesystem::PathView, Filesystem::PathView> splitPath(const std::filesystem::path& path, size_t removeCount) {
    auto filename = Filesystem::filenameView(path);
    filename.remove_suffix(removeCount);
    auto& str = path.native();
    return std::make_pair(Filesystem::PathView(str.data(), str.size() - filename.size() - removeCount), filename);
}

void migrateTrails(const std::filesystem::path& path) {
    if (!Filesystem::doesExist(path)) return;

    log::info("Beginning trail migration in {}", path);

    auto& saveContainer = Mod::get()->getSaveContainer();
    if (!saveContainer.contains("migrated-trails")) saveContainer.set("migrated-trails", matjson::Value::array());

    auto& migratedTrails = saveContainer["migrated-trails"];

    Filesystem::iterate(path, std::filesystem::file_type::regular, [&migratedTrails](const std::filesystem::path& path) {
        auto filename = Filesystem::filenameView(path);
        if (!filename.ends_with(L(".png"))) return;

        matjson::Value pathValue = string::pathToString(path);
        if (std::ranges::contains(migratedTrails, pathValue)) return;
        else migratedTrails.push(std::move(pathValue));

        auto stem = filename.substr(0, filename.size() - 4);
        auto directory = Filesystem::parentPath(path) / stem;

        if (auto res = file::createDirectoryAll(directory); res.isErr()) {
            return log::error("Failed to create trail directory {}: {}", Filesystem::strNarrow(stem), res.unwrapErr());
        }

        if (auto res = Filesystem::renameFile(path, directory / L("trail.png")); res.isErr()) {
            return log::error("Failed to move {}: {}", Filesystem::strNarrow(filename), res.unwrapErr());
        }

        if (auto res = Filesystem::renameFile(fmt::format(L("{}.json"), directory), directory / L("settings.json")); res.isErr()) {
            return log::error("Failed to move {}.json: {}", Filesystem::strNarrow(stem), res.unwrapErr());
        }
    });

    log::info("Finished trail migration in {}", path);
}

float factor = 0.0f;

void Icons::loadPacks() {
    factor = Get::Director()->getContentScaleFactor();
    packs.clear();
    packs.emplace_back(std::string("More Icons", 10), std::string(), dirs::getGeodeDir(), false, false);
    migrateTrails(std::move(Mod::get()->getConfigDir().make_preferred()) / L("trail"));

    for (auto& pack : texture_loader::getAppliedPacks()) {
        auto& name = pack.name;
        auto& id = pack.id;
        auto& resourcesPath = pack.resourcesPath;

        resourcesPath.make_preferred();

        auto& str = pack.path.native();
        auto zipped = str.ends_with(L(".apk")) || str.ends_with(L(".zip"));
        if (traditionalPacks) {
            if (Filesystem::doesExist(resourcesPath / L("icons"))) {
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

                        auto filename = Filesystem::filenameView(it->path());
                        if (
                            (filename.starts_with(L("streak_")) && filename.ends_with(L("_001.png"))) ||
                            (filename.starts_with(L("PlayerExplosion_")) && filename.ends_with(L(".png"))) ||
                            (filename.starts_with(L("shipfire")) && filename.ends_with(L("_001.png")))
                        ) {
                            packs.emplace_back(name, id, resourcesPath, true, zipped);
                            break;
                        }
                    }
                    if (code) log::error("{}: Failed to iterate over directory: {}", resourcesPath, code.message());
                }
            }
        }

        auto configPath = resourcesPath / CONFIG_PATH;
        if (Filesystem::doesExist(configPath)) {
            packs.emplace_back(name, id, resourcesPath, false, zipped);
            migrateTrails(std::move(configPath) / L("trail"));
        }
    }
}

#ifdef GEODE_IS_WINDOWS
Result<> checkPath(const std::filesystem::path& path) {
    auto& wstr = path.native();
    auto count = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), nullptr, 0, nullptr, nullptr);
    if (count == 0) return Err(formatSystemError(GetLastError()));

    std::string str(count, '\0');
    auto result = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, &str[0], count, nullptr, nullptr);
    if (result == 0) return Err(formatSystemError(GetLastError()));
    else return Ok();
}
#else
Result<> checkPath(const std::filesystem::path& path) {
    return Ok();
}
#endif

#ifdef GEODE_IS_MOBILE
std::filesystem::path Icons::getUhdResourcesDir() {
    return dirs::getModConfigDir() / L("weebify.high-graphics-android") / L(GEODE_GD_VERSION_STRING);
}

std::filesystem::path vanillaTexturePath(std::string_view path, bool skipSuffix) {
    return (!skipSuffix && Get::Director()->getContentScaleFactor() >= 4.0f ? Icons::getUhdResourcesDir() : dirs::getResourcesDir()) / path;
}
#else
std::filesystem::path vanillaTexturePath(Filesystem::PathView path, bool skipSuffix) {
    return dirs::getResourcesDir() / path;
}
#endif

#ifdef GEODE_IS_WINDOWS
std::string Icons::vanillaTexturePath(std::string_view path, bool skipSuffix) {
    return string::pathToString(::vanillaTexturePath(Filesystem::strWide(path), skipSuffix));
}
#else
std::string Icons::vanillaTexturePath(Filesystem::PathView path, bool skipSuffix) {
    return ::vanillaTexturePath(path, skipSuffix);
}
#endif

void loadIcon(const std::filesystem::path& path, const IconPack& pack) {
    auto [parent, stem] = splitPath(path, 6);

    std::string name;
    std::string shortName;
    TextureQuality quality;
    if (stem.ends_with(L("-uhd"))) {
        stem.remove_suffix(4);
        shortName = Filesystem::strNarrow(stem);
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        if (factor < 4.0f && factor >= 2.0f) {
            if (
                !Filesystem::doesExist(fmt::format(L("{}{}-hd.plist"), parent, stem)) &&
                !Filesystem::doesExist(fmt::format(L("{}{}.plist"), parent, stem))
            ) {
                Log::warn(std::move(name), "Ignoring high-quality icon on medium texture quality");
            }
            return;
        }
        else if (factor < 2.0f) {
            if (!Filesystem::doesExist(fmt::format(L("{}{}.plist"), parent, stem))) {
                Log::warn(std::move(name), "Ignoring high-quality icon on low texture quality");
            }
            return;
        }
        quality = kTextureQualityHigh;
    }
    else if (stem.ends_with(L("-hd"))) {
        stem.remove_suffix(3);
        shortName = Filesystem::strNarrow(stem);
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        if (factor < 2.0f) {
            if (!Filesystem::doesExist(fmt::format(L("{}{}.plist"), parent, stem))) {
                Log::warn(std::move(name), "Ignoring medium-quality icon on low texture quality");
            }
            return;
        }

        if (factor >= 4.0f && Filesystem::doesExist(fmt::format(L("{}{}-uhd.plist"), parent, stem))) return;
        quality = kTextureQualityMedium;
    }
    else {
        shortName = Filesystem::strNarrow(stem);
        name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

        if (factor >= 4.0f && Filesystem::doesExist(fmt::format(L("{}{}-uhd.plist"), parent, stem))) return;
        else if (factor >= 2.0f && Filesystem::doesExist(fmt::format(L("{}{}-hd.plist"), parent, stem))) return;
        quality = kTextureQualityLow;
    }

    log::debug("Pre-loading icon {} from {}", name, pack.name);

    if (auto res = checkPath(path); res.isErr()) {
        return Log::error(std::move(name), fmt::format("Failed to convert path: {}", res.unwrapErr()));
    }

    auto texturePath = Filesystem::withExt(path, L(".png"));
    if (!Filesystem::doesExist(texturePath)) {
        return Log::error(std::move(name), fmt::format("Texture file {} not found", Filesystem::filenameFormat(texturePath)));
    }

    auto icon = more_icons::addIcon(
        std::move(name), std::move(shortName), Log::currentType, std::move(texturePath), path, quality, pack.id, pack.name, false, pack.zipped
    );

    log::debug("Finished pre-loading icon {} from {}", icon->getName(), pack.name);
}

void loadVanillaIcon(const std::filesystem::path& path, const IconPack& pack) {
    auto [parent, stem] = splitPath(path, 4);

    auto plistPath = Filesystem::withExt(path, L(".plist"));
    auto vanillaPath = !Filesystem::doesExist(plistPath);
    if (vanillaPath) {
        auto filename = Filesystem::filenameView(path);
        plistPath = vanillaTexturePath(Filesystem::PathView(filename.data() - 6, filename.size() + 6), false);
    }

    std::string name;
    std::string shortName;
    TextureQuality quality;
    if (stem.ends_with(L("-uhd"))) {
        if (factor < 4.0f) return;
        stem.remove_suffix(4);
        shortName = Filesystem::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityHigh;
    }
    else if (stem.ends_with(L("-hd"))) {
        if (factor < 2.0f) return;
        stem.remove_suffix(3);
        if (factor >= 4.0f && Filesystem::doesExist(fmt::format(L("{}{}-uhd.png"), parent, stem))) return;
        shortName = Filesystem::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityMedium;
    }
    else {
        if (factor >= 4.0f && Filesystem::doesExist(fmt::format(L("{}{}-uhd.png"), parent, stem))) return;
        else if (factor >= 2.0f && Filesystem::doesExist(fmt::format(L("{}{}-hd.png"), parent, stem))) return;
        shortName = Filesystem::strNarrow(stem);
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
    auto shortName = std::string(Filesystem::strNarrow(Filesystem::filenameView(path)));
    auto name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);
    auto texturePath = path / L("trail.png");

    if (!Filesystem::doesExist(texturePath)) {
        return Log::error(std::move(name), fmt::format("Texture file {} not found", Filesystem::filenameFormat(texturePath)));
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
    if (factor >= 4.0f && Filesystem::doesExist(iconUhdPng)) iconPath = std::move(iconUhdPng);
    else if (factor >= 2.0f && Filesystem::doesExist(iconHdPng)) iconPath = std::move(iconHdPng);
    else if (Filesystem::doesExist(iconPng)) iconPath = std::move(iconPng);

    auto trailInfo = file::readJson(jsonPath).unwrapOr(Defaults::getTrailInfo(0));
    auto icon = more_icons::addTrail(
        std::move(name), std::move(shortName), std::move(texturePath), std::move(jsonPath), std::move(iconPath),
        pack.id, pack.name, 0, std::move(trailInfo), false, pack.zipped
    );

    log::debug("Finished pre-loading trail {} from {}", icon->getName(), pack.name);
}

void loadVanillaTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto filename = Filesystem::filenameView(path);
    filename.remove_suffix(4);
    auto shortName = std::string(Filesystem::strNarrow(filename));
    auto name = fmt::format("{}:{}", pack.id, shortName);

    log::debug("Pre-loading vanilla trail {} from {}", name, pack.name);

    if (auto res = checkPath(path); res.isErr()) {
        return Log::error(std::move(name), fmt::format("Failed to convert path: {}", res.unwrapErr()));
    }

    auto trailID = jasmine::convert::getOr<int>(std::string_view(shortName.data() + 7, shortName.size() - 11));
    if (trailID == 0) trailID = -1;

    auto icon = more_icons::addTrail(
        std::move(name), std::move(shortName), path, {}, {}, pack.id, pack.name, trailID, Defaults::getTrailInfo(trailID), true, pack.zipped
    );

    log::debug("Finished pre-loading vanilla trail {} from {}", icon->getName(), pack.name);
}

void loadDeathEffect(const std::filesystem::path& path, const IconPack& pack) {
    auto shortName = std::string(Filesystem::strNarrow(Filesystem::filenameView(path)));
    auto name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);
    auto uhdPath = path / L("effect-uhd.plist");
    auto hdPath = path / L("effect-hd.plist");
    auto sdPath = path / L("effect.plist");

    TextureQuality quality;
    std::filesystem::path plistPath;
    if (factor >= 4.0f && Filesystem::doesExist(uhdPath)) {
        plistPath = std::move(uhdPath);
        quality = kTextureQualityHigh;
    }
    else if (factor >= 2.0f && Filesystem::doesExist(hdPath)) {
        plistPath = std::move(hdPath);
        quality = kTextureQualityMedium;
    }
    else if (Filesystem::doesExist(sdPath)) {
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

    auto texturePath = Filesystem::withExt(plistPath, L(".png"));
    if (!Filesystem::doesExist(texturePath)) {
        return Log::error(std::move(name), fmt::format("Texture file {} not found", Filesystem::filenameFormat(texturePath)));
    }

    auto jsonPath = path / L("settings.json");

    auto iconPng = path / L("icon.png");
    auto iconHdPng = factor >= 2.0f ? path / L("icon-hd.png") : std::filesystem::path();
    auto iconUhdPng = factor >= 4.0f ? path / L("icon-uhd.png") : std::filesystem::path();

    std::filesystem::path iconPath;
    if (factor >= 4.0f && Filesystem::doesExist(iconUhdPng)) iconPath = std::move(iconUhdPng);
    else if (factor >= 2.0f && Filesystem::doesExist(iconHdPng)) iconPath = std::move(iconHdPng);
    else if (Filesystem::doesExist(iconPng)) iconPath = std::move(iconPng);

    auto deathInfo = file::readJson(jsonPath).unwrapOr(Defaults::getDeathEffectInfo(0));
    auto icon = more_icons::addDeathEffect(
        std::move(name), std::move(shortName), std::move(texturePath), std::move(plistPath), std::move(jsonPath), std::move(iconPath),
        quality, pack.id, pack.name, 0, std::move(deathInfo), false, pack.zipped
    );

    log::debug("Finished pre-loading death effect {} from {}", icon->getName(), pack.name);
}

void loadVanillaDeathEffect(const std::filesystem::path& path, const IconPack& pack) {
    auto [parent, stem] = splitPath(path, 4);

    auto plistPath = Filesystem::withExt(path, L(".plist"));
    auto vanillaPath = !Filesystem::doesExist(plistPath);
    if (vanillaPath) plistPath = vanillaTexturePath(Filesystem::filenameView(plistPath), false);

    std::string name;
    std::string shortName;
    TextureQuality quality;
    if (stem.ends_with(L("-uhd"))) {
        if (factor < 4.0f) return;
        stem.remove_suffix(4);
        shortName = Filesystem::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityHigh;
    }
    else if (stem.ends_with(L("-hd"))) {
        if (factor < 2.0f) return;
        stem.remove_suffix(3);
        if (factor >= 4.0f && Filesystem::doesExist(fmt::format(L("{}{}-uhd.png"), parent, stem))) return;
        shortName = Filesystem::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityMedium;
    }
    else {
        if (factor >= 4.0f && Filesystem::doesExist(fmt::format(L("{}{}-uhd.png"), parent, stem))) return;
        else if (factor >= 2.0f && Filesystem::doesExist(fmt::format(L("{}{}-hd.png"), parent, stem))) return;
        shortName = Filesystem::strNarrow(stem);
        name = fmt::format("{}:{}", pack.id, shortName);
        quality = kTextureQualityLow;
    }

    log::debug("Pre-loading vanilla death effect {} from {}", name, pack.name);

    if (auto res = checkPath(path); res.isErr()) {
        return Log::error(std::move(name), fmt::format("Failed to convert path: {}", res.unwrapErr()));
    }

    auto doesntExist = vanillaPath && !Load::doesExist(plistPath);
    if (doesntExist) return Log::error(std::move(name), fmt::format("Plist file not found (Last attempt: {})", plistPath));

    auto effectID = jasmine::convert::getOr<int>(std::string_view(shortName.data() + 16, shortName.size() - 16));
    if (effectID == 0) effectID = -1;
    else effectID++;

    auto icon = more_icons::addDeathEffect(
        std::move(name), std::move(shortName), path, std::move(plistPath), {}, {}, quality, pack.id, pack.name,
        effectID, Defaults::getDeathEffectInfo(effectID), true, pack.zipped
    );

    log::debug("Finished pre-loading vanilla death effect {} from {}", icon->getName(), pack.name);
}

void loadShipFire(const std::filesystem::path& path, const IconPack& pack) {
    auto shortName = std::string(Filesystem::strNarrow(Filesystem::filenameView(path)));
    auto name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

    auto fireCount = 0;
    Filesystem::iterate(path, std::filesystem::file_type::regular, [&fireCount](const std::filesystem::path& path) {
        auto filename = Filesystem::filenameView(path);
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
    if (factor >= 4.0f && Filesystem::doesExist(iconUhdPng)) iconPath = std::move(iconUhdPng);
    else if (factor >= 2.0f && Filesystem::doesExist(iconHdPng)) iconPath = std::move(iconHdPng);
    else if (Filesystem::doesExist(iconPng)) iconPath = std::move(iconPng);

    auto fireInfo = file::readJson(jsonPath).unwrapOr(Defaults::getShipFireInfo(0));
    auto icon = more_icons::addShipFire(
        std::move(name), std::move(shortName), path / L("fire_001.png"), std::move(jsonPath), std::move(iconPath),
        pack.id, pack.name, 0, std::move(fireInfo), fireCount, false, pack.zipped
    );

    log::debug("Finished pre-loading ship fire {} from {}", icon->getName(), pack.name);
}

void loadVanillaShipFire(const std::filesystem::path& path, const IconPack& pack) {
    auto filename = Filesystem::filenameView(path);
    filename.remove_suffix(8);
    auto shortName = std::string(Filesystem::strNarrow(filename));
    auto name = fmt::format("{}:{}", pack.id, shortName);

    auto fireID = jasmine::convert::getOr<int>(std::string_view(shortName.data() + 12, shortName.size() - 12));
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

void Icons::loadIcons(IconType type) {
    Log::currentType = type;

    Filesystem::PathView prefix;
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
    auto name = Constants::getPluralLowercase(type);

    for (auto it = packs.begin(); it != packs.end(); ++it) {
        auto& pack = *it;

        if (!pack.vanilla) {
            auto path = pack.path / CONFIG_PATH / folder;
            if (!Filesystem::doesExist(path)) {
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
                Filesystem::iterate(path, std::filesystem::file_type::regular, [&pack](const std::filesystem::path& path) {
                    if (path.native().ends_with(L(".plist"))) loadIcon(path, pack);
                });
            }
            else if (type >= IconType::DeathEffect) {
                Filesystem::iterate(path, std::filesystem::file_type::directory, [type, &pack](const std::filesystem::path& path) {
                    if (type == IconType::DeathEffect) loadDeathEffect(path, pack);
                    else if (type == IconType::Special) loadTrail(path, pack);
                    else if (type == IconType::ShipFire) loadShipFire(path, pack);
                });
            }

            log::info("Finished pre-loading {} from {}", name, path);
        }
        else if (traditionalPacks) {
            auto path = type <= IconType::Jetpack ? pack.path / L("icons") : pack.path;
            if (!Filesystem::doesExist(path)) continue;

            log::info("Pre-loading {} from {}", name, path);

            Filesystem::iterate(path, std::filesystem::file_type::regular, [type, &pack, prefix](const std::filesystem::path& path) {
                auto filename = Filesystem::filenameView(path);
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

CCTexture2D* Icons::createAndAddFrames(IconInfo* info) {
    if (auto res = createFrames(info)) {
        return addFrames(res.unwrap(), info);
    }
    else if (res.isErr()) {
        log::error("{}: {}", info->getName(), res.unwrapErr());
    }
    return nullptr;
}

CCSpriteFrame* Icons::getFrame(const char* name) {
    auto spriteFrame = Get::SpriteFrameCache()->spriteFrameByName(name);
    if (!spriteFrame || spriteFrame->getTag() == 105871529) spriteFrame = nullptr;
    return spriteFrame;
}

void Icons::setName(CCNode* node, std::string_view name) {
    auto str = static_cast<CCString*>(node->getUserObject("name"_spr));
    if (!str) {
        str = new CCString();
        str->autorelease();
        node->setUserObject("name"_spr, str);
    }

    if (name.empty()) {
        str->m_sString.clear();
    }
    else {
        #ifdef GEODE_IS_ANDROID
        str->m_sString = name.data();
        #else
        str->m_sString = name;
        #endif
    }
}

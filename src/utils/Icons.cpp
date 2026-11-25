#include "Icons.hpp"
#include "Constants.hpp"
#include "Defaults.hpp"
#include "Filesystem.hpp"
#include "Get.hpp"
#include "Json.hpp"
#include "Load.hpp"
#include "Log.hpp"
#include <asp/fs/DirectoryIterator.hpp>
#include <Geode/cocos/textures/CCTextureCache.h>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/EventExport.hpp>
#include <Geode/loader/Mod.hpp>
#include <jasmine/setting.hpp>
#include <MoreIcons.hpp>

using namespace geode::prelude;
using namespace std::string_literals;

std::map<int, std::map<IconType, IconInfo*>> Icons::requestedIcons;
std::unordered_map<IconInfo*, int> Icons::loadedIcons;
std::unordered_map<IconWrapper*, IconInfo*> Icons::iconWrappers;
std::vector<Hook*> Icons::hooks;
Hook* Icons::deathEffectHook = nullptr;
bool Icons::traditionalPacks = true;
bool Icons::preloadIcons = false;
bool Icons::loadingFinished = false;

void Icons::loadSettings() {
    auto logLevel = jasmine::setting::get<std::string>("log-level");
    auto mod = Mod::get();
    if (!mod->setSavedValue("migrated-log-level", true)) {
        auto& data = mod->getSavedSettingsData();
        if (!Json::get(data, "info-logs", true)) logLevel->setValue("Warning");
        else if (!Json::get(data, "debug-logs", true)) logLevel->setValue("Info");
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

std::pair<std::string_view, std::string_view> splitPath(asp::fs::PathView path, size_t removeCount) {
    auto filename = path.filename().value_or("");
    return {
        path.view().substr(0, path.size() - filename.size()),
        filename.view().substr(0, filename.size() - removeCount)
    };
}

void migrateTrails(asp::fs::PathView path) {
    if (!path.exists()) return;

    log::info("Beginning trail migration in {}", path);

    auto& migratedTrails = Mod::get()->getSaveContainer()["migrated-trails"];
    if (!migratedTrails.isArray()) migratedTrails = matjson::Value::array();

    Filesystem::iterate(path, asp::fs::FileType::RegularFile, [&migratedTrails](const asp::fs::Path& path) {
        auto filename = path.filename().value_or("").view();
        if (!filename.ends_with(".png")) return;

        matjson::Value pathValue = path.string();
        if (std::ranges::contains(migratedTrails, pathValue)) return;
        else migratedTrails.push(std::move(pathValue));

        auto stem = filename.substr(0, filename.size() - 4);
        auto directory = path.parent().value_or("") / stem;

        if (auto res = file::createDirectoryAll(directory); res.isErr()) {
            return log::error("{}: {}", stem, res.unwrapErr());
        }

        if (auto res = Filesystem::renameFile(path, directory / "trail.png"); res.isErr()) {
            return log::error("{}: {}", filename, res.unwrapErr());
        }

        if (auto res = Filesystem::renameFile(fmt::format("{}.json", directory), directory / "settings.json"); res.isErr()) {
            return log::error("{}.json: {}", stem, res.unwrapErr());
        }
    });

    log::info("Finished trail migration in {}", path);
}

struct IconPack {
    std::string name;
    std::string id;
    asp::fs::Path path;
    bool vanilla;
    bool zipped;
};

std::vector<IconPack> packs;

namespace geode::texture_loader {
    struct Pack {
        std::string id;
        std::string name;
        VersionInfo version;
        std::vector<std::string> authors;
        asp::fs::Path path;
        asp::fs::Path resourcesPath;
    };

    GEODE_EVENT_EXPORT_CALL(std::vector<Pack>, getAppliedPacks, (), (), "geode.texture-loader/getAppliedPacks"s);
}

void Icons::loadPacks() {
    packs.emplace_back("More Icons"s, std::string(), dirs::getGeodeDir(), false, false);
    migrateTrails(std::move(Mod::get()->getConfigDir().makePreferred()) / "trail");

    for (auto& pack : texture_loader::getAppliedPacks()) {
        auto& name = pack.name;
        auto& id = pack.id;
        auto& resourcesPath = pack.resourcesPath;

        resourcesPath.makePreferred();

        auto str = pack.path.view().view();
        auto zipped = str.ends_with(".apk") || str.ends_with(".zip");
        if (traditionalPacks) {
            if ((resourcesPath / "icons").exists()) {
                packs.emplace_back(name, id, resourcesPath, true, zipped);
                log::info("Added vanilla icon pack {} from {}", name, resourcesPath);
            }
            else {
                if (auto iterRes = asp::fs::DirectoryIterator::create(resourcesPath)) {
                    for (auto itemRes : std::move(iterRes).unwrap()) {
                        if (itemRes.isErr()) continue;

                        auto item = std::move(itemRes).unwrap();
                        std::error_code code;
                        if (item.type != asp::fs::FileType::RegularFile) continue;

                        auto filename = item.path.filename().value_or("").view();
                        if (
                            (filename.starts_with("streak_") && filename.ends_with("_001.png")) ||
                            (filename.starts_with("PlayerExplosion_") && filename.ends_with(".png")) ||
                            (filename.starts_with("shipfire") && filename.ends_with("_001.png"))
                        ) {
                            packs.emplace_back(name, id, resourcesPath, true, zipped);
                            log::info("Added vanilla icon pack {} from {}", name, resourcesPath);
                            break;
                        }
                    }
                }
            }
        }

        auto configPath = resourcesPath / CONFIG_PATH;
        if (configPath.exists()) {
            packs.emplace_back(name, id, resourcesPath, false, zipped);
            log::info("Added icon pack {} from {}", name, resourcesPath);
            migrateTrails(std::move(configPath) / "trail");
        }
    }
}

#ifdef GEODE_IS_MOBILE
asp::fs::Path Icons::getUhdResourcesDir() {
    return dirs::getModConfigDir() / "weebify.high-graphics-android/" GEODE_GD_VERSION_STRING;
}

asp::fs::Path Icons::vanillaTexturePath(asp::fs::PathView path, bool skipSuffix) {
    return (!skipSuffix && Get::director->getContentScaleFactor() >= 4.0f ? Icons::getUhdResourcesDir() : dirs::getResourcesDir()) / path;
}
#else
asp::fs::Path Icons::vanillaTexturePath(asp::fs::PathView path, bool skipSuffix) {
    return dirs::getResourcesDir() / path;
}
#endif

void loadIcon(asp::fs::PathView path, const IconPack& pack) {
    auto factor = Get::director->getContentScaleFactor();
    auto [parent, stem] = splitPath(path, 6);

    std::string name;
    std::string shortName;
    TextureQuality quality;
    if (stem.ends_with("-uhd")) {
        stem.remove_suffix(4);
        name = pack.id.empty() ? std::string(stem) : fmt::format("{}:{}", pack.id, stem);

        if (factor < 4.0f && factor >= 2.0f) {
            if (
                !asp::fs::Path(fmt::format("{}{}-hd.plist", parent, stem)).exists() &&
                !asp::fs::Path(fmt::format("{}{}.plist", parent, stem)).exists()
            ) {
                Log::warn(std::move(name), "Ignoring high-quality icon on medium texture quality");
            }
            return;
        }
        else if (factor < 2.0f) {
            if (!asp::fs::Path(fmt::format("{}{}.plist", parent, stem)).exists()) {
                Log::warn(std::move(name), "Ignoring high-quality icon on low texture quality");
            }
            return;
        }
        quality = kTextureQualityHigh;
    }
    else if (stem.ends_with("-hd")) {
        stem.remove_suffix(3);
        name = pack.id.empty() ? std::string(stem) : fmt::format("{}:{}", pack.id, stem);

        if (factor < 2.0f) {
            if (!asp::fs::Path(fmt::format("{}{}.plist", parent, stem)).exists()) {
                Log::warn(std::move(name), "Ignoring medium-quality icon on low texture quality");
            }
            return;
        }

        if (factor >= 4.0f && asp::fs::Path(fmt::format("{}{}-uhd.plist", parent, stem)).exists()) return;
        quality = kTextureQualityMedium;
    }
    else {
        name = pack.id.empty() ? std::string(stem) : fmt::format("{}:{}", pack.id, stem);

        if (factor >= 4.0f && asp::fs::Path(fmt::format("{}{}-uhd.plist", parent, stem)).exists()) return;
        else if (factor >= 2.0f && asp::fs::Path(fmt::format("{}{}-hd.plist", parent, stem)).exists()) return;
        quality = kTextureQualityLow;
    }

    log::debug("Pre-loading icon {} from {}", name, pack.name);

    auto texturePath = path.withExtension(".png");
    if (!texturePath.exists()) {
        return Log::error(std::move(name), fmt::format("Texture file {} not found", texturePath.filename().value_or("")));
    }

    auto icon = more_icons::addIcon(
        std::move(name), std::move(shortName), Log::currentType, std::move(texturePath), path, quality, pack.id, pack.name, false, pack.zipped
    );

    log::debug("Finished pre-loading icon {} from {}", icon->getName(), pack.name);
}

void loadVanillaIcon(asp::fs::PathView path, const IconPack& pack) {
    auto factor = Get::director->getContentScaleFactor();
    auto [parent, stem] = splitPath(path, 4);

    auto plistPath = path.withExtension(".plist");
    auto vanillaPath = !plistPath.exists();
    if (vanillaPath) {
        plistPath = Icons::vanillaTexturePath(fmt::format("{}.plist", std::string_view(stem.data() - 6, stem.size() + 6)), false);
    }

    std::string name;
    std::string shortName;
    TextureQuality quality;
    if (stem.ends_with("-uhd")) {
        if (factor < 4.0f) return;
        stem.remove_suffix(4);
        name = fmt::format("{}:{}", pack.id, stem);
        quality = kTextureQualityHigh;
    }
    else if (stem.ends_with("-hd")) {
        if (factor < 2.0f) return;
        stem.remove_suffix(3);
        if (factor >= 4.0f && asp::fs::Path(fmt::format("{}{}-uhd.png", parent, stem)).exists()) return;
        name = fmt::format("{}:{}", pack.id, stem);
        quality = kTextureQualityMedium;
    }
    else {
        if (factor >= 4.0f && asp::fs::Path(fmt::format("{}{}-uhd.png", parent, stem)).exists()) return;
        else if (factor >= 2.0f && asp::fs::Path(fmt::format("{}{}-hd.png", parent, stem)).exists()) return;
        name = fmt::format("{}:{}", pack.id, stem);
        quality = kTextureQualityLow;
    }

    log::debug("Pre-loading vanilla icon {} from {}", name, pack.name);

    auto doesntExist = vanillaPath && !Load::doesExist(plistPath);
    if (doesntExist) return Log::error(std::move(name), fmt::format("Plist file not found (Last attempt: {})", plistPath));

    auto icon = more_icons::addIcon(
        std::move(name), std::move(shortName), Log::currentType, path, std::move(plistPath), quality, pack.id, pack.name, true, pack.zipped
    );

    log::debug("Finished pre-loading vanilla icon {} from {}", icon->getName(), pack.name);
}

void loadTrail(asp::fs::PathView path, const IconPack& pack) {
    auto factor = Get::director->getContentScaleFactor();
    auto shortName = path.filename().value_or("").string();
    auto name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);
    auto texturePath = path / "trail.png";

    if (!texturePath.exists()) {
        return Log::error(std::move(name), fmt::format("Texture file {} not found", texturePath.filename().value_or("")));
    }

    log::debug("Pre-loading trail {} from {}", name, pack.name);

    auto jsonPath = path / "settings.json";

    auto iconPng = path / "icon.png";
    auto iconHdPng = factor >= 2.0f ? path / "icon-hd.png" : asp::fs::Path();
    auto iconUhdPng = factor >= 4.0f ? path / "icon-uhd.png" : asp::fs::Path();

    asp::fs::Path iconPath;
    if (factor >= 4.0f && iconUhdPng.exists()) iconPath = std::move(iconUhdPng);
    else if (factor >= 2.0f && iconHdPng.exists()) iconPath = std::move(iconHdPng);
    else if (iconPng.exists()) iconPath = std::move(iconPng);

    auto trailInfo = file::readJson(jsonPath).unwrapOr(Defaults::getTrailInfo(0));
    auto icon = more_icons::addTrail(
        std::move(name), std::move(shortName), std::move(texturePath), std::move(jsonPath), std::move(iconPath),
        pack.id, pack.name, 0, std::move(trailInfo), false, pack.zipped
    );

    log::debug("Finished pre-loading trail {} from {}", icon->getName(), pack.name);
}

void loadVanillaTrail(asp::fs::PathView path, const IconPack& pack) {
    auto filename = path.filename().value_or("");
    auto shortName = std::string(filename.string().substr(0, filename.size() - 4));
    auto name = fmt::format("{}:{}", pack.id, shortName);

    log::debug("Pre-loading vanilla trail {} from {}", name, pack.name);

    auto trailID = numFromString<int>(std::string_view(shortName.data() + 7, shortName.size() - 11)).unwrapOrDefault();
    if (trailID == 0) trailID = -1;

    auto icon = more_icons::addTrail(
        std::move(name), std::move(shortName), path, {}, {}, pack.id, pack.name, trailID, Defaults::getTrailInfo(trailID), true, pack.zipped
    );

    log::debug("Finished pre-loading vanilla trail {} from {}", icon->getName(), pack.name);
}

void loadDeathEffect(asp::fs::PathView path, const IconPack& pack) {
    auto factor = Get::director->getContentScaleFactor();
    auto shortName = path.filename().value_or("").string();
    auto name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);
    auto uhdPath = path / "effect-uhd.plist";
    auto hdPath = path / "effect-hd.plist";
    auto sdPath = path / "effect.plist";

    TextureQuality quality;
    asp::fs::Path plistPath;
    if (factor >= 4.0f && uhdPath.exists()) {
        plistPath = std::move(uhdPath);
        quality = kTextureQualityHigh;
    }
    else if (factor >= 2.0f && hdPath.exists()) {
        plistPath = std::move(hdPath);
        quality = kTextureQualityMedium;
    }
    else if (sdPath.exists()) {
        plistPath = std::move(sdPath);
        quality = kTextureQualityLow;
    }
    else {
        return Log::warn(std::move(name), "No compatible death effect plist found");
    }

    log::debug("Pre-loading death effect {} from {}", name, pack.name);

    auto texturePath = plistPath.withExtension(".png");
    if (!texturePath.exists()) {
        return Log::error(std::move(name), fmt::format("Texture file {} not found", texturePath.filename().value_or("")));
    }

    auto jsonPath = path / "settings.json";

    auto iconPng = path / "icon.png";
    auto iconHdPng = factor >= 2.0f ? path / "icon-hd.png" : asp::fs::Path();
    auto iconUhdPng = factor >= 4.0f ? path / "icon-uhd.png" : asp::fs::Path();

    asp::fs::Path iconPath;
    if (factor >= 4.0f && iconUhdPng.exists()) iconPath = std::move(iconUhdPng);
    else if (factor >= 2.0f && iconHdPng.exists()) iconPath = std::move(iconHdPng);
    else if (iconPng.exists()) iconPath = std::move(iconPng);

    auto deathInfo = file::readJson(jsonPath).unwrapOr(Defaults::getDeathEffectInfo(0));
    auto icon = more_icons::addDeathEffect(
        std::move(name), std::move(shortName), std::move(texturePath), std::move(plistPath), std::move(jsonPath), std::move(iconPath),
        quality, pack.id, pack.name, 0, std::move(deathInfo), false, pack.zipped
    );

    log::debug("Finished pre-loading death effect {} from {}", icon->getName(), pack.name);
}

void loadVanillaDeathEffect(asp::fs::PathView path, const IconPack& pack) {
    auto factor = Get::director->getContentScaleFactor();
    auto [parent, stem] = splitPath(path, 4);

    auto plistPath = path.withExtension(".plist");
    auto vanillaPath = !plistPath.exists();
    if (vanillaPath) plistPath = Icons::vanillaTexturePath(plistPath.filename().value_or(""), false);

    std::string name;
    std::string shortName;
    TextureQuality quality;
    if (stem.ends_with("-uhd")) {
        if (factor < 4.0f) return;
        stem.remove_suffix(4);
        name = fmt::format("{}:{}", pack.id, stem);
        quality = kTextureQualityHigh;
    }
    else if (stem.ends_with("-hd")) {
        if (factor < 2.0f) return;
        stem.remove_suffix(3);
        if (factor >= 4.0f && asp::fs::Path(fmt::format("{}{}-uhd.png", parent, stem)).exists()) return;
        name = fmt::format("{}:{}", pack.id, stem);
        quality = kTextureQualityMedium;
    }
    else {
        if (factor >= 4.0f && asp::fs::Path(fmt::format("{}{}-uhd.png", parent, stem)).exists()) return;
        else if (factor >= 2.0f && asp::fs::Path(fmt::format("{}{}-hd.png", parent, stem)).exists()) return;
        name = fmt::format("{}:{}", pack.id, stem);
        quality = kTextureQualityLow;
    }

    log::debug("Pre-loading vanilla death effect {} from {}", name, pack.name);

    auto doesntExist = vanillaPath && !Load::doesExist(plistPath);
    if (doesntExist) return Log::error(std::move(name), fmt::format("Plist file not found (Last attempt: {})", plistPath));

    auto effectID = numFromString<int>(std::string_view(shortName.data() + 16, shortName.size() - 16)).unwrapOrDefault();
    if (effectID == 0) effectID = -1;
    else effectID++;

    auto icon = more_icons::addDeathEffect(
        std::move(name), std::move(shortName), path, std::move(plistPath), {}, {}, quality, pack.id, pack.name,
        effectID, Defaults::getDeathEffectInfo(effectID), true, pack.zipped
    );

    log::debug("Finished pre-loading vanilla death effect {} from {}", icon->getName(), pack.name);
}

void loadShipFire(asp::fs::PathView path, const IconPack& pack) {
    auto factor = Get::director->getContentScaleFactor();
    auto shortName = path.filename().value_or("").string();
    auto name = pack.id.empty() ? shortName : fmt::format("{}:{}", pack.id, shortName);

    auto fireCount = 0;
    while ((path / fmt::format("fire_{:03}.png", fireCount + 1)).exists()) fireCount++;
    if (fireCount == 0) return Log::error(std::move(name), "No ship fire frames found");

    log::debug("Pre-loading ship fire {} from {}", name, pack.name);

    auto jsonPath = path / "settings.json";

    auto iconPng = path / "icon.png";
    auto iconHdPng = factor >= 2.0f ? path / "icon-hd.png" : asp::fs::Path();
    auto iconUhdPng = factor >= 4.0f ? path / "icon-uhd.png" : asp::fs::Path();

    asp::fs::Path iconPath;
    if (factor >= 4.0f && iconUhdPng.exists()) iconPath = std::move(iconUhdPng);
    else if (factor >= 2.0f && iconHdPng.exists()) iconPath = std::move(iconHdPng);
    else if (iconPng.exists()) iconPath = std::move(iconPng);

    auto fireInfo = file::readJson(jsonPath).unwrapOr(Defaults::getShipFireInfo(0));
    auto icon = more_icons::addShipFire(
        std::move(name), std::move(shortName), path / "fire_001.png", std::move(jsonPath), std::move(iconPath),
        fireCount, pack.id, pack.name, 0, std::move(fireInfo), false, pack.zipped
    );

    log::debug("Finished pre-loading ship fire {} from {}", icon->getName(), pack.name);
}

void loadVanillaShipFire(asp::fs::PathView path, const IconPack& pack) {
    auto filename = path.filename().value_or("");
    auto shortName = filename.string().substr(0, filename.size() - 8);
    auto name = fmt::format("{}:{}", pack.id, shortName);

    auto fireID = numFromString<int>(std::string_view(shortName.data() + 12, shortName.size() - 12)).unwrapOrDefault();
    if (fireID == 0) fireID = -1;

    auto fireCount = Defaults::getShipFireCount(fireID);

    log::debug("Pre-loading vanilla ship fire {} from {}", name, pack.name);

    auto icon = more_icons::addShipFire(
        std::move(name), std::move(shortName), path, {}, {}, Defaults::getShipFireCount(fireID),
        pack.id, pack.name, fireID, Defaults::getShipFireInfo(fireID), true, pack.zipped
    );

    log::debug("Finished pre-loading vanilla ship fire {} from {}", icon->getName(), pack.name);
}

Result<ImageResult> createFrames(IconInfo* info) {
    return Load::createFrames(info->getTexture(), info->getSheet(), info->getName(), info->getType(), {}, true);
}

CCTexture2D* addFrames(ImageResult& image, IconInfo* info) {
    return Load::addFrames(image, const_cast<std::vector<std::string>&>(info->getFrameNames()));
}

std::vector<std::pair<ImageResult, IconInfo*>> images;
std::mutex imageMutex;

void Icons::loadIcons(IconType type) {
    Log::currentType = type;

    std::string_view prefix;
    switch (type) {
        case IconType::Cube: prefix = "player_"; break;
        case IconType::Ship: prefix = "ship_"; break;
        case IconType::Ball: prefix = "player_ball_"; break;
        case IconType::Ufo: prefix = "bird_"; break;
        case IconType::Wave: prefix = "dart_"; break;
        case IconType::Robot: prefix = "robot_"; break;
        case IconType::Spider: prefix = "spider_"; break;
        case IconType::Swing: prefix = "swing_"; break;
        case IconType::Jetpack: prefix = "jetpack_"; break;
        case IconType::DeathEffect: prefix = "PlayerExplosion_"; break;
        case IconType::Special: prefix = "streak_"; break;
        case IconType::ShipFire: prefix = "shipfire"; break;
        default: prefix = ""; break;
    }
    auto folder = Constants::getFolderName(type);
    auto name = Constants::getPluralLowercase(type);

    for (auto it = packs.begin(); it != packs.end(); ++it) {
        auto& pack = *it;

        if (!pack.vanilla) {
            auto path = pack.path / CONFIG_PATH / folder;
            if (!path.exists()) {
                if (it == packs.begin()) {
                    if (auto res = file::createDirectoryAll(path); res.isErr()) log::error("{}: {}", path, res.unwrapErr());
                }
                continue;
            }

            if (it == packs.begin()) {
                if (auto res = Filesystem::giveAllPermissions(path); res.isErr()) log::error("{}: {}", path, res.unwrapErr());
            }

            log::info("Pre-loading {} from {}", name, path);

            if (type <= IconType::Jetpack) {
                Filesystem::iterate(path, asp::fs::FileType::RegularFile, [&pack](const asp::fs::Path& path) {
                    if (path.view().view().ends_with(".plist")) loadIcon(path, pack);
                });
            }
            else if (type >= IconType::DeathEffect) {
                Filesystem::iterate(path, asp::fs::FileType::Directory, [type, &pack](const asp::fs::Path& path) {
                    if (type == IconType::DeathEffect) loadDeathEffect(path, pack);
                    else if (type == IconType::Special) loadTrail(path, pack);
                    else if (type == IconType::ShipFire) loadShipFire(path, pack);
                });
            }

            log::info("Finished pre-loading {} from {}", name, path);
        }
        else if (traditionalPacks) {
            auto path = type <= IconType::Jetpack ? pack.path / "icons" : pack.path;
            if (!path.exists()) continue;

            log::info("Pre-loading {} from {}", name, path);

            Filesystem::iterate(path, asp::fs::FileType::RegularFile, [type, &pack, prefix](const asp::fs::Path& path) {
                auto filename = path.filename().value_or("").view();
                if (!filename.ends_with(".png") || !filename.starts_with(prefix)) return;

                filename.remove_suffix(4);

                if (type <= IconType::Jetpack) {
                    if (type != IconType::Cube || !filename.starts_with("player_ball_")) {
                        if (!filename.ends_with("00") && !filename.ends_with("00-hd") && !filename.ends_with("00-uhd")) {
                            loadVanillaIcon(path, pack);
                        }
                    }
                }
                else if (type == IconType::DeathEffect) {
                    loadVanillaDeathEffect(path, pack);
                }
                else if (type == IconType::Special) {
                    if (filename.ends_with("_001")) loadVanillaTrail(path, pack);
                }
                else if (type == IconType::ShipFire) {
                    if (filename.ends_with("_001")) loadVanillaShipFire(path, pack);
                }
            });

            log::info("Finished pre-loading {} from {}", name, path);
        }
    }

    if (!preloadIcons) return;

    auto icons = more_icons::getIcons(type);
    if (!icons || icons->empty()) return;

    auto size = icons->size();
    log::info("Pre-loading {} {} textures", size, name);

    images.reserve(size);

    auto& rt = runtime();
    std::vector<arc::BlockingTaskHandle<void>> tasks;
    for (auto& info : *icons) {
        tasks.push_back(rt.spawnBlocking<void>([info = &info] {
            if (auto res = createFrames(info)) {
                std::unique_lock lock(imageMutex);

                images.emplace_back(std::move(res).unwrap(), info);
            }
            else log::error("{}: {}", info->getName(), res.unwrapErr());
        }));
    }

    for (auto& task : tasks) {
        task.blockOn();
    }

    std::unique_lock lock(imageMutex);

    for (auto it = images.begin(); it != images.end(); it = images.erase(it)) {
        addFrames(it->first, it->second);
    }

    log::info("Finished pre-loading {} {} textures", size, name);
}

void Icons::finishLoading() {
    packs.clear();
    images.clear();
    loadingFinished = true;
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

CCSpriteFrame* Icons::getFrame(const std::string& name) {
    auto spriteFrame = Get::spriteFrameCache->spriteFrameByName(name.c_str());
    if (!spriteFrame || spriteFrame->getTag() == 105871529) spriteFrame = nullptr;
    return spriteFrame;
}

void Icons::setIcon(CCNode* node, IconInfo* info) {
    if (auto obj = static_cast<IconWrapper*>(node->getUserObject("info"_spr))) {
        obj->setInfo(info);
    }
    else {
        node->setUserObject("info"_spr, IconWrapper::create(info));
    }
}

void Icons::uncacheIcon(IconInfo* info) {
    if (Get::spriteFrameCache) {
        auto& frameNames = const_cast<std::vector<std::string>&>(info->getFrameNames());
        for (auto it = frameNames.begin(); it != frameNames.end(); it = frameNames.erase(it)) {
            Get::spriteFrameCache->removeSpriteFrameByName(it->c_str());
        }
    }
    if (Get::textureCache) {
        Get::textureCache->removeTextureForKey(info->getTexture().c_str());
    }
}

IconWrapper* IconWrapper::create(IconInfo* info) {
    auto ret = new IconWrapper();
    ret->setInfo(info);
    ret->autorelease();
    return ret;
}

void IconWrapper::setInfo(IconInfo* info) {
    Icons::iconWrappers.emplace(this, m_info);
    m_info = info;
}

IconWrapper::~IconWrapper() {
    Icons::iconWrappers.erase(this);
}

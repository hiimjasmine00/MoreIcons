#include "MoreIcons.hpp"
#include "api/MoreIconsAPI.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/ranges.hpp>
#include <geode.texture-loader/include/TextureLoader.hpp>

using namespace geode::prelude;

bool doesExist(const std::filesystem::path& path) {
    std::error_code code;
    auto exists = std::filesystem::exists(path, code);
    if (code) log::error("Error checking existence of {}: {}", MoreIcons::string(path), code.message());
    return exists;
}

$on_mod(Loaded) {
    auto mod = Mod::get();
    MoreIcons::debugLogs = mod->getSettingValue<bool>("debug-logs");
    MoreIcons::traditionalPacks = mod->getSettingValue<bool>("traditional-packs");
    #ifdef GEODE_IS_ANDROID
    auto assetsDir = mod->getTempDir() / "assets";
    if (doesExist(assetsDir)) {
        std::error_code cleanCode;
        std::filesystem::remove_all(assetsDir, cleanCode);
        if (cleanCode) log::error("Failed to clean assets folder: {}", cleanCode.message());
    }
    #endif
}

$on_mod(DataSaved) {
    MoreIcons::saveTrails();
}

std::vector<IconPack> MoreIcons::getTexturePacks() {
    std::vector<IconPack> packs;

    packs.push_back({
        .name = "More Icons",
        .id = "",
        .path = dirs::getGeodeDir(),
        .vanilla = false
    });
    for (auto& pack : texture_loader::getAppliedPacks()) {
        if (traditionalPacks) {
            if (doesExist(pack.resourcesPath / "icons")) packs.push_back({
                .name = pack.name,
                .id = pack.id,
                .path = pack.resourcesPath,
                .vanilla = true
            });
            else {
                auto trailCount = GameManager::get()->countForType(IconType::Special);
                for (int i = 1; i <= trailCount; i++) {
                    if (doesExist(pack.resourcesPath / fmt::format("streak_{:02}_001.png", i))) {
                        packs.push_back({
                            .name = pack.name,
                            .id = pack.id,
                            .path = pack.resourcesPath,
                            .vanilla = true
                        });
                        break;
                    }
                }
            }
        }
        if (doesExist(pack.resourcesPath / "config" / GEODE_MOD_ID)) packs.push_back({
            .name = pack.name,
            .id = pack.id,
            .path = pack.resourcesPath,
            .vanilla = false
        });
    }

    return packs;
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
    return string(dirs::getResourcesDir() / path);
}

std::string MoreIcons::string(const std::filesystem::path& path) {
    #ifdef GEODE_IS_WINDOWS
    return string::wideToUtf8(path.native());
    #else
    return path.native();
    #endif
}

bool naturalSorter(const std::string& aStr, const std::string& bStr) {
    auto a = aStr.substr(aStr.find(':') + 1);
    auto b = bStr.substr(bStr.find(':') + 1);
    auto aIt = a.begin();
    auto bIt = b.begin();

    while (aIt < a.end() && bIt < b.end()) {
        if (std::isdigit(*aIt) && std::isdigit(*bIt)) {
            std::string aNum, bNum;
            while (std::isdigit(*aIt)) aNum += *aIt++;
            while (std::isdigit(*bIt)) bNum += *bIt++;
            if (aNum != bNum) {
                if (aNum.size() != bNum.size()) return aNum.size() < bNum.size();
                for (int i = 0; i < aNum.size(); i++) {
                    if (aNum[i] != bNum[i]) return aNum[i] < bNum[i];
                }
            }
        }
        else {
            auto aLower = std::tolower(*aIt);
            auto bLower = std::tolower(*bIt);
            if (aLower != bLower) return aLower < bLower;
            aIt++;
            bIt++;
        }
    }

    return a.size() < b.size();
}

struct NaturalSorter {
    bool operator()(const std::string& a, const std::string& b) const {
        return naturalSorter(a, b);
    }
};

void naturalSort(IconType type) {
    using PackMap = std::map<std::string, std::vector<IconInfo>, NaturalSorter>;
    auto packs = ranges::reduce<PackMap>(MoreIconsAPI::icons, [type](PackMap& acc, const IconInfo& info) {
        if (info.type != type) return acc;
        auto index = info.name.find(':');
        acc[index != std::string::npos ? info.name.substr(0, index) : ""].push_back(info);
        return acc;
    });

    ranges::remove(MoreIconsAPI::icons, [type](const IconInfo& icon) { return icon.type == type; });
    ranges::push(MoreIconsAPI::icons, ranges::reduce<std::vector<IconInfo>>(packs, [](std::vector<IconInfo>& acc, const PackMap::value_type& pack) {
        auto vec = pack.second;
        std::ranges::sort(vec, [](const IconInfo& a, const IconInfo& b) { return naturalSorter(a.name, b.name); });
        return ranges::push(acc, vec);
    }));
}

std::string replaceEnd(const std::string& str, size_t end, std::string_view replace) {
    return str.substr(0, str.size() - end).append(replace);
}

template <typename... Args>
void printLog(const std::string& name, int severity, fmt::format_string<Args...> message, Args&&... args) {
    auto logMessage = fmt::format(message, std::forward<Args>(args)...);
    log::logImpl(Severity::cast(severity), Mod::get(), "{}: {}", name, logMessage);
    MoreIcons::logs.push_back({ name, logMessage, MoreIcons::currentType, severity });
    auto& currentSeverity = MoreIcons::severities[MoreIcons::currentType];
    if (currentSeverity < severity) currentSeverity = severity;
    if (MoreIcons::severity < severity) MoreIcons::severity = severity;
}

template <typename... Args>
void safeDebug(fmt::format_string<Args...> message, Args&&... args) {
    if (MoreIcons::debugLogs) log::debug(message, std::forward<Args>(args)...);
}

void loadFolderIcon(const std::filesystem::path& path, const IconPack& pack, IconType type) {
    auto pathStem = MoreIcons::string(path.stem());
    auto name = pack.id.empty() ? pathStem : fmt::format("{}:{}", pack.id, pathStem);

    safeDebug("Pre-loading folder icon {} from {}", name, pack.name);

    if (MoreIconsAPI::hasIcon(name, type)) return printLog(name, Severity::Warning, "Duplicate icon name");

    std::error_code code;
    std::filesystem::directory_iterator it(path, code);
    if (code) return printLog(name, Severity::Error, "Failed to create directory iterator: {}", code.message());

    std::vector<std::string> textures;
    std::vector<std::string> frameNames;
    auto factor = CCDirector::get()->getContentScaleFactor();

    for (; it != std::filesystem::end(it); it.increment(code)) {
        auto& subEntry = *it;
        if (!subEntry.is_regular_file()) continue;

        auto subEntryPath = subEntry.path();
        if (subEntryPath.extension() != ".png") continue;

        auto pathFilename = MoreIcons::string(subEntryPath.filename());
        auto pathString = MoreIcons::string(subEntryPath);
        std::string frameName;
        if (pathFilename.ends_with("-uhd.png")) {
            frameName = replaceEnd(pathFilename, 8, ".png");
            auto logName = fmt::format("{}/{}", name, frameName);

            if (factor < 4.0f && factor >= 2.0f) {
                if (!doesExist(replaceEnd(pathString, 8, "-hd.png")) && !doesExist(replaceEnd(pathString, 8, ".png")))
                    printLog(logName, Severity::Warning, "Ignoring high-quality frame on medium texture quality");
                continue;
            }
            else if (factor < 2.0f) {
                if (!doesExist(replaceEnd(pathString, 8, ".png")))
                    printLog(logName, Severity::Warning, "Ignoring high-quality frame on low texture quality");
                continue;
            }
        }
        else if (pathFilename.ends_with("-hd.png")) {
            frameName = replaceEnd(pathFilename, 7, ".png");
            auto logName = fmt::format("{}/{}", name, frameName);

            if (factor < 2.0f) {
                if (!doesExist(replaceEnd(pathString, 7, ".png")))
                    printLog(logName, Severity::Warning, "Ignoring medium-quality frame for low texture quality");
                continue;
            }

            if (doesExist(replaceEnd(pathString, 7, "-uhd.png")) && factor >= 4.0f) continue;
        }
        else {
            frameName = pathFilename;

            if (doesExist(replaceEnd(pathString, 4, "-uhd.png")) && factor >= 4.0f) continue;
            else if (doesExist(replaceEnd(pathString, 4, "-hd.png")) && factor >= 2.0f) continue;
        }

        textures.push_back(pathString);
        frameNames.push_back(MoreIconsAPI::getFrameName(frameName, name, type));
    }
    if (code) printLog(name, Severity::Warning, "Failed to iterate over directory: {}", code.message());

    if (!textures.empty()) MoreIconsAPI::icons.push_back({
        .name = name,
        .textures = textures,
        .frameNames = frameNames,
        .sheetName = "",
        .packName = pack.name,
        .packID = pack.id,
        .type = type,
        .trailID = 0,
        .blend = false,
        .tint = false
    });

    safeDebug("Finished pre-loading folder icon {} from {}", name, pack.name);
}

void loadFileIcon(const std::filesystem::path& path, const IconPack& pack, IconType type) {
    auto factor = CCDirector::get()->getContentScaleFactor();
    auto pathFilename = MoreIcons::string(path.filename());
    auto pathStem = MoreIcons::string(path.stem());
    auto pathString = MoreIcons::string(path);
    std::string name;
    if (pathFilename.ends_with("-uhd.plist")) {
        name = replaceEnd(pathStem, 4, "");
        if (!pack.id.empty()) name = fmt::format("{}:{}", pack.id, name);

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
        name = replaceEnd(pathStem, 3, "");
        if (!pack.id.empty()) name = fmt::format("{}:{}", pack.id, name);

        if (factor < 2.0f) {
            if (!doesExist(replaceEnd(pathString, 9, ".plist")))
                printLog(name, Severity::Warning, "Ignoring medium-quality icon on low texture quality");
            return;
        }

        if (doesExist(replaceEnd(pathString, 9, "-uhd.plist")) && factor >= 4.0f) return;
    }
    else {
        name = pathStem;
        if (!pack.id.empty()) name = fmt::format("{}:{}", pack.id, name);

        if (doesExist(replaceEnd(pathString, 6, "-uhd.plist")) && factor >= 4.0f) return;
        else if (doesExist(replaceEnd(pathString, 6, "-hd.plist")) && factor >= 2.0f) return;
    }

    safeDebug("Pre-loading file icon {} from {}", name, pack.name);

    if (MoreIconsAPI::hasIcon(name, type)) return printLog(name, Severity::Warning, "Duplicate icon name");

    auto fullTexturePath = replaceEnd(pathString, 6, ".png");
    if (!doesExist(fullTexturePath)) return printLog(name, Severity::Error, "Texture file {}.png not found", pathStem);

    MoreIconsAPI::icons.push_back({
        .name = name,
        .textures = { fullTexturePath },
        .frameNames = {},
        .sheetName = pathString,
        .packName = pack.name,
        .packID = pack.id,
        .type = type,
        .trailID = 0,
        .blend = false,
        .tint = false
    });

    safeDebug("Finished pre-loading file icon {} from {}", name, pack.name);
}

void loadVanillaIcon(const std::filesystem::path& path, const IconPack& pack, IconType type) {
    auto factor = CCDirector::get()->getContentScaleFactor();
    auto pathFilename = MoreIcons::string(path.filename());
    auto pathStem = MoreIcons::string(path.stem());
    auto pathString = MoreIcons::string(path);
    std::string name;
    if (pathFilename.ends_with("-uhd.png")) {
        if (factor < 4.0f) return;
        name = fmt::format("{}:{}", pack.id, replaceEnd(pathStem, 4, ""));
    }
    else if (pathFilename.ends_with("-hd.png")) {
        if (factor < 2.0f) return;
        if (doesExist(replaceEnd(pathString, 7, "-uhd.png")) && factor >= 4.0f) return;
        name = fmt::format("{}:{}", pack.id, replaceEnd(pathStem, 3, ""));
    }
    else {
        if (doesExist(replaceEnd(pathString, 4, "-uhd.png")) && factor >= 4.0f) return;
        else if (doesExist(replaceEnd(pathString, 4, "-hd.png")) && factor >= 2.0f) return;
        name = fmt::format("{}:{}", pack.id, pathStem);
    }

    safeDebug("Pre-loading vanilla icon {} from {}", name, pack.name);

    auto plistPath = replaceEnd(pathString, 4, ".plist");
    if (!doesExist(plistPath)) plistPath = MoreIcons::vanillaTexturePath(fmt::format("icons/{}.plist", pathStem), false);
    if (!CCFileUtils::get()->isFileExist(plistPath)) return printLog(name, Severity::Error, "Plist file not found (Last attempt: {})", plistPath);

    ranges::remove(MoreIconsAPI::icons, [name, type](const IconInfo& icon) { return icon.name == name && icon.type == type; });

    MoreIconsAPI::icons.push_back({
        .name = name,
        .textures = { pathString },
        .frameNames = {},
        .sheetName = plistPath,
        .packName = pack.name,
        .packID = pack.id,
        .type = type,
        .trailID = 0,
        .blend = false,
        .tint = false
    });

    safeDebug("Finished pre-loading vanilla icon {} from {}", name, pack.name);
}

void loadTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto pathStem = MoreIcons::string(path.stem());
    auto name = pack.id.empty() ? pathStem : fmt::format("{}:{}", pack.id, pathStem);

    safeDebug("Pre-loading trail {} from {}", name, pack.name);

    if (MoreIconsAPI::hasIcon(name, IconType::Special)) return printLog(name, Severity::Warning, "Duplicate trail name");

    auto pathString = MoreIcons::string(path);
    auto json = file::readJson(replaceEnd(pathString, 4, ".json")).unwrapOr(matjson::makeObject({
        { "blend", false },
        { "tint", false },
        { "show", false },
        { "fade", 0.3f },
        { "stroke", 14.0f }
    }));

    MoreIconsAPI::icons.push_back({
        .name = name,
        .textures = { pathString },
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
        .stroke = json.contains("stroke") ? (float)json["stroke"].asDouble().unwrapOr(14.0) : 14.0f
    });

    safeDebug("Finished pre-loading trail {} from {}", name, pack.name);
}

void loadVanillaTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto pathStem = MoreIcons::string(path.stem());
    auto name = fmt::format("{}:{}", pack.id, pathStem);

    safeDebug("Pre-loading vanilla trail {} from {}", name, pack.name);
    auto trailID = numFromString<int>(pathStem.substr(pathStem.find('_') + 1, pathStem.rfind('_') - pathStem.find('_') - 1)).unwrapOr(0);
    if (trailID <= 0) return;

    ranges::remove(MoreIconsAPI::icons, [name](const IconInfo& icon) { return icon.name == name && icon.type == IconType::Special; });

    MoreIconsAPI::icons.push_back({
        .name = name,
        .textures = { MoreIcons::string(path) },
        .frameNames = {},
        .sheetName = "",
        .packName = pack.name,
        .packID = pack.id,
        .type = IconType::Special,
        .trailID = trailID
    });

    safeDebug("Finished pre-loading vanilla trail {} from {}", name, pack.name);
}

void createDirectories(const std::filesystem::path& path) {
    std::error_code code;
    std::filesystem::create_directories(path, code);
    if (code) log::error("Failed to create directory {}: {}", MoreIcons::string(path), code.message());
}

#define DIRECTORY_ITERATOR(path) \
    std::error_code code; \
    std::filesystem::directory_iterator it(path, code); \
    if (code) { \
        log::error("{}: Failed to create directory iterator for: {}", pathString, code.message()); \
        continue; \
    } \
    for (; it != std::filesystem::end(it); it.increment(code))

#define DIRECTORY_ITERATOR_END if (code) log::error("{}: Failed to iterate over directory: {}", pathString, code.message());

void MoreIcons::loadIcons(const std::vector<IconPack>& packs, std::string_view suffix, IconType type) {
    MoreIconsAPI::iconIndices[type].first = MoreIconsAPI::icons.size();
    currentType = type;

    for (int i = 0; i < packs.size(); i++) {
        auto& pack = packs[i];

        if (!pack.vanilla) {
            auto path = pack.path / "config" / GEODE_MOD_ID / suffix;
            if (!doesExist(path)) {
                if (i == 0) createDirectories(path);
                continue;
            }

            auto pathString = string(path);
            if (i == 0) {
                std::error_code code;
                std::filesystem::permissions(path, std::filesystem::perms::all, code);
                if (code) log::error("Failed to set permissions for {}: {}", pathString, code.message());
            }

            log::info("Pre-loading {}s from {}", suffix, pathString);

            DIRECTORY_ITERATOR(path) {
                auto& entry = *it;
                auto& entryPath = entry.path();
                if (entry.is_regular_file()) {
                    if (entryPath.extension() == ".plist") loadFileIcon(entryPath, pack, type);
                }
                else if (entry.is_directory()) loadFolderIcon(entryPath, pack, type);
            }
            DIRECTORY_ITERATOR_END

            log::info("Finished pre-loading {}s from {}", suffix, pathString);
        }
        else if (traditionalPacks) {
            auto path = pack.path / "icons";
            if (!doesExist(path)) continue;
            auto pathString = string(path);

            log::info("Pre-loading {}s from {}", suffix, pathString);

            DIRECTORY_ITERATOR(path) {
                auto& entry = *it;
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                if (entryPath.extension() != ".png") continue;

                auto entryFilename = string(entryPath.filename());
                if (!entryFilename.starts_with(prefixes[(int)type]) || (type == IconType::Cube && entryFilename.starts_with("player_ball_"))) continue;

                loadVanillaIcon(entryPath, pack, type);
            }
            DIRECTORY_ITERATOR_END

            log::info("Finished pre-loading {}s from {}", suffix, pathString);
        }
    }

    naturalSort(type);

    MoreIconsAPI::iconIndices[type].second = MoreIconsAPI::icons.size();
}

void MoreIcons::loadTrails(const std::vector<IconPack>& packs) {
    MoreIconsAPI::iconIndices[IconType::Special].first = MoreIconsAPI::icons.size();
    currentType = IconType::Special;

    for (int i = 0; i < packs.size(); i++) {
        auto& pack = packs[i];

        if (!pack.vanilla) {
            auto path = pack.path / "config" / GEODE_MOD_ID / "trail";
            if (!doesExist(path)) {
                if (i == 0) createDirectories(path);
                continue;
            }

            auto pathString = string(path);
            if (i == 0) {
                std::error_code code;
                std::filesystem::permissions(path, std::filesystem::perms::all, code);
                if (code) log::error("Failed to set permissions for {}: {}", pathString, code.message());
            }

            log::info("Pre-loading trails from {}", pathString);

            DIRECTORY_ITERATOR(path) {
                auto& entry = *it;
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                if (entryPath.extension() != ".png") continue;

                loadTrail(entryPath, pack);
            }
            DIRECTORY_ITERATOR_END

            log::info("Finished pre-loading trails from {}", pathString);
        }
        else if (traditionalPacks) {
            auto pathString = string(pack.path);

            log::info("Pre-loading trails from {}", pathString);

            auto trailCount = GameManager::get()->countForType(IconType::Special);
            DIRECTORY_ITERATOR(pack.path) {
                auto& entry = *it;
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                auto entryFilename = string(entryPath.filename());
                for (int i = 1; i <= trailCount; i++) {
                    if (entryFilename == fmt::format("streak_{:02}_001.png", i)) {
                        loadVanillaTrail(entryPath, pack);
                        break;
                    }
                }
            }
            DIRECTORY_ITERATOR_END

            log::info("Finished pre-loading trails from {}", pathString);
        }
    }

    naturalSort(IconType::Special);

    MoreIconsAPI::iconIndices[IconType::Special].second = MoreIconsAPI::icons.size();
}

void MoreIcons::saveTrails() {
    for (auto& info : MoreIconsAPI::icons) {
        if (info.type != IconType::Special || info.trailID > 0) continue;
        (void)file::writeToJson(replaceEnd(info.textures[0], 4, ".json"), matjson::makeObject({
            { "blend", info.blend },
            { "tint", info.tint },
            { "show", info.show },
            { "fade", info.fade },
            { "stroke", info.stroke }
        })).inspectErr([](const std::string& err) { log::error("Failed to save trail JSON: {}", err); });
    }
}

#if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_ANDROID)
bool MoreIcons::saveToFile(const std::filesystem::path& path, CCImage* image) {
    return image->saveToFile(string(path).c_str(), false);
}

bool MoreIcons::saveToFile(const std::filesystem::path& path, void* data, int width, int height) {
    auto image = new CCImage();
    if (!image->initWithImageData(data, width * height * 4, CCImage::kFmtRawData, width, height)) {
        image->release();
        return false;
    }
    auto result = image->saveToFile(string(path).c_str(), false);
    image->release();
    return result;
}
#elif defined(GEODE_IS_MACOS) || defined(GEODE_IS_IOS)
bool MoreIcons::saveToFile(const std::filesystem::path& path, CCImage* image) {
    return saveToFile(path, image->getData(), image->getWidth(), image->getHeight());
}
#endif

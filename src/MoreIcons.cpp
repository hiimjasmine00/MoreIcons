#include "MoreIcons.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Popup.hpp>
#include <geode.texture-loader/include/TextureLoader.hpp>
#include <Geode/utils/ranges.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
    MoreIcons::DEBUG_LOGS = Mod::get()->getSettingValue<bool>("debug-logs");
    MoreIcons::TRADITIONAL_PACKS = Mod::get()->getSettingValue<bool>("traditional-packs");
    #ifdef GEODE_IS_ANDROID
    auto tempDir = Mod::get()->getTempDir();
    if (std::filesystem::exists(tempDir / "assets")) {
        std::error_code cleanCode;
        std::filesystem::remove_all(tempDir / "assets", cleanCode);
        if (cleanCode) return log::error("Failed to clean assets folder: {}", cleanCode.message());
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
        if (MoreIcons::TRADITIONAL_PACKS) {
            if (std::filesystem::exists(pack.resourcesPath / "icons")) packs.push_back({
                .name = pack.name,
                .id = pack.id,
                .path = pack.resourcesPath,
                .vanilla = true
            });
            else {
                auto trailCount = GameManager::get()->countForType(IconType::Special);
                for (int i = 1; i <= trailCount; i++) {
                    if (std::filesystem::exists(pack.resourcesPath / fmt::format("streak_{:02}_001.png", i))) {
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
        if (std::filesystem::exists(pack.resourcesPath / "config" / GEODE_MOD_ID)) packs.push_back({
            .name = pack.name,
            .id = pack.id,
            .path = pack.resourcesPath,
            .vanilla = false
        });
    }

    return packs;
}

std::string MoreIcons::vanillaTexturePath(const std::string& path, bool skipSuffix) {
    #ifdef GEODE_IS_ANDROID
    if (CCDirector::get()->getContentScaleFactor() >= 4.0f && !skipSuffix) {
        if (auto highGraphicsAndroid = Loader::get()->getLoadedMod("weebify.high-graphics-android")) {
            auto configDir = highGraphicsAndroid->getConfigDir(false) / Loader::get()->getGameVersion();
            if (std::filesystem::exists(configDir)) return configDir / path;
        }
        return path;
    }
    return fmt::format("assets/{}", path);
    #else
    return (dirs::getGameDir() / "Resources" / path).string();
    #endif
}

bool naturalSorter(const std::string& aStr, const std::string& bStr) {
    auto a = aStr.substr(aStr.find_first_of(':') + 1);
    auto b = bStr.substr(bStr.find_first_of(':') + 1);
    auto aIt = a.begin();
    auto bIt = b.begin();

    while (aIt != a.end() && bIt != b.end()) {
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

void naturalSort(IconType type) {
    auto packs = ranges::reduce<std::map<std::string, std::vector<IconInfo>>>(MoreIconsAPI::ICONS,
        [type](std::map<std::string, std::vector<IconInfo>>& acc, const IconInfo& info) {
            if (info.type != type) return acc;
            acc[info.name.substr(0, string::contains(info.name, ':') ? info.name.find_first_of(':') : 0)].push_back(info);
            return acc;
        });

    auto packIDs = ranges::map<std::vector<std::string>>(packs, [](const std::pair<std::string, std::vector<IconInfo>>& pair) {
        return pair.first;
    });
    std::ranges::sort(packIDs, naturalSorter);

    ranges::remove(MoreIconsAPI::ICONS, [type](const IconInfo& icon) { return icon.type == type; });
    ranges::push(MoreIconsAPI::ICONS, ranges::reduce<std::vector<IconInfo>>(packIDs, [&packs](std::vector<IconInfo>& acc, const std::string& packID) {
        std::ranges::sort(packs[packID], [](const IconInfo& a, const IconInfo& b) { return naturalSorter(a.name, b.name); });
        return ranges::push(acc, packs[packID]);
    }));
}

std::string replaceEnd(const std::string& str, size_t end, std::string_view replace) {
    return fmt::format("{}{}", str.substr(0, str.size() - end), replace);
}

template <typename... Args>
void printLog(Severity severity, fmt::format_string<Args...> message, Args&&... args) {
    log::vlogImpl(severity, Mod::get(), message, fmt::make_format_args(args...));
    MoreIcons::LOGS.push_back({ fmt::vformat(message, fmt::make_format_args(args...)), severity });
    if (MoreIcons::HIGHEST_SEVERITY < severity) MoreIcons::HIGHEST_SEVERITY = severity;

}

template <typename... Args>
void safeDebug(fmt::format_string<Args...> message, Args&&... args) {
    if (MoreIcons::DEBUG_LOGS) log::vlogImpl(Severity::Debug, Mod::get(), message, fmt::make_format_args(args...));
}

void loadFolderIcon(const std::filesystem::path& path, const IconPack& pack, IconType type) {
    auto scaleFactor = CCDirector::get()->getContentScaleFactor();
    auto name = pack.id.empty() ? path.stem().string() : fmt::format("{}:{}", pack.id, path.stem());

    safeDebug("Pre-loading folder icon {} from {}", name, pack.name);

    if (std::ranges::any_of(MoreIconsAPI::ICONS, [name, type](const IconInfo& icon) { return icon.name == name && icon.type == type; }) > 0)
        return printLog(Severity::Warning, "{}: Duplicate icon name", path);

    std::vector<std::string> textures;
    std::vector<std::string> frameNames;
    for (auto& subEntry : std::filesystem::directory_iterator(path)) {
        if (!subEntry.is_regular_file()) continue;

        auto subEntryPath = subEntry.path();
        if (subEntryPath.extension() != ".png") continue;

        auto pathFilename = subEntryPath.filename().string();
        auto fileQuality = kTextureQualityLow;
        if (pathFilename.ends_with("-uhd.png")) {
            if (scaleFactor < 4.0f) {
                printLog(Severity::Info, "{}: Ignoring high-quality PNG file for {} texture quality",
                    subEntryPath, scaleFactor >= 2.0f ? "medium" : "low");
                continue;
            }

            fileQuality = kTextureQualityHigh;
        }
        else if (pathFilename.ends_with("-hd.png")) {
            if (scaleFactor < 2.0f) {
                printLog(Severity::Info, "{}: Ignoring medium-quality PNG file for low texture quality", subEntryPath);
                continue;
            }

            if (std::filesystem::exists(replaceEnd(subEntryPath.string(), 7, "-uhd.png")) && scaleFactor >= 4.0f) continue;
            else fileQuality = kTextureQualityMedium;
        }
        else {
            if (std::filesystem::exists(replaceEnd(subEntryPath.string(), 4, "-uhd.png")) && scaleFactor >= 4.0f) continue;
            else if (std::filesystem::exists(replaceEnd(subEntryPath.string(), 4, "-hd.png")) && scaleFactor >= 2.0f) continue;
            else fileQuality = kTextureQualityLow;
        }

        textures.push_back(subEntryPath.string());
        frameNames.push_back(MoreIconsAPI::getFrameName(fileQuality == kTextureQualityHigh ? replaceEnd(subEntryPath.filename().string(), 8, ".png") :
            fileQuality == kTextureQualityMedium ? replaceEnd(subEntryPath.filename().string(), 7, ".png") : subEntryPath.filename().string(), name, type));
    }

    if (!textures.empty()) MoreIconsAPI::ICONS.push_back({
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
    auto scaleFactor = CCDirector::get()->getContentScaleFactor();
    auto pathFilename = path.filename().string();
    auto fileQuality = kTextureQualityLow;
    if (pathFilename.ends_with("-uhd.plist")) {
        if (scaleFactor < 4.0f) return printLog(Severity::Info, "{}: Ignoring high-quality plist file for {} texture quality",
                path, scaleFactor >= 2.0f ? "medium" : "low");

        fileQuality = kTextureQualityHigh;
    }
    else if (pathFilename.ends_with("-hd.plist")) {
        if (scaleFactor < 2.0f) return printLog(Severity::Info, "{}: Ignoring medium-quality plist file for low texture quality", path);

        if (std::filesystem::exists(replaceEnd(path.string(), 9, "-uhd.plist")) && scaleFactor >= 4.0f) return;
        else fileQuality = kTextureQualityMedium;
    }
    else {
        if (std::filesystem::exists(replaceEnd(path.string(), 6, "-uhd.plist")) && scaleFactor >= 4.0f) return;
        else if (std::filesystem::exists(replaceEnd(path.string(), 6, "-hd.plist")) && scaleFactor >= 2.0f) return;
        else fileQuality = kTextureQualityLow;
    }

    auto name = fileQuality == kTextureQualityHigh ? replaceEnd(path.stem().string(), 4, "") :
        fileQuality == kTextureQualityMedium ? replaceEnd(path.stem().string(), 3, "") : path.stem().string();
    if (!pack.id.empty()) name = fmt::format("{}:{}", pack.id, name);

    safeDebug("Pre-loading file icon {} from {}", name, pack.name);

    if (std::ranges::any_of(MoreIconsAPI::ICONS, [name, type](const IconInfo& icon) { return icon.name == name && icon.type == type; }) > 0)
        return printLog(Severity::Warning, "{}: Duplicate icon name", path);

    auto fullTexturePath = replaceEnd(path.string(), 6, ".png");
    if (!std::filesystem::exists(fullTexturePath)) return printLog(Severity::Error, "{}: Texture file {}.png not found", path, path.stem());

    MoreIconsAPI::ICONS.push_back({
        .name = name,
        .textures = { fullTexturePath },
        .frameNames = {},
        .sheetName = path.string(),
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
    auto scaleFactor = CCDirector::get()->getContentScaleFactor();
    auto pathFilename = path.filename().string();
    auto fileQuality = kTextureQualityLow;
    if (pathFilename.ends_with("-uhd.png")) {
        if (scaleFactor < 4.0f) return;
        fileQuality = kTextureQualityHigh;
    }
    else if (pathFilename.ends_with("-hd.png")) {
        if (scaleFactor < 2.0f) return;
        if (std::filesystem::exists(replaceEnd(path.string(), 7, "-uhd.png")) && scaleFactor >= 4.0f) return;
        else fileQuality = kTextureQualityMedium;
    }
    else {
        if (std::filesystem::exists(replaceEnd(path.string(), 4, "-uhd.png")) && scaleFactor >= 4.0f) return;
        else if (std::filesystem::exists(replaceEnd(path.string(), 4, "-hd.png")) && scaleFactor >= 2.0f) return;
        else fileQuality = kTextureQualityLow;
    }

    auto name = fmt::format("{}:{}", pack.id, fileQuality == kTextureQualityHigh ? replaceEnd(path.stem().string(), 4, "") :
        fileQuality == kTextureQualityMedium ? replaceEnd(path.stem().string(), 3, "") : path.stem().string());

    safeDebug("Pre-loading vanilla icon {} from {}", name, pack.name);

    auto plistPath = replaceEnd(path.string(), 4, ".plist");
    if (!std::filesystem::exists(plistPath)) plistPath = MoreIcons::vanillaTexturePath(fmt::format("icons/{}.plist", path.stem()), false);
    if (!CCFileUtils::get()->isFileExist(plistPath)) return printLog(Severity::Error, "{}: Plist file not found (Last attempt: {})", path, plistPath);

    ranges::remove(MoreIconsAPI::ICONS, [name, type](const IconInfo& icon) { return icon.name == name && icon.type == type; });

    MoreIconsAPI::ICONS.push_back({
        .name = name,
        .textures = { path.string() },
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
    auto name = pack.id.empty() ? path.stem().string() : fmt::format("{}:{}", pack.id, path.stem());

    safeDebug("Pre-loading trail {} from {}", name, pack.name);

    if (std::ranges::any_of(MoreIconsAPI::ICONS, [name](const IconInfo& icon) { return icon.name == name && icon.type == IconType::Special; }) > 0)
        return printLog(Severity::Warning, "{}: Duplicate trail name", path);

    auto json = file::readJson(replaceEnd(path.string(), 4, ".json")).unwrapOr(matjson::makeObject({
        { "blend", false },
        { "tint", false }
    }));

    MoreIconsAPI::ICONS.push_back({
        .name = name,
        .textures = { path.string() },
        .frameNames = {},
        .sheetName = "",
        .packName = pack.name,
        .packID = pack.id,
        .type = IconType::Special,
        .trailID = 0,
        .blend = json.contains("blend") ? json["blend"].asBool().unwrapOr(false) : false,
        .tint = json.contains("tint") ? json["tint"].asBool().unwrapOr(false) : false
    });

    safeDebug("Finished pre-loading trail {} from {}", name, pack.name);
}

void loadVanillaTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto pathStem = path.stem().string();
    auto name = fmt::format("{}:{}", pack.id, pathStem);

    safeDebug("Pre-loading vanilla trail {} from {}", name, pack.name);
    auto trailID = numFromString<int>(pathStem.substr(pathStem.find_first_of('_') + 1,
        pathStem.find_last_of('_') - pathStem.find_first_of('_') - 1)).unwrapOr(0);
    if (trailID <= 0) return;

    ranges::remove(MoreIconsAPI::ICONS, [name](const IconInfo& icon) { return icon.name == name && icon.type == IconType::Special; });

    MoreIconsAPI::ICONS.push_back({
        .name = name,
        .textures = { path.string() },
        .frameNames = {},
        .sheetName = "",
        .packName = pack.name,
        .packID = pack.id,
        .type = IconType::Special,
        .trailID = trailID,
        .blend = false,
        .tint = false
    });

    safeDebug("Finished pre-loading vanilla trail {} from {}", name, pack.name);
}

void MoreIcons::loadIcons(const std::vector<IconPack>& packs, std::string_view suffix, IconType type) {
    for (int i = 0; i < packs.size(); i++) {
        auto& pack = packs[i];

        if (!pack.vanilla) {
            auto path = pack.path / "config" / GEODE_MOD_ID / suffix;
            if (!std::filesystem::exists(path)) {
                if (i == 0) std::filesystem::create_directories(path);
                continue;
            }

            log::info("Pre-loading {}s from {}", suffix, path);
            for (auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    auto& entryPath = entry.path();
                    if (entryPath.extension() != ".plist") continue;

                    loadFileIcon(entryPath, pack, type);
                }
                else if (entry.is_directory()) loadFolderIcon(entry.path(), pack, type);
            }

            log::info("Finished pre-loading {}s from {}", suffix, path);
        }
        else {
            if (!TRADITIONAL_PACKS) continue;

            auto path = pack.path / "icons";
            if (!std::filesystem::exists(path)) continue;

            log::info("Pre-loading {}s from {}", suffix, path);
            auto prefix = "";
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
                default: break;
            }
            for (auto& entry : std::filesystem::directory_iterator(path)) {
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                if (entryPath.extension() != ".png" || !entryPath.filename().string().starts_with(prefix) ||
                    (type == IconType::Cube && string::contains(entryPath.filename().string(), "player_ball_"))) continue; // Nice one RobTop

                loadVanillaIcon(entryPath, pack, type);
            }

            log::info("Finished pre-loading {}s from {}", suffix, path);
        }
    }

    naturalSort(type);
}

void MoreIcons::loadTrails(const std::vector<IconPack>& packs) {
    for (int i = 0; i < packs.size(); i++) {
        auto& pack = packs[i];

        if (!pack.vanilla) {
            auto path = pack.path / "config" / GEODE_MOD_ID / "trail";
            if (!std::filesystem::exists(path)) {
                if (i == 0) std::filesystem::create_directories(path);
                continue;
            }

            log::info("Pre-loading trails from {}", path);
            for (auto& entry : std::filesystem::directory_iterator(path)) {
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                if (entryPath.extension() != ".png") continue;

                loadTrail(entryPath, {
                    .name = pack.name,
                    .id = pack.id
                });
            }

            log::info("Finished pre-loading trails from {}", path);
        }
        else {
            if (!TRADITIONAL_PACKS) continue;

            log::info("Pre-loading trails from {}", pack.path);
            auto trailCount = GameManager::get()->countForType(IconType::Special);
            for (auto& entry : std::filesystem::directory_iterator(pack.path)) {
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                auto entryFilename = entryPath.filename().string();
                for (int i = 1; i <= trailCount; i++) {
                    if (entryFilename == fmt::format("streak_{:02}_001.png", i)) {
                        loadVanillaTrail(entryPath, {
                            .name = pack.name,
                            .id = pack.id
                        });
                        break;
                    }
                }
            }

            log::info("Finished pre-loading trails from {}", pack.path);
        }
    }

    naturalSort(IconType::Special);
}

void MoreIcons::saveTrails() {
    for (auto& info : MoreIconsAPI::ICONS) {
        if (info.type != IconType::Special || info.trailID > 0) continue;
        (void)file::writeToJson(replaceEnd(info.textures[0], 4, ".json"), matjson::makeObject({
            { "blend", info.blend },
            { "tint", info.tint },
        })).mapErr([](const std::string& err) {
            return log::error("Failed to save trail JSON: {}", err), err;
        });
    }
}

void MoreIcons::showInfoPopup(bool folderButton) {
    createQuickPopup(
        "More Icons",
        fmt::format(std::locale(""),
            "<cg>Icons</c>: {:L}\n"
            "<cp>Ships</c>: {:L}\n"
            "<cr>Balls</c>: {:L}\n"
            "<co>UFOs</c>: {:L}\n"
            "<cj>Waves</c>: {:L}\n"
            "Robots: {:L}\n"
            "<ca>Spiders</c>: {:L}\n"
            "<cy>Swings</c>: {:L}\n"
            "<cd>Jetpacks</c>: {:L}\n"
            "<cb>Trails</c>: {:L}",
            MoreIconsAPI::countForType(IconType::Cube),
            MoreIconsAPI::countForType(IconType::Ship),
            MoreIconsAPI::countForType(IconType::Ball),
            MoreIconsAPI::countForType(IconType::Ufo),
            MoreIconsAPI::countForType(IconType::Wave),
            MoreIconsAPI::countForType(IconType::Robot),
            MoreIconsAPI::countForType(IconType::Spider),
            MoreIconsAPI::countForType(IconType::Swing),
            MoreIconsAPI::countForType(IconType::Jetpack),
            MoreIconsAPI::countForType(IconType::Special)
        ),
        "OK",
        folderButton ? "Folder" : nullptr,
        300.0f,
        [folderButton](auto, bool btn2) {
            if (folderButton && btn2) file::openFolder(Mod::get()->getConfigDir());
        }
    );
}

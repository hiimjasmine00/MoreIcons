#include "MoreIcons.hpp"
#include "api/MoreIconsAPI.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Popup.hpp>
#include <geode.texture-loader/include/TextureLoader.hpp>
#include <Geode/utils/ranges.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
    auto mod = Mod::get();
    MoreIcons::debugLogs = mod->getSettingValue<bool>("debug-logs");
    MoreIcons::traditionalPacks = mod->getSettingValue<bool>("traditional-packs");
    #ifdef GEODE_IS_ANDROID
    auto tempDir = mod->getTempDir();
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
        if (MoreIcons::traditionalPacks) {
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

GEODE_IOS(const char* iosResourcePath(const char*);)

std::string MoreIcons::vanillaTexturePath(const std::string& path, bool skipSuffix) {
    #ifdef GEODE_IS_MOBILE
    if (CCDirector::get()->getContentScaleFactor() >= 4.0f && !skipSuffix) {
        if (auto highGraphicsMobile = Loader::get()->getLoadedMod("weebify.high-graphics-android")) {
            auto configDir = highGraphicsMobile->getConfigDir(false) / Loader::get()->getGameVersion();
            if (std::filesystem::exists(configDir)) return configDir / path;
        }
        return path;
    }
    GEODE_ANDROID(return fmt::format("assets/{}", path);)
    GEODE_IOS(return iosResourcePath(path.c_str());)
    #else
    return (dirs::getGameDir() / "Resources" / path).string();
    #endif
}

bool naturalSorter(const std::string& aStr, const std::string& bStr) {
    auto a = aStr.substr(aStr.find(':') + 1);
    auto b = bStr.substr(bStr.find(':') + 1);
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
    return fmt::format("{}{}", str.substr(0, str.size() - end), replace);
}

template <typename... Args>
void printLog(Severity severity, fmt::format_string<Args...> message, Args&&... args) {
    auto formatArgs = fmt::make_format_args(args...);
    log::vlogImpl(severity, Mod::get(), message, formatArgs);
    MoreIcons::logs.push_back({ fmt::vformat(message, formatArgs), severity });
    if (MoreIcons::severity < severity) MoreIcons::severity = severity;
}

template <typename... Args>
void safeDebug(fmt::format_string<Args...> message, Args&&... args) {
    if (MoreIcons::debugLogs) log::vlogImpl(Severity::Debug, Mod::get(), message, fmt::make_format_args(args...));
}

void loadFolderIcon(const std::filesystem::path& path, const IconPack& pack, IconType type) {
    auto scaleFactor = CCDirector::get()->getContentScaleFactor();
    auto name = pack.id.empty() ? path.stem().string() : fmt::format("{}:{}", pack.id, path.stem());

    safeDebug("Pre-loading folder icon {} from {}", name, pack.name);

    if (MoreIconsAPI::hasIcon(name, type)) return printLog(Severity::Warning, "{}: Duplicate icon name", path);

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

    if (MoreIconsAPI::hasIcon(name, type)) return printLog(Severity::Warning, "{}: Duplicate icon name", path);

    auto fullTexturePath = replaceEnd(path.string(), 6, ".png");
    if (!std::filesystem::exists(fullTexturePath)) return printLog(Severity::Error, "{}: Texture file {}.png not found", path, path.stem());

    MoreIconsAPI::icons.push_back({
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

    ranges::remove(MoreIconsAPI::icons, [name, type](const IconInfo& icon) { return icon.name == name && icon.type == type; });

    MoreIconsAPI::icons.push_back({
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

    if (MoreIconsAPI::hasIcon(name, IconType::Special)) return printLog(Severity::Warning, "{}: Duplicate trail name", path);

    auto json = file::readJson(replaceEnd(path.string(), 4, ".json")).unwrapOr(matjson::makeObject({
        { "blend", false },
        { "tint", false },
        { "show", false },
        { "fade", 0.3f },
        { "stroke", 14.0f }
    }));

    MoreIconsAPI::icons.push_back({
        .name = name,
        .textures = { path.string() },
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
    auto pathStem = path.stem().string();
    auto name = fmt::format("{}:{}", pack.id, pathStem);

    safeDebug("Pre-loading vanilla trail {} from {}", name, pack.name);
    auto trailID = numFromString<int>(pathStem.substr(pathStem.find('_') + 1, pathStem.rfind('_') - pathStem.find('_') - 1)).unwrapOr(0);
    if (trailID <= 0) return;

    ranges::remove(MoreIconsAPI::icons, [name](const IconInfo& icon) { return icon.name == name && icon.type == IconType::Special; });

    MoreIconsAPI::icons.push_back({
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

constexpr std::array prefixes = { "player_", "ship_", "player_ball_", "bird_", "dart_", "robot_", "spider_", "swing_", "jetpack_" };

void MoreIcons::loadIcons(const std::vector<IconPack>& packs, std::string_view suffix, IconType type) {
    MoreIconsAPI::iconIndices[type].first = MoreIconsAPI::icons.size();

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
            if (!traditionalPacks) continue;

            auto path = pack.path / "icons";
            if (!std::filesystem::exists(path)) continue;

            log::info("Pre-loading {}s from {}", suffix, path);
            for (auto& entry : std::filesystem::directory_iterator(path)) {
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                if (entryPath.extension() != ".png") continue;

                auto entryFilename = entryPath.filename().string();
                if (!entryFilename.starts_with(prefixes[(int)type]) || (type == IconType::Cube && entryFilename.starts_with("player_ball_"))) continue;

                loadVanillaIcon(entryPath, pack, type);
            }

            log::info("Finished pre-loading {}s from {}", suffix, path);
        }
    }

    naturalSort(type);

    MoreIconsAPI::iconIndices[type].second = MoreIconsAPI::icons.size();
}

void MoreIcons::loadTrails(const std::vector<IconPack>& packs) {
    MoreIconsAPI::iconIndices[IconType::Special].first = MoreIconsAPI::icons.size();

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
            if (!traditionalPacks) continue;

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
        })).mapErr([](const std::string& err) {
            return log::error("Failed to save trail JSON: {}", err), err;
        });
    }
}

void MoreIcons::showInfoPopup(bool folderButton) {
    auto counts = ranges::reduce<std::map<IconType, int>>(MoreIconsAPI::icons, [](std::map<IconType, int>& acc, const IconInfo& info) {
        acc[info.type]++;
        return acc;
    });

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
            counts[IconType::Cube],
            counts[IconType::Ship],
            counts[IconType::Ball],
            counts[IconType::Ufo],
            counts[IconType::Wave],
            counts[IconType::Robot],
            counts[IconType::Spider],
            counts[IconType::Swing],
            counts[IconType::Jetpack],
            counts[IconType::Special]
        ),
        "OK",
        folderButton ? "Folder" : nullptr,
        300.0f,
        [folderButton](auto, bool btn2) {
            if (folderButton && btn2) file::openFolder(Mod::get()->getConfigDir());
        }
    );
}
